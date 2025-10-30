#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "AvatarQuestRP.h"
#include "AvatarQuestGameWeapon.h"

using namespace AvatarQuest;

TEST_CASE("Skill attribute multiplier uses governing attribute", "[rp][skills]") {
    CharacterSheet cs{};
    setAttribute(cs.attributes, AttributeType::Dexterity, 14); // mod +4 => 1.2
    // Swords is governed by Dexterity
    REQUIRE(getSkillAttributeMultiplier(cs, SkillType::Swords) == Catch::Approx(1.2f).margin(1e-6f));
}

TEST_CASE("Compute HP/MP from attributes and level", "[rp][pools]") {
    CharacterSheet cs{};
    setLevel(cs, 3);
    setAttribute(cs.attributes, AttributeType::Endurance, 12); // +2
    setAttribute(cs.attributes, AttributeType::Intelligence, 11); // +1
    // HPMax = 50 + 3*10 + 12*5 = 50 + 30 + 60 = 140
    // MPMax = 30 + 3*8 + 11*5 = 30 + 24 + 55 = 109
    REQUIRE(computeMaxHP(cs) == 140);
    REQUIRE(computeMaxMP(cs) == 109);
}

TEST_CASE("Leveling increases level and pools, carries XP", "[rp][leveling]") {
    CharacterSheet cs{};
    // Defaults Level=1, XPToNext=100
    setAttribute(cs.attributes, AttributeType::Endurance, 10);
    setAttribute(cs.attributes, AttributeType::Intelligence, 10);
    recalcDerivedPools(cs, true);
    int hpMaxL1 = getHPMax(cs);
    int mpMaxL1 = getMPMax(cs);

    addExperience(cs, 150); // Should level once, carry 50 XP
    REQUIRE(getLevel(cs) == 2);
    REQUIRE(getExperience(cs) == 50);
    REQUIRE(getExperienceToNextLevel(cs) == 200);

    // Pools should have increased and refilled
    REQUIRE(getHPMax(cs) > hpMaxL1);
    REQUIRE(getMPMax(cs) > mpMaxL1);
    REQUIRE(getHP(cs) == getHPMax(cs));
    REQUIRE(getMP(cs) == getMPMax(cs));
}
