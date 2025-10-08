cmake_minimum_required(VERSION 3.24)

project(AvatarQuest VERSION 0.1.0 LANGUAGES C CXX)

# Make find_package honor <PackageName>_ROOT variables (helps find ZLIB_ROOT/PNG_ROOT when set)
if(POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW)
endif()



set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(AVATARQUEST_BUILD_TESTS "Build AvatarQuest tests" OFF)

# Option to vendor SDL3 from the official GitHub repo. When ON CMake will
# fetch and build SDL3 as part of the project. When OFF CMake will first try
# to find an installed SDL3 package via `find_package(SDL3 CONFIG)`. If a
# suitable SDL3 target isn't available the build will fall back to fetching
# SDL3 automatically (useful for CI or contributor setups).
option(AVATARQUEST_VENDOR_SDL3 "Download and build SDL3 from GitHub when system SDL3 is not available" OFF)
option(AVATARQUEST_SDL3_STATIC "When vendoring SDL3, build it as a static library instead of shared" ON)

# Options to vendor SDL_image / SDL_mixer / SDL_ttf (SDL3-compatible ports)
option(AVATARQUEST_VENDOR_SDL_IMAGE "Download and build SDL_image from GitHub when system SDL_image is not available" ${AVATARQUEST_VENDOR_SDL3})
option(AVATARQUEST_VENDOR_SDL_MIXER "Download and build SDL_mixer from GitHub when system SDL_mixer is not available" ${AVATARQUEST_VENDOR_SDL3})
option(AVATARQUEST_VENDOR_SDL_TTF "Download and build SDL_ttf from GitHub when system SDL_ttf is not available" ${AVATARQUEST_VENDOR_SDL3})

# Try to find an installed SDL3 package if the user didn't explicitly ask for
# a vendored copy.
if(NOT AVATARQUEST_VENDOR_SDL3)
    find_package(SDL3 CONFIG QUIET)
endif()

# If the SDL3 CMake target isn't available, fetch SDL3 from upstream GitHub.
if(NOT TARGET SDL3::SDL3)
    include(FetchContent)
    message(STATUS "SDL3 not found or vendor build requested; fetching SDL3 from GitHub (main branch)")

    # When vendoring, allow building SDL3 as static to avoid shipping a DLL.
    if(AVATARQUEST_SDL3_STATIC)
        set(SDL_SHARED OFF CACHE BOOL "Build SDL shared" FORCE)
        set(SDL_STATIC ON CACHE BOOL "Build SDL static" FORCE)
    endif()

    # If we're also vendoring SDL_image/mixer/ttf, prefer building SDL3 as a shared
    # library so its exported package config can be consumed by those subprojects.
    if(AVATARQUEST_VENDOR_SDL_IMAGE OR AVATARQUEST_VENDOR_SDL_MIXER OR AVATARQUEST_VENDOR_SDL_TTF)
        message(STATUS "Vendoring SDL_image/mixer/ttf: prefer building SDL3 as shared to satisfy subproject find_package()")
        set(SDL_SHARED ON CACHE BOOL "Build SDL shared" FORCE)
        set(SDL_STATIC OFF CACHE BOOL "Build SDL static" FORCE)
    endif()

    FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG main
    )
    FetchContent_MakeAvailable(SDL3)
endif()

# If SDL3 was fetched via FetchContent, make its binary dir available so subprojects can find it
if(DEFINED SDL3_SOURCE_DIR AND DEFINED SDL3_BINARY_DIR)
    # Some SDL subprojects expect SDL3Config.cmake via CMAKE_PREFIX_PATH / SDL3_DIR
    list(APPEND CMAKE_PREFIX_PATH "${SDL3_BINARY_DIR}")
    set(SDL3_DIR "${SDL3_BINARY_DIR}")
endif()


# Helper macro to try find_package for an SDL subproject and fall back to FetchContent
macro(_fetch_or_find_sdl3 name repo_var git_repo git_tag vendor_option)
    # name: SDL module short name (e.g. "SDL_image")
    # repo_var/git_repo/git_tag: FetchContent parameters
    if(NOT ${vendor_option})
        find_package(${name} CONFIG QUIET)
    endif()

    if(NOT TARGET ${name} AND NOT TARGET ${name}::${name})
        include(FetchContent)
        message(STATUS "${name} not found or vendor build requested; fetching ${name} from GitHub (${git_tag})")
        FetchContent_Declare(
            ${name}
            GIT_REPOSITORY ${git_repo}
            GIT_TAG ${git_tag}
        )
        FetchContent_MakeAvailable(${name})
    endif()
