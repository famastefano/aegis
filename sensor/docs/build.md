# Build

The sensor is a standalone CMake project rooted at `sensor/`.

## Windows MSVC Environment

The Windows collector is a native MSVC project. Initialize an x64 MSVC
environment before invoking CMake. Use one of:

- Visual Studio Developer PowerShell
- Visual Studio Developer Command Prompt
- A shell where `vcvars64.bat` has already been called

The CMake presets intentionally do not call `vcvars` themselves. Configure and
build from an initialized environment:

```powershell
cmake --preset msvc-debug
cmake --build --preset msvc-debug
ctest --preset msvc-debug
```

Release builds use the matching release preset:

```powershell
cmake --preset msvc-release
cmake --build --preset msvc-release
ctest --preset msvc-release
```

Preset output is written to `sensor/build/<preset-name>`.

## Dependency Policy

Normal configure and build steps must not access the network. Repo-wide testing
dependency policy, including GoogleTest/GoogleMock v1.17, is documented in
[`../../docs/testing.md`](../../docs/testing.md).
