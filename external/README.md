Vendor/External dependencies
============================

This folder documents how AvatarQuest can vendor or obtain external dependencies.

SDL3
----

By default the top-level `CMakeLists.txt` will try to `find_package(SDL3 CONFIG)`
in your system. If that fails (or if you prefer to build SDL3 alongside the
project), the project can fetch SDL3 from the official upstream GitHub repo
using CMake's `FetchContent`.

Enable vendoring via CMake cache option:

```powershell
cmake -S . -B build -DAVATARQUEST_VENDOR_SDL3=ON
cmake --build build
```

Notes:
- The CMake fetch pulls SDL from https://github.com/libsdl-org/SDL (main branch).
- On Windows you may prefer to use `vcpkg` and point CMake at the vcpkg toolchain
  file instead of vendoring. Example:

```powershell
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build
```

If you want a pinned SDL3 revision for reproducible builds, edit the
`FetchContent_Declare` in the top-level `CMakeLists.txt` and replace `GIT_TAG`
with a commit hash or release tag.
