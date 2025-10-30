Catch2 integration
===================

This directory fetches Catch2 at configure time using CMake's FetchContent.

How it works
------------
- Top-level `CMakeLists.txt` adds this directory when `AVATARQUEST_ENABLE_TESTS=ON`.
- This subdir declares `FetchContent_Declare(Catch2 ...)` and `FetchContent_MakeAvailable(Catch2)`.
- After configure, you can link against `Catch2::Catch2WithMain` or `Catch2::Catch2` targets.

Changing version
----------------
Update the `CATCH2_GIT_TAG` cache variable, for example:

```powershell
cmake -S . -B build -DCATCH2_GIT_TAG=v3.5.4
```

Offline vendoring
-----------------
If you prefer to vendor Catch2 sources directly, you can clone it into this folder
and remove the `FetchContent` logic, then use `add_subdirectory(external/Catch2)`
to bring in its CMake targets. Refer to Catch2's upstream README for details.

License
-------
Catch2 is distributed under the Boost Software License 1.0. See the upstream
project for the full text: https://github.com/catchorg/Catch2/blob/devel/LICENSE.txt
