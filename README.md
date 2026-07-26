# User Profile Project

A multi-module, multi-language project that demonstrates:
- C++ shared library design (Builder pattern, nested structs)
- A rich terminal UI CLI app in C++
- C# interop via a reflection-based bridge invoked with `dotnet exec`
- Extensible C# loader registry discovered entirely via **reflection**
- Structured logging (INFO/DEBUG) and YAML-based configuration

---

## Architecture

```
user-profile-project/
├── model/              # Module 1 — libUserProfileModel.so
│   ├── include/
│   │   ├── UserProfile.h   nested structs + Builder
│   │   ├── Logger.h        INFO/DEBUG/WARN/ERROR logger
│   │   └── Config.h        conf.yml reader
│   └── src/
│       ├── UserProfile.cpp
│       ├── Logger.cpp
│       └── Config.cpp
│
├── app/                # Module 2 — user_profile_app (executable)
│   ├── include/
│   │   ├── Terminal.h      ANSI colour helpers, table/field rendering
│   │   ├── UserStore.h     In-memory profile list
│   │   ├── CsBridgeInvoker.h  Extensible C# script registry + runner
│   │   └── Menus.h         All menu/screen declarations
│   └── src/
│       ├── main.cpp        Bootstrap: config → logger → bridge → menus
│       ├── UserStore.cpp
│       ├── CsBridgeInvoker.cpp
│       └── Menus.cpp       All interactive screens
│
├── csbridge/CsRunner/  # Module 3 — CsRunner.dll  (C# / .NET 8)
│   ├── Models/
│   │   └── UserProfile.cs  Mirror of C++ struct
│   ├── Loaders/
│   │   ├── IProfileLoader.cs   Interface (reflection target)
│   │   ├── LoaderRegistry.cs   Reflection-based discovery + dispatch
│   │   ├── JsonProfileLoader.cs
│   │   └── YamlProfileLoader.cs
│   ├── CsLogger.cs
│   └── Program.cs          Entry point; emits JSON sentinel block
│
├── config/conf.yml     Runtime configuration
├── samples/            Example JSON and YAML files
├── build.sh            Master build + run script
└── CMakeLists.txt      Root CMake
```

---

## Requirements

| Tool | Version | Install |
|------|---------|---------|
| CMake | ≥ 3.18 | `sudo apt install cmake` |
| GCC / Clang | C++17 | `sudo apt install g++` |
| ninja-build | any | `sudo apt install ninja-build` |
| nlohmann-json3-dev | any | `sudo apt install nlohmann-json3-dev` |
| dotnet-sdk-8.0 | ≥ 8.0 | https://dotnet.microsoft.com/download |

---

## Quick Start

```bash
# Install dependencies

sudo apt install cmake ninja-build nlohmann-json3-dev
chmod 755 dotnet-install.sh
./dotnet-install.sh --version latest
export DOTNET_ROOT=$HOME/.dotnet
export PATH=$PATH:$HOME/.dotnet
export DOTNET_ROLL_FORWARD=Major
dotnet restore csbridge/CsRunner/CsRunner.csproj

```

```bash
# First-time: build everything and run
./build.sh --build-all --run

# After code changes, rebuild and run
./build.sh --build-all --run --clean

# Build only C++ (faster iteration when C# unchanged)
./build.sh --build

# Build only C#
./build.sh --build-cs

# Run without rebuilding
./build.sh --run-only

# Release build
./build.sh --build-all --release --run
```

---

## Menu Structure

```
Main Menu
├── [1] User Management
│   ├── [1] Add New User        ← interactive form
│   ├── [2] List All Users      ← table view
│   └── [3] View Single User    ← full profile card
├── [2] View Users
│   ├── [1] List All Users
│   └── [2] View Single User
└── [3] C# Reflection Bridge
    ├── [1] Load from JSON/YAML File
    └── [2] List Registered C# Scripts
```

---

## Loading Files via C# Bridge

When you choose **C# Reflection Bridge → Load from File**, the app:

1. Asks for a file name (relative to the exe directory or `samples/`)
2. Calls `CsBridgeInvoker::invoke("profile_loader", {filePath})`
3. The invoker runs `dotnet exec CsRunner.dll <filePath>`
4. CsRunner's `LoaderRegistry` uses reflection to discover all `IProfileLoader`
   implementations, picks the right one by file extension, loads the profiles,
   **clones each instance via reflection** (`Activator.CreateInstance` + property iteration),
   then emits them as JSON wrapped in sentinel markers
5. The C++ side parses the JSON and adds valid profiles to the in-memory store

### Adding a New C# Loader

1. Implement `IProfileLoader` in `csbridge/CsRunner/Loaders/`
2. Rebuild: `./build.sh --build-cs`
3. The reflection registry picks it up automatically — no other changes needed

### Registering New C# Scripts in C++

In `app/src/main.cpp`, add to `registerDefaultScripts()`:

```cpp
bridge.registerScript({
    .name        = "my_script",
    .dllPath     = (exeDir / "csbridge/MyScript.dll").string(),
    .typeName    = "MyNamespace.MyClass",
    .description = "Does something useful"
});
```

Then invoke by name: `bridge.invoke("my_script", {arg1, arg2})`.

---

## Configuration (conf.yml)

```yaml
[app]
name    = UserProfileApp
version = 1.0.0

[logging]
level     = DEBUG      # DEBUG | INFO | WARN | ERROR
to_file   = false      # true → writes to file_path
file_path = app.log

[csbridge]
runner_path = csbridge/CsRunner.dll   # relative to exe
```

The C# side inherits the log level via `CSRUNNER_LOG_LEVEL` env var,
set automatically by `build.sh --run`.

---

## Nested UserProfile Structure

```
UserProfile
 ├── id          : string
 ├── firstName   : string
 ├── lastName    : string
 ├── username    : string
 ├── password    : string  (store hashed in production)
 ├── age         : int
 ├── emails      : []string
 ├── mobiles     : []string
 └── address     : Address
       ├── firstLine
       ├── aptUnit
       ├── city
       ├── state
       └── zip
```

Supported in both **interactive add** and **file import** (JSON + YAML).