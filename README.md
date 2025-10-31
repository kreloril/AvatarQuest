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
├── src/                # C++ source files for the engine, platform, and layers
├── CMakeLists.txt      # Build configuration
└── README.md
```

## Prerequisites

* A C++20-capable compiler (MSVC, Clang, or GCC).
* [CMake](https://cmake.org/) 3.24 or newer.
* SDL3 development libraries available on your system. The build script expects `find_package(SDL3 CONFIG REQUIRED)` to succeed, which typically means SDL3 was installed using the official CMake package configuration.

### Additional build-time tools (when vendoring SDL subprojects)

When building with the repository's `AVATARQUEST_VENDOR_SDL3` option enabled (the project can fetch and build SDL3 and its related libraries like `SDL_image`, `SDL_mixer`, and `SDL_ttf`), the following tools may be required by some vendored subprojects:

- Perl (used by aom/dav1d and other configure scripts)
- NASM or YASM (assembler used by codecs like aom/dav1d for optimized builds)

If you plan to enable vendoring (or want full image/audio codec support), install these on your platform as shown below.

Windows (PowerShell)

- Install with Chocolatey (system-wide):

```powershell
choco install nasm -y
choco install strawberryperl -y
```

- Or with Scoop (user-level):

```powershell
scoop install nasm
scoop install perl
```

- Manual install:
	- NASM: https://www.nasm.us/
	- Perl: https://strawberryperl.com/ or https://www.activestate.com/products/perl/
	- Make sure `nasm.exe` and `perl.exe` are on your PATH (restart your shell after installing).

Linux (Debian/Ubuntu)

```bash
sudo apt update
sudo apt install nasm perl build-essential pkg-config
```

Fedora / RHEL (dnf)

```bash
sudo dnf install nasm perl make gcc-c++ pkgconfig
```

Arch Linux / Manjaro (pacman)

```bash
sudo pacman -S nasm perl base-devel pkgconf
```

Notes

- If you don't need AVIF or some optimized codec support, you can disable those features when configuring CMake (they are optional). For example:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DAVATARQUEST_VENDOR_SDL3=ON -DSDLIMAGE_AVIF=OFF -DSDLIMAGE_DAV1D=OFF -DSDLIMAGE_AOM=OFF
```

- The CMake option `AVATARQUEST_VENDOR_SDL3` controls whether SDL3 (and its helper libraries) are fetched and built automatically. When OFF, CMake will try to find system-installed packages first.


## Building

```powershell
# Cross-platform (single-config generators):
cmake -S . -B build
cmake --build build

# Windows + Visual Studio (multi-config):
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug
```

On success, the `AvatarQuest` executable will be placed in the build output directory.

## Running

From the repository root:

```powershell
# Single-config builds
./build/AvatarQuest

# Visual Studio multi-config builds
./build/Debug/AvatarQuest.exe
```

A window titled **AvatarQuest** should appear. Press Esc or close the window to exit.

## Next Steps

* Flesh out the `AvatarQuestGame` struct with world data, player entities, and component systems.
* Expand the `platform` module with input handling, audio playback, and persistence helpers.
* Populate the `assets/` folder with real textures, music, and sound effects.
* Add CMake options for unit tests, tooling, or additional platforms as the project grows.
