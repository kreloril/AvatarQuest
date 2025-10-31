cmake_minimum_required(VERSION 3.16)
project(AvatarQuest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MSVC compatibility: define SDL_ALIGNED if SDL3 headers don't provide it yet
if (MSVC)
    add_compile_definitions("SDL_ALIGNED(x)=__declspec(align(x))")
endif()

# Optionally enable unit tests (Catch2)
option(AVATARQUEST_ENABLE_TESTS "Build unit tests with Catch2" ON)
# Toggle audio (SDL3_mixer). We'll default ON if the vendored folder exists.
set(_AQ_AUDIO_DEFAULT OFF)
if (EXISTS "${CMAKE_SOURCE_DIR}/external/SDL_mixer/CMakeLists.txt")
    set(_AQ_AUDIO_DEFAULT ON)
endif()
option(AVATARQUEST_ENABLE_AUDIO "Enable audio via SDL3_mixer" ${_AQ_AUDIO_DEFAULT})

# Engine source files
file(GLOB ENGINE_SRC
    "${CMAKE_SOURCE_DIR}/common/*.cpp"
    "${CMAKE_SOURCE_DIR}/common/*.h"
)

# Drop Sound.cpp from sources when audio is disabled
if (NOT AVATARQUEST_ENABLE_AUDIO)
    list(REMOVE_ITEM ENGINE_SRC "${CMAKE_SOURCE_DIR}/common/Sound.cpp")
endif()

source_group("OffenderEngine" FILES
    ${ENGINE_SRC}
)

source_group("AvatarQuest" FILES
    ${GAME_SRC}
)

file(GLOB IMGUI_SRC
    "${CMAKE_SOURCE_DIR}/external/imgui/*.cpp"
    "${CMAKE_SOURCE_DIR}/external/imgui/*.h"
)

source_group("ImGui" FILES
    ${IMGUI_SRC}
)

file(GLOB OffenderEdit 
    "${CMAKE_SOURCE_DIR}/OffenderEdit/*.cpp"
    "${CMAKE_SOURCE_DIR}/OffenderEdit/*.h"
)

source_group("OffenderEdit" FILES
    ${OffenderEdit}
)


FILE(GLOB AvatarQuest
    "${CMAKE_SOURCE_DIR}/AvatarQuest/*.cpp"
    "${CMAKE_SOURCE_DIR}/AvatarQuest/*.h"
)

source_group("AvatarQuest" FILES
    ${AvatarQuest}
)

set(COMMONE_ENGINE
    ${ENGINE_SRC}
    ${IMGUI_SRC}
)

# External libraries (SDL/SDL_ttf/SDL_mixer)
# Use shared (DLL) version of SDL3
set(SDL3_SHARED ON CACHE BOOL "" FORCE)
add_subdirectory(external/SDL)

# Text rendering via vendored SDL_ttf
if (EXISTS "${CMAKE_SOURCE_DIR}/external/SDL_ttf/CMakeLists.txt")
    add_subdirectory(external/SDL_ttf)
else()
    message(FATAL_ERROR "external/SDL_ttf not found. Please add it or switch to a FetchContent path.")
endif()

# Audio via vendored SDL_mixer when enabled
if (AVATARQUEST_ENABLE_AUDIO)
    if (EXISTS "${CMAKE_SOURCE_DIR}/external/SDL_mixer/CMakeLists.txt")
        # Keep SDL_mixer lean and avoid tricky deps on Windows
        set(SDLMIXER_WAVPACK OFF CACHE BOOL "" FORCE)
        set(SDLMIXER_GME OFF CACHE BOOL "" FORCE)
        set(SDLMIXER_MOD_XMP OFF CACHE BOOL "" FORCE)
        set(SDLMIXER_MIDI_FLUIDSYNTH OFF CACHE BOOL "" FORCE)
        add_subdirectory(external/SDL_mixer)
        add_compile_definitions(AVATARQUEST_ENABLE_AUDIO=1)
    else()
        message(FATAL_ERROR "AVATARQUEST_ENABLE_AUDIO=ON but external/SDL_mixer not found. Provide it or disable audio.")
    endif()
endif()

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/external/Imgui
    ${CMAKE_SOURCE_DIR}/external/stb
    ${CMAKE_SOURCE_DIR}/external/SDL/include
    ${CMAKE_BINARY_DIR}/external/SDL/Debug
    ${CMAKE_BINARY_DIR}/external/SDL/Release
)


# Set output directory to ./bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)


add_executable(AvatarQuest
    ${COMMONE_ENGINE}
    ${AvatarQuest}
)

# Set working directory to ./bin for MSVC
if (MSVC)
    set_target_properties(AvatarQuest PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
    target_link_options(AvatarQuest PRIVATE /SUBSYSTEM:WINDOWS)

endif()

add_dependencies(AvatarQuest SDL3-shared)
add_dependencies(AvatarQuest SDL3_ttf-shared)
if (AVATARQUEST_ENABLE_AUDIO)
    add_dependencies(AvatarQuest SDL3_mixer-shared)
endif()

target_link_libraries(AvatarQuest PRIVATE SDL3-shared SDL3_ttf::SDL3_ttf)
if (AVATARQUEST_ENABLE_AUDIO)
    target_link_libraries(AvatarQuest PRIVATE SDL3_mixer::SDL3_mixer)
endif()


target_include_directories(AvatarQuest PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/external/SDL/include
    ${CMAKE_BINARY_DIR}/external/SDL/Debug
    ${CMAKE_BINARY_DIR}/external/SDL/Release
)


# Link additional dependencies if needed (OpenGL, Win32, etc.)
if(WIN32)

set(COMMON_LIB 
opengl32
)

    target_link_libraries(AvatarQuest PRIVATE ${COMMON_LIB})
    # Copy SDL3.dll to ./bin after build

    add_custom_command(TARGET AvatarQuest POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:SDL3-shared>"
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    )
    add_custom_command(TARGET AvatarQuest POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:SDL3_ttf::SDL3_ttf>"
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    )
    if (AVATARQUEST_ENABLE_AUDIO)
        add_custom_command(TARGET AvatarQuest POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:SDL3_mixer::SDL3_mixer>"
                "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        )
        # Copy common codec DLLs if these targets exist (Windows runtime dependency)
        foreach(_codec_target IN ITEMS libmpg123 ogg vorbis vorbisfile FLAC opus opusfile)
            if (TARGET ${_codec_target})
                add_custom_command(TARGET AvatarQuest POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "$<TARGET_FILE:${_codec_target}>"
                        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
                )
            endif()
        endforeach()
    endif()
endif()

# -----------------------------
# Tests (Catch2)
# -----------------------------
if (AVATARQUEST_ENABLE_TESTS)
    include(CTest)
    enable_testing()
    # Make Catch2 available via a small FetchContent wrapper
    add_subdirectory(external/Catch2)
    # Add our tests directory
    add_subdirectory(tests)
endif()
