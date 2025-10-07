cmake_minimum_required(VERSION 3.24)

project(AvatarQuest VERSION 0.1.0 LANGUAGES C CXX)



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

    FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG main
    )
    FetchContent_MakeAvailable(SDL3)
endif()

add_executable(AvatarQuest
    src/main.cpp
)

target_include_directories(AvatarQuest
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/src/common
    ${PROJECT_SOURCE_DIR}/external/Imgui
)

target_link_libraries(AvatarQuest PRIVATE SDL3::SDL3)

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

if(MSVC)
    target_compile_options(AvatarQuest PRIVATE /W4 /permissive-)
else()
    target_compile_options(AvatarQuest PRIVATE -Wall -Wextra -Wpedantic)
endif()

install(TARGETS AvatarQuest)
