cmake_minimum_required(VERSION 3.16)

# This subdirectory fetches Catch2 and makes it available to the parent project.
# It does not build any tests by itself.

include(FetchContent)

set(CATCH2_GIT_REPOSITORY "https://github.com/catchorg/Catch2.git" CACHE STRING "Catch2 upstream repository")
set(CATCH2_GIT_TAG "v3.5.4" CACHE STRING "Catch2 version/tag to fetch")

message(STATUS "Fetching Catch2 from ${CATCH2_GIT_REPOSITORY} (${CATCH2_GIT_TAG})")

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY ${CATCH2_GIT_REPOSITORY}
    GIT_TAG        ${CATCH2_GIT_TAG}
)

# Avoid overriding parent policies/variables
set(FETCHCONTENT_QUIET OFF)
FetchContent_MakeAvailable(Catch2)

# Expose the extras/cmake functions (e.g., catch_discover_tests) to parent
# The lower-case name 'catch2' is used by FetchContent variables
FetchContent_GetProperties(Catch2)
if(catch2_SOURCE_DIR)
    list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")
endif()
