cmake_minimum_required(VERSION 3.24)

project(AvatarQuest VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(AVATARQUEST_BUILD_TESTS "Build AvatarQuest tests" OFF)

find_package(SDL3 CONFIG REQUIRED)

add_executable(AvatarQuest
    src/main.c
    src/game.c
    src/platform.c
)

target_include_directories(AvatarQuest
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(AvatarQuest PRIVATE SDL3::SDL3)

if(MSVC)
    target_compile_options(AvatarQuest PRIVATE /W4 /permissive-)
else()
    target_compile_options(AvatarQuest PRIVATE -Wall -Wextra -Wpedantic)
endif()

install(TARGETS AvatarQuest)