endmacro()

# Try to find or vendor SDL_image, SDL_mixer, SDL_ttf

# Configure a few SDL_image vendoring options to avoid heavy build deps (dav1d/asms)
if(NOT DEFINED SDLIMAGE_AVIF)
    set(SDLIMAGE_AVIF OFF CACHE BOOL "Disable AVIF support in vendored SDL_image to avoid nasm/dav1d" FORCE)
endif()
if(NOT DEFINED SDLIMAGE_DAV1D)
    set(SDLIMAGE_DAV1D OFF CACHE BOOL "Disable dav1d vendoring" FORCE)
endif()
if(NOT DEFINED SDLIMAGE_AOM)
    set(SDLIMAGE_AOM OFF CACHE BOOL "Disable aom vendoring" FORCE)
endif()

## NOTE: SDL3_image can use system image libraries (zlib, libpng, libjpeg, libtiff, giflib).
## Vendoring those projects reliably inside this top-level configure is fragile because
## some of them (libpng, libtiff, libjpeg-turbo) expect zlib or other system deps to be
## available at configure-time. If you want SDL3_image to use external libs, provide
## them via one of these approaches:
##  - Install dev packages on the system and ensure they are on PATH/standard include/lib locations.
##  - Use vcpkg and pass -DCMAKE_TOOLCHAIN_FILE=<vcpkg_root>/scripts/buildsystems/vcpkg.cmake to cmake.
##  - Manually set ZLIB_ROOT, PNG_PNG_INCLUDE_DIR, PNG_LIBRARY, JPEG_LIBRARY, TIFF_LIBRARY, GIF_LIBRARY
##    in the CMake cache to point to prebuilt libraries before running configure.
##
## If you prefer, we can add FetchContent for those libraries and attempt to wire up their
## CMake variables automatically — but that is error-prone on Windows (zlib/libpng need
## to be built first so find_package() can locate them). Ask me if you'd like me to try
## a vcpkg-based integration or an automatic FetchContent+install flow.

