---
name: aegis-sensor-build
description: Configure, build, test, and run the Aegis sensor Windows collector from the command line. Use when working in the aegis repository on sensor/win32, when MSVC vcvars needs to be initialized for CMake, or when validating the msvc-debug/msvc-release presets without editing sensor/CMakePresets.json.
---

# Aegis Sensor Build

## Overview

Use this skill to configure, build, test, and run the `sensor/win32` project
through the repository helper script. Do not edit `sensor/CMakePresets.json`; it
is the source of truth for presets.

## Workflow

1. Resolve the repository root or `sensor/` directory.
2. Use `sensor/tools/invoke-msvc-cmake.ps1` for every configure/build/test/run
   operation.
3. Run PowerShell with execution policy bypass when invoking the script from
   automation:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\sensor\tools\invoke-msvc-cmake.ps1 -Action all -Preset msvc-debug -Fresh
```

From inside `sensor/`, use:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\invoke-msvc-cmake.ps1 -Action all -Preset msvc-debug -Fresh
```

## Actions

Use these actions with `-Preset msvc-debug` or `-Preset msvc-release`:

- `configure`: runs `cmake --preset <preset>`; add `-Fresh` to use
  `cmake --fresh`.
- `build`: runs `cmake --build --preset <preset>`.
- `test`: runs `ctest --preset <preset>`.
- `run`: runs `sensor/build/<preset>/win32/aegis_sensor_win32.exe`.
- `all`: runs configure, build, test, then run.

## Rules

- Treat `sensor/CMakePresets.json` as read-only unless the user explicitly asks
  to change presets.
- Do not hardcode Visual Studio installation paths. The helper discovers an
  active `cl.exe`, `vswhere`, or `vcvarsall.bat` from standard installation
  roots.
- Prefer validating both `msvc-debug` and `msvc-release` before reporting
  success.
- If build output contains warnings from placeholder code, report them but do
  not change source files unless the user asked for code cleanup.
