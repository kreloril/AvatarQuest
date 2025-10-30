#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "Common.h"
#include "AvatarQuestRP.h"

using namespace AvatarQuest;

TEST_CASE("CharacterSheet: basic pools and XP fields exist", "[rp][charsheet]") {
    CharacterSheet cs{};
    // Defaults
    REQUIRE(getLevel(cs) == 1);
    REQUIRE(getHP(cs) == 0);
    REQUIRE(getHPMax(cs) == 0);
    REQUIRE(getMP(cs) == 0);
    REQUIRE(getMPMax(cs) == 0);
    REQUIRE(getExperience(cs) == 0);
    REQUIRE(getExperienceToNextLevel(cs) == 100);
}

TEST_CASE("Attributes: modifier and multiplier computation", "[rp][attributes]") {
    CharacterSheet cs{};
    setAttribute(cs.attributes, AttributeType::Strength, 14); // mod +4
    setAttribute(cs.attributes, AttributeType::Dexterity, 8); // mod -2

    REQUIRE(attributeModifier(14) == 4);
    REQUIRE(attributeModifier(8) == -2);

    // Multiplier = 1 + 0.05 * mod
    REQUIRE(attributeMultiplier(cs, AttributeType::Strength) == Catch::Approx(1.2f).margin(1e-6f));
    REQUIRE(attributeMultiplier(cs, AttributeType::Dexterity) == Catch::Approx(0.9f).margin(1e-6f));
}
