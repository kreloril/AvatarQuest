cmake_minimum_required(VERSION 3.16)
project(AvatarQuest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Engine source files
file(GLOB ENGINE_SRC
    "${CMAKE_SOURCE_DIR}/common/*.cpp"
    "${CMAKE_SOURCE_DIR}/common/*.h"
)

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

# External libraries (SDL)
# Use shared (DLL) version of SDL3
set(SDL3_SHARED ON CACHE BOOL "" FORCE)
add_subdirectory(external/SDL)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/common
      ${CMAKE_SOURCE_DIR}/external/Imgui
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


target_link_libraries(AvatarQuest PRIVATE SDL3-shared)



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
endif()
