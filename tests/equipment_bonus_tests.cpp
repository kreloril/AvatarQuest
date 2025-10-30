#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "AvatarQuestRP.h"
#include "AvatarQuestGameWeapon.h"
#include "AvatarQuestGameArmor.h"

using namespace AvatarQuest;

TEST_CASE("Equipment bonuses affect effective attributes", "[equipment][attributes]") {
    CharacterSheet cs{};
    // Base attributes
    setAttribute(cs.attributes, AttributeType::Strength, 10);
    setAttribute(cs.attributes, AttributeType::Dexterity, 8);
    setHPMax(cs, 5); // base pool

    // Create a weapon with Strength +4 and HPMax +10
    Ref<AvatarQuestWeapon> w = std::make_shared<AvatarQuestWeapon>();
    w->weaponName = "Test Sword";
    w->attributeBonuses.push_back({AttributeType::Strength, 4});
    w->attributeBonuses.push_back({AttributeType::HPMax, 10});

    // Create an armor with Dexterity +1
    Ref<AvatarQuestArmor> a = std::make_shared<AvatarQuestArmor>();
    a->armorName = "Test Boots";
    a->slot = ArmorSlot::Feet;
    a->attributeBonuses.push_back({AttributeType::Dexterity, 1});

    equipPrimaryWeapon(cs, w);
    equipArmor(cs, a);

    REQUIRE(getEffectiveAttribute(cs, AttributeType::Strength) == 14);
    REQUIRE(getEffectiveAttribute(cs, AttributeType::Dexterity) == 9);
    REQUIRE(getEffectiveAttribute(cs, AttributeType::HPMax) == 15);
}

TEST_CASE("Effective attribute multiplier reflects equipment bonuses", "[equipment][attributes]") {
    CharacterSheet cs{};
    setAttribute(cs.attributes, AttributeType::Strength, 10);

    Ref<AvatarQuestWeapon> w = std::make_shared<AvatarQuestWeapon>();
    w->attributeBonuses.push_back({AttributeType::Strength, 4});
    equipPrimaryWeapon(cs, w);

    // Effective Strength = 14 => modifier +4 => multiplier = 1 + 0.05*4 = 1.2
    REQUIRE(attributeMultiplierEffective(cs, AttributeType::Strength) == Catch::Approx(1.2f).margin(1e-6f));
}
