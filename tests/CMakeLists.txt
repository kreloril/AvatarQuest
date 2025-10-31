# Unit tests for AvatarQuest

cmake_minimum_required(VERSION 3.16)

# Helper to reduce duplication across multiple test executables
set(AQ_TEST_COMMON_INCLUDE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/AvatarQuest
)

set(AQ_TEST_PRODUCT_SOURCES
    ${CMAKE_SOURCE_DIR}/AvatarQuest/AvatarQuestPlayer.cpp
    ${CMAKE_SOURCE_DIR}/AvatarQuest/AvatarQuestGameWeapon.cpp
    ${CMAKE_SOURCE_DIR}/AvatarQuest/AvatarQuestGameArmor.cpp
    ${CMAKE_SOURCE_DIR}/AvatarQuest/AvatarQuestNaming.cpp
    ${CMAKE_SOURCE_DIR}/AvatarQuest/AvatarQuestSkills.cpp
    ${CMAKE_SOURCE_DIR}/AvatarQuest/AvatarQuestRP.cpp
    ${CMAKE_SOURCE_DIR}/AvatarQuest/AvatarQuestCurrency.cpp
    ${CMAKE_SOURCE_DIR}/common/Random.cpp
)

find_package(Catch2 3 CONFIG QUIET)
if(Catch2_FOUND)
    set(AQ_CATCH_TARGET Catch2::Catch2WithMain)
    include(Catch)
else()
    set(AQ_CATCH_TARGET Catch2::Catch2WithMain)
endif()

function(aq_add_test_exe NAME)
    add_executable(${NAME} ${ARGN})
    target_include_directories(${NAME} PRIVATE ${AQ_TEST_COMMON_INCLUDE})
    target_link_libraries(${NAME} PRIVATE ${AQ_CATCH_TARGET} SDL3-shared)
    target_sources(${NAME} PRIVATE ${AQ_TEST_PRODUCT_SOURCES})
    target_compile_definitions(${NAME} PRIVATE AQ_BUILD_TESTS=1)
    # Avoid MSVC PDB write contention when building tests in parallel
    if (MSVC)
        target_compile_options(${NAME} PRIVATE /FS)
    endif()
    add_test(NAME ${NAME} COMMAND ${NAME})
endfunction()

# Define separate test executables by domain to keep output manageable
aq_add_test_exe(aq_tests_weapon         weapon_tests.cpp)
aq_add_test_exe(aq_tests_armor          armor_tests.cpp)
aq_add_test_exe(aq_tests_skills         skills_tests.cpp skills_eval_tests.cpp)
aq_add_test_exe(aq_tests_combat         combat_skill_tests.cpp)
aq_add_test_exe(aq_tests_equipment      equipment_bonus_tests.cpp)
aq_add_test_exe(aq_tests_leveling       leveling_tests.cpp)
aq_add_test_exe(aq_tests_naming         naming_tests.cpp)
aq_add_test_exe(aq_tests_misc           sample_test.cpp player_camera_tests.cpp)
aq_add_test_exe(aq_tests_currency       currency_tests.cpp)
