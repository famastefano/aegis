param(
    [ValidateSet('configure', 'build', 'test', 'run', 'all')]
    [string] $Action = 'all',

    [ValidateSet('msvc-debug', 'msvc-release')]
    [string] $Preset = 'msvc-debug',

    [string] $ProjectDir = (Get-Location).Path,

    [ValidateSet('x64', 'x86', 'arm64')]
    [string] $Arch = 'x64',

    [string] $RunArgs = '',

    [switch] $Fresh
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Resolve-SensorRoot {
    param([string] $Path)

    $resolved = (Resolve-Path -LiteralPath $Path).Path
    $directPreset = Join-Path $resolved 'CMakePresets.json'
    $directProject = Join-Path $resolved 'CMakeLists.txt'
    if ((Test-Path -LiteralPath $directPreset) -and (Test-Path -LiteralPath $directProject)) {
        return $resolved
    }

    $nested = Join-Path $resolved 'sensor'
    $nestedPreset = Join-Path $nested 'CMakePresets.json'
    $nestedProject = Join-Path $nested 'CMakeLists.txt'
    if ((Test-Path -LiteralPath $nestedPreset) -and (Test-Path -LiteralPath $nestedProject)) {
        return $nested
    }

    throw "Could not resolve a sensor CMake project from '$Path'. Run from the repository root or sensor directory, or pass -ProjectDir."
}

function Get-RequiredCommand {
    param([string] $Name)

    $command = Get-Command $Name -ErrorAction SilentlyContinue
    if (-not $command) {
        throw "Required command '$Name' was not found on PATH."
    }

    return $command.Source
}

function Find-VsWhere {
    $command = Get-Command vswhere -ErrorAction SilentlyContinue
    if ($command) {
        return $command.Source
    }

    $programFilesX86 = [Environment]::GetFolderPath('ProgramFilesX86')
    if ($programFilesX86) {
        $candidate = Join-Path $programFilesX86 'Microsoft Visual Studio\Installer\vswhere.exe'
        if (Test-Path -LiteralPath $candidate) {
            return $candidate
        }
    }

    return $null
}

function Find-VcVarsAll {
    $vsWhere = Find-VsWhere
    if ($vsWhere) {
        $installPath = & $vsWhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
        if (($LASTEXITCODE -eq 0) -and $installPath) {
            $candidate = Join-Path $installPath 'VC\Auxiliary\Build\vcvarsall.bat'
            if (Test-Path -LiteralPath $candidate) {
                return $candidate
            }
        }
    }

    $roots = @(
        [Environment]::GetFolderPath('ProgramFiles'),
        [Environment]::GetFolderPath('ProgramFilesX86')
    ) | Where-Object { $_ }

    foreach ($root in $roots) {
        $visualStudioRoot = Join-Path $root 'Microsoft Visual Studio'
        if (-not (Test-Path -LiteralPath $visualStudioRoot)) {
            continue
        }

        $candidate = Get-ChildItem -Path $visualStudioRoot -Filter vcvarsall.bat -Recurse -ErrorAction SilentlyContinue |
            Sort-Object -Property FullName -Descending |
            Select-Object -First 1
        if ($candidate) {
            return $candidate.FullName
        }
    }

    return $null
}

function ConvertTo-CmdArgument {
    param([string] $Value)

    if ($Value -notmatch '[\s&()<>|^"]') {
        return $Value
    }

    return '"' + ($Value -replace '"', '\"') + '"'
}

function Split-RunArguments {
    param([string] $CommandLine)

    if ([string]::IsNullOrWhiteSpace($CommandLine)) {
        return @()
    }

    $arguments = [System.Collections.Generic.List[string]]::new()
    $current = [System.Text.StringBuilder]::new()
    $inSingleQuote = $false
    $inDoubleQuote = $false
    $escapeNext = $false

    foreach ($character in $CommandLine.ToCharArray()) {
        if ($escapeNext) {
            [void] $current.Append($character)
            $escapeNext = $false
            continue
        }

        if ($character -eq '`') {
            $escapeNext = $true
            continue
        }

        if ($character -eq "'" -and -not $inDoubleQuote) {
            $inSingleQuote = -not $inSingleQuote
            continue
        }

        if ($character -eq '"' -and -not $inSingleQuote) {
            $inDoubleQuote = -not $inDoubleQuote
            continue
        }

        if ([char]::IsWhiteSpace($character) -and -not $inSingleQuote -and -not $inDoubleQuote) {
            if ($current.Length -gt 0) {
                $arguments.Add($current.ToString())
                [void] $current.Clear()
            }
            continue
        }

        [void] $current.Append($character)
    }

    if ($escapeNext) {
        [void] $current.Append('`')
    }

    if ($inSingleQuote -or $inDoubleQuote) {
        throw 'Run arguments contain an unterminated quote.'
    }

    if ($current.Length -gt 0) {
        $arguments.Add($current.ToString())
    }

    return $arguments.ToArray()
}

function Invoke-MsvcCommand {
    param(
        [string] $SensorRoot,
        [string[]] $Arguments
    )

    $commandLine = ($Arguments | ForEach-Object { ConvertTo-CmdArgument $_ }) -join ' '

    if ($script:VcVarsAll) {
        $vcvars = ConvertTo-CmdArgument $script:VcVarsAll
        $commandLine = "call $vcvars $script:Arch >nul && $commandLine"
    }

    Push-Location $SensorRoot
    try {
        & cmd.exe /d /s /c $commandLine
        if ($LASTEXITCODE -ne 0) {
            throw "Command failed with exit code $LASTEXITCODE`: $commandLine"
        }
    }
    finally {
        Pop-Location
    }
}

$sensorRoot = Resolve-SensorRoot -Path $ProjectDir

Get-RequiredCommand -Name cmake | Out-Null
Get-RequiredCommand -Name ctest | Out-Null
Get-RequiredCommand -Name ninja | Out-Null

$script:VcVarsAll = $null
$script:Arch = $Arch

if (-not (Get-Command cl -ErrorAction SilentlyContinue)) {
    $script:VcVarsAll = Find-VcVarsAll
    if (-not $script:VcVarsAll) {
        throw 'MSVC cl.exe is not active and vcvarsall.bat could not be discovered. Install Visual Studio or Build Tools with the x64 C++ workload.'
    }
}

Push-Location $sensorRoot
try {
    $presetNames = (cmake --list-presets 2>$null) -join "`n"
    if ($presetNames -notmatch [Regex]::Escape("`"$Preset`"")) {
        throw "CMake preset '$Preset' was not found in '$sensorRoot'."
    }
}
finally {
    Pop-Location
}

function Invoke-Configure {
    $arguments = @('cmake')
    if ($Fresh) {
        $arguments += '--fresh'
    }

    $arguments += @('--preset', $Preset)
    Invoke-MsvcCommand -SensorRoot $sensorRoot -Arguments $arguments
    Copy-CompileCommands -SensorRoot $sensorRoot -Preset $Preset
}

function Copy-CompileCommands {
    param(
        [string] $SensorRoot,
        [string] $Preset
    )

    $source = Join-Path $SensorRoot "build\$Preset\compile_commands.json"
    $destination = Join-Path $SensorRoot 'compile_commands.json'

    if (-not (Test-Path -LiteralPath $source)) {
        throw "CMake did not generate '$source'. Check CMAKE_EXPORT_COMPILE_COMMANDS for preset '$Preset'."
    }

    Copy-Item -LiteralPath $source -Destination $destination -Force
}

function Invoke-Build {
    Invoke-MsvcCommand -SensorRoot $sensorRoot -Arguments @('cmake', '--build', '--preset', $Preset)
}

function Invoke-Test {
    Invoke-MsvcCommand -SensorRoot $sensorRoot -Arguments @('ctest', '--preset', $Preset)
}

function Invoke-Run {
    $exe = Join-Path $sensorRoot "build\$Preset\win32\aegis_sensor_win32.exe"
    if (-not (Test-Path -LiteralPath $exe)) {
        throw "Executable was not found at '$exe'. Build the '$Preset' preset first."
    }

    Invoke-MsvcCommand -SensorRoot $sensorRoot -Arguments (@($exe) + (Split-RunArguments -CommandLine $RunArgs))
}

switch ($Action) {
    'configure' { Invoke-Configure }
    'build' { Invoke-Build }
    'test' { Invoke-Test }
    'run' { Invoke-Run }
    'all' {
        Invoke-Configure
        Invoke-Build
        Invoke-Test
        Invoke-Run
    }
}
