#include <catch2/catch_test_macros.hpp>
#include "Common.h"
#include "AvatarQuestNaming.h"

using namespace AvatarQuest;

static Vector<DamageInstanceType> makeOne(DamageType t) {
    Vector<DamageInstanceType> v;
    DamageInstanceType d{}; d.damageType = t; d.damageRange = {1,2}; d.tileID = -1;
    v.push_back(d);
    return v;
}

TEST_CASE("Naming: tier adjective by level band for weapons", "[naming][weapon]") {
    // Level bands: band = (level-1)/5
    auto name1 = GenerateWeaponName(makeOne(DamageType::Damage_Slashing), 1);
    REQUIRE(name1.rfind("Shoddy ", 0) == 0);

    auto name5 = GenerateWeaponName(makeOne(DamageType::Damage_Slashing), 5);
    REQUIRE(name5.rfind("Shoddy ", 0) == 0);

    auto name6 = GenerateWeaponName(makeOne(DamageType::Damage_Slashing), 6);
    REQUIRE(name6.rfind("Worn ", 0) == 0);

    auto name46 = GenerateWeaponName(makeOne(DamageType::Damage_Slashing), 46);
    REQUIRE(name46.rfind("Exquisite ", 0) == 0);
}

TEST_CASE("Naming: overflow bands use '+N' suffix at high levels", "[naming]") {
    // Highest tier is index 16 (Empyrean); band 17 => level 86..90 => 'Empyrean +1'
    auto name90w = GenerateWeaponName(makeOne(DamageType::Damage_Fire), 90);
    REQUIRE(name90w.find("Empyrean +1 ") != String::npos);

    auto name90a = GenerateArmorName(makeOne(DamageType::Damage_Fire), 90, ArmorType::Plate);
    REQUIRE(name90a.find("Empyrean +1 ") != String::npos);
}
