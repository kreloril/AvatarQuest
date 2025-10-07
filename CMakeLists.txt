cmake_minimum_required(VERSION 3.24)

project(AvatarQuest VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(AVATARQUEST_BUILD_TESTS "Build AvatarQuest tests" OFF)

# Option to vendor SDL3 from the official GitHub repo. When ON CMake will
# fetch and build SDL3 as part of the project. When OFF CMake will first try
# to find an installed SDL3 package via `find_package(SDL3 CONFIG)`. If a
# suitable SDL3 target isn't available the build will fall back to fetching
# SDL3 automatically (useful for CI or contributor setups).
option(AVATARQUEST_VENDOR_SDL3 "Download and build SDL3 from GitHub when system SDL3 is not available" OFF)

# Try to find an installed SDL3 package if the user didn't explicitly ask for
# a vendored copy.
if(NOT AVATARQUEST_VENDOR_SDL3)
    find_package(SDL3 CONFIG QUIET)
endif()

# If the SDL3 CMake target isn't available, fetch SDL3 from upstream GitHub.
if(NOT TARGET SDL3::SDL3)
    include(FetchContent)
    message(STATUS "SDL3 not found or vendor build requested; fetching SDL3 from GitHub (main branch)")
    FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG main
    )
    FetchContent_MakeAvailable(SDL3)
endif()

add_executable(AvatarQuest
    src/main.c
    src/game.c
    src/platform.c
)

target_include_directories(AvatarQuest
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/src/common
)

target_link_libraries(AvatarQuest PRIVATE SDL3::SDL3)

# Add C++ engine sources found in src/common
file(GLOB COMMON_CPP_SOURCES ${PROJECT_SOURCE_DIR}/src/common/*.cpp)
target_sources(AvatarQuest PRIVATE ${COMMON_CPP_SOURCES})

if(MSVC)
    target_compile_options(AvatarQuest PRIVATE /W4 /permissive-)
else()
    target_compile_options(AvatarQuest PRIVATE -Wall -Wextra -Wpedantic)
endif()

install(TARGETS AvatarQuest)