# Try to find or vendor SDL_image, SDL_mixer, SDL_ttf
_fetch_or_find_sdl3(SDL3_image SDL3_IMAGE_REPO https://github.com/libsdl-org/SDL_image.git main AVATARQUEST_VENDOR_SDL_IMAGE)
_fetch_or_find_sdl3(SDL3_mixer SDL3_MIXER_REPO https://github.com/libsdl-org/SDL_mixer.git main AVATARQUEST_VENDOR_SDL_MIXER)
_fetch_or_find_sdl3(SDL3_ttf SDL3_TTF_REPO https://github.com/libsdl-org/SDL_ttf.git main AVATARQUEST_VENDOR_SDL_TTF)

# Configure a few SDL_image vendoring options to avoid heavy build deps (dav1d/asms)
if(NOT DEFINED SDLIMAGE_AVIF)
    set(SDLIMAGE_AVIF OFF CACHE BOOL "Disable AVIF support in vendored SDL_image to avoid nasm/dav1d" FORCE)
endif()
if(NOT DEFINED SDLIMAGE_DAV1D)
    set(SDLIMAGE_DAV1D OFF CACHE BOOL "Disable dav1d vendoring" FORCE)
endif()
if(NOT DEFINED SDLIMAGE_AOM)
    set(SDLIMAGE_AOM OFF CACHE BOOL "Disable aom vendoring" FORCE)
endif()

add_executable(AvatarQuest
    src/main.cpp
)

target_include_directories(AvatarQuest
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/src/common
    ${PROJECT_SOURCE_DIR}/external/Imgui
        # Add stb single-header library include path
        ${PROJECT_SOURCE_DIR}/external/stb


)

# Add stb single-header library was here; removed (stb fallback removed)

target_link_libraries(AvatarQuest PRIVATE SDL3::SDL3)

# Link SDL_image / SDL_mixer / SDL_ttf if available (targets may be named SDL3_image::SDL3_image etc.)
if(TARGET SDL3_image::SDL3_image)
    target_link_libraries(AvatarQuest PRIVATE SDL3_image::SDL3_image)
elseif(TARGET SDL_image::SDL_image)
    target_link_libraries(AvatarQuest PRIVATE SDL_image::SDL_image)
endif()

if(TARGET SDL3_mixer::SDL3_mixer)
    target_link_libraries(AvatarQuest PRIVATE SDL3_mixer::SDL3_mixer)
elseif(TARGET SDL_mixer::SDL_mixer)
    target_link_libraries(AvatarQuest PRIVATE SDL_mixer::SDL_mixer)
endif()

if(TARGET SDL3_ttf::SDL3_ttf)
    target_link_libraries(AvatarQuest PRIVATE SDL3_ttf::SDL3_ttf)
elseif(TARGET SDL_ttf::SDL_ttf)
    target_link_libraries(AvatarQuest PRIVATE SDL_ttf::SDL_ttf)
endif()

# Add C++ engine sources found in src/common
file(GLOB COMMON_CPP_SOURCES ${PROJECT_SOURCE_DIR}/src/common/*.cpp)
target_sources(AvatarQuest PRIVATE ${COMMON_CPP_SOURCES})

# AvatarQuest module sources
file(GLOB AVATARQUEST_CPP_SOURCES ${PROJECT_SOURCE_DIR}/src/AvatarQuest/*.cpp)
if(AVATARQUEST_CPP_SOURCES)
    target_sources(AvatarQuest PRIVATE ${AVATARQUEST_CPP_SOURCES})
endif()

target_include_directories(AvatarQuest PRIVATE ${PROJECT_SOURCE_DIR}/src/AvatarQuest)

# ImGui: compile bundled backends and core from external/Imgui if present
if(EXISTS ${PROJECT_SOURCE_DIR}/external/Imgui)
    file(GLOB IMGUI_SOURCES
        ${PROJECT_SOURCE_DIR}/external/Imgui/*.cpp
    )
    if(IMGUI_SOURCES)
        target_sources(AvatarQuest PRIVATE ${IMGUI_SOURCES})
    endif()
endif()

    # Ensure a predictable runtime folder for the produced binary and copy SDL3 DLL there
    set(AVATARQUEST_BIN_DIR "${PROJECT_SOURCE_DIR}/bin")
    file(MAKE_DIRECTORY ${AVATARQUEST_BIN_DIR})

    set_target_properties(AvatarQuest PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${AVATARQUEST_BIN_DIR}
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${AVATARQUEST_BIN_DIR}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${AVATARQUEST_BIN_DIR}
    )

    # If SDL3 is a shared target, copy its runtime file (DLL) beside the executable.
    if(TARGET SDL3::SDL3)
        # Only copy the SDL3 runtime if SDL3 is built/linked as a shared library.
        get_target_property(SDL3_TYPE SDL3::SDL3 TYPE)
        if(SDL3_TYPE STREQUAL "SHARED_LIBRARY")
            add_custom_command(TARGET AvatarQuest POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_FILE:SDL3::SDL3>
                    ${AVATARQUEST_BIN_DIR}
                COMMENT "Copying SDL3 runtime to ${AVATARQUEST_BIN_DIR}"
            )
        endif()
    endif()

    # Copy runtimes for SDL_image / SDL_mixer / SDL_ttf if they're shared
    macro(_copy_if_shared tgt)
        if(TARGET ${tgt})
            get_target_property(_type ${tgt} TYPE)
            if(_type STREQUAL "SHARED_LIBRARY")
                add_custom_command(TARGET AvatarQuest POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        $<TARGET_FILE:${tgt}>
                        ${AVATARQUEST_BIN_DIR}
                )
            endif()
        endif()
    endmacro()

    _copy_if_shared(SDL3_image::SDL3_image)
    _copy_if_shared(SDL_image::SDL_image)
    _copy_if_shared(SDL3_mixer::SDL3_mixer)
    _copy_if_shared(SDL_mixer::SDL_mixer)
    _copy_if_shared(SDL3_ttf::SDL3_ttf)
    _copy_if_shared(SDL_ttf::SDL_ttf)

if(MSVC)
    target_compile_options(AvatarQuest PRIVATE /W4 /permissive-)
else()
    target_compile_options(AvatarQuest PRIVATE -Wall -Wextra -Wpedantic)
endif()

install(TARGETS AvatarQuest)
