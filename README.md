# AvatarQuest

AvatarQuest is an SDL3-based starter project that lays the foundation for building a 2D adventure game. The repository provides a clean layout for engine, platform, and gameplay code, along with directories ready for art and audio assets.

## Project Structure

```
AvatarQuest/
├── assets/             # Placeholder folders for runtime assets
│   ├── audio/
│   ├── fonts/
│   ├── music/
│   ├── sfx/
│   └── textures/
├── docs/               # Design documents, story notes, etc.
├── include/
│   └── AvatarQuest/    # Public headers for the game modules
├── scripts/            # Utility scripts for tooling and automation
├── src/                # C source files for the game and platform layer
├── CMakeLists.txt      # Build configuration
└── README.md
```

## Prerequisites

* A C17-capable compiler (GCC, Clang, or MSVC).
* [CMake](https://cmake.org/) 3.24 or newer.
* SDL3 development libraries available on your system. The build script expects `find_package(SDL3 CONFIG REQUIRED)` to succeed, which typically means SDL3 was installed using the official CMake package configuration.

## Building

```bash
cmake -S . -B build
cmake --build build
```

On success, the `AvatarQuest` executable will be placed in the build output directory.

## Running

From the repository root:

```bash
./build/AvatarQuest
```

A window titled **AvatarQuest** should appear with a placeholder hero sprite represented by a golden square. Press <kbd>Esc</kbd> or close the window to exit.

## Next Steps

* Flesh out the `AvatarQuestGame` struct with world data, player entities, and component systems.
* Expand the `platform` module with input handling, audio playback, and persistence helpers.
* Populate the `assets/` folder with real textures, music, and sound effects.
* Add CMake options for unit tests, tooling, or additional platforms as the project grows.
