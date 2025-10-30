#include <catch2/catch_test_macros.hpp>
#include "Common.h"
#include "AvatarQuestGameArmor.h"
#include "AvatarQuestGameWeapon.h"
#include <random>

using namespace AvatarQuest;

TEST_CASE("GameArmor: createResistance uses models for tileID", "[armor][resistance]") {
    // Prepare a couple of resistance models with tile IDs
    DamageInstanceType models[2];
    models[0].damageType = DamageType::Damage_Fire;   models[0].tileID = 101; models[0].damageRange = { 1, 2 };
    models[1].damageType = DamageType::Damage_Ice;    models[1].tileID = 202; models[1].damageRange = { 3, 4 };

    createResistanceTypes(models, 2);

    DamageInstanceType out{};
    createResistance(DamageType::Damage_Fire, { 10, 20 }, out);
    REQUIRE(out.damageType == DamageType::Damage_Fire);
    REQUIRE(out.damageRange.x == 10.0f);
    REQUIRE(out.damageRange.y == 20.0f);
    REQUIRE(out.tileID == 101);

    createResistance(DamageType::Damage_Poison, { 5, 6 }, out);
    REQUIRE(out.damageType == DamageType::Damage_Poison);
    REQUIRE(out.tileID == -1); // not in models
}

TEST_CASE("Armor: description samples for all armor types and slots", "[armor][description][console]") {
    using TypePair = std::pair<ArmorType, const char*>;
    using SlotPair = std::pair<ArmorSlot, const char*>;

    Vector<TypePair> types = {
        { ArmorType::Cloth,   "Cloth" },
        { ArmorType::Leather, "Leather" },
        { ArmorType::Chain,   "Chain" },
        { ArmorType::Plate,   "Plate" },
        { ArmorType::Shield,  "Shield" }
    };
    Vector<SlotPair> slots = {
        { ArmorSlot::Head,      "Head" },
        { ArmorSlot::Chest,     "Chest" },
        { ArmorSlot::Feet,      "Feet" },
        { ArmorSlot::Hands,     "Hands" },
        { ArmorSlot::Shoulders, "Shoulders" }
    };
    Vector<int> levels = {1, 10, 25};

    // Fixed resistances used across samples
    DamageInstanceType slash{}; createResistance(DamageType::Damage_Slashing, { 2.f, 5.f }, slash);
    DamageInstanceType fire{};  createResistance(DamageType::Damage_Fire,      { 1.f, 3.f }, fire);

    for (auto [t, tname] : types) {
        for (auto [s, sname] : slots) {
            for (int lvl : levels) {
                Vector<DamageInstanceType> res{ slash, fire };
                Ref<AvatarQuestArmor> a;
                createArmor(res, lvl, t, s, a);
                REQUIRE(a != nullptr);
                REQUIRE_FALSE(a->description.empty());

                // Basic sanity of description contains key labels
                REQUIRE(a->description.find("Level ") != String::npos);
                REQUIRE(a->description.find("Type=") != String::npos);
                REQUIRE(a->description.find("Slot=") != String::npos);
                REQUIRE(a->description.find("AC=") != String::npos);
                REQUIRE(a->description.find("Dodge=") != String::npos);
                REQUIRE(a->description.find("Durability=") != String::npos);
                REQUIRE(a->description.find("Resists:") != String::npos);

                std::cout << "[ArmorDescSample] type=" << tname
                          << " slot=" << sname
                          << " lvl=" << lvl << "\n"
                          << a->description << "\n";
            }
        }
    }
}

TEST_CASE("Naming: sample outputs for types and levels", "[naming][console]") {
    using Pair = std::pair<DamageType, const char*>;
    Vector<Pair> types = {
        {DamageType::Damage_Fire, "Fire"},
        {DamageType::Damage_Ice, "Ice"},
        {DamageType::Damage_Lightning, "Lightning"},
        {DamageType::Damage_Slashing, "Slashing"}
    };
    Vector<int> levels = {1, 5, 10, 25, 50};

    for (auto [t,label] : types) {
        for (int lvl : levels) {
            // Build weapon name
            Vector<DamageInstanceType> dt;
            DamageInstanceType di{}; createResistance(t, {1,2}, di); // reuse creator to set type (tile id optional)
            dt.push_back(di);

            Ref<AvatarQuestWeapon> w; createWeapon(dt, lvl, w);
            Ref<AvatarQuestArmor> a; createArmor(dt, lvl, a);

            REQUIRE_FALSE(w->weaponName.empty());
            REQUIRE_FALSE(a->armorName.empty());

            std::cout << "[Naming] lvl=" << lvl << " type=" << label
                      << " weapon=\"" << w->weaponName << "\""
                      << " armor=\"" << a->armorName << "\"\n";
        }
    }
}

TEST_CASE("GameArmor: createArmor populates fields", "[armor][create]") {
    Vector<DamageInstanceType> res;
    DamageInstanceType r{}; createResistance(DamageType::Damage_Slashing, { 2, 6 }, r);
    res.push_back(r);

    Ref<AvatarQuestArmor> a;
    createArmor(res, /*level*/12, a);
    REQUIRE(a != nullptr);

    REQUIRE(a->resistances.size() == 1);
    REQUIRE(a->levelRequirement == 12);

    // Armor proficiency should be assigned based on armor type
    REQUIRE(a->armorType == ArmorType::Chain);
    REQUIRE(a->armorSkillType == SkillType::HeavyArmor);

    REQUIRE(a->armorClass == 5 + 12 * 2); // 29
    REQUIRE(std::fabs(a->dodgeChance - (0.02f + 12 * 0.005f)) < 1e-6f); // ~0.08
    REQUIRE(std::fabs(a->durability - (150.0f + 12 * 12.0f)) < 1e-6f); // ~294
    // Name should be generated and non-empty
    REQUIRE_FALSE(a->armorName.empty());
}

TEST_CASE("GameArmor: createRandomArmor deterministic with seed", "[armor][random]") {
    std::srand(9876);
    Ref<AvatarQuestArmor> a1; createRandomArmor(9, a1);

    std::srand(9876);
    Ref<AvatarQuestArmor> a2; createRandomArmor(9, a2);

    REQUIRE(a1 != nullptr);
    REQUIRE(a2 != nullptr);

    REQUIRE(a1->armorSkillType == a2->armorSkillType);
    REQUIRE(a1->resistances.size() == a2->resistances.size());
    REQUIRE(a1->levelRequirement == 9);
}

TEST_CASE("Random armor: attribute bonus count is capped at 2", "[armor][random][rules]") {
    // Random attribute bonuses on armor must be 0..2
    for (int lvl : {1, 9, 17, 25}) {
        for (int seed = 200; seed < 230; ++seed) {
            std::srand(seed);
            Ref<AvatarQuestArmor> a;
            createRandomArmor(lvl, a);
            REQUIRE(a != nullptr);
            REQUIRE(a->attributeBonuses.size() <= 2);
        }
    }
}

TEST_CASE("GameArmor: console defense output for test armor", "[armor][defense][console]") {
    // Build a deterministic test armor with two resistances (reductions per hit)
    Vector<DamageInstanceType> res;
    DamageInstanceType slash{}; createResistance(DamageType::Damage_Slashing, { 2.f, 5.f }, slash);
    DamageInstanceType fire{};  createResistance(DamageType::Damage_Fire,      { 1.f, 3.f }, fire);
    res.push_back(slash);
    res.push_back(fire);

    Ref<AvatarQuestArmor> a;
    createArmor(res, /*level*/8, a);
    REQUIRE(a != nullptr);
    // Show generated armor name
    std::cout << "[ArmorName] " << a->armorName << "\n";
    REQUIRE_FALSE(a->description.empty());
    std::cout << "[ArmorDesc]\n" << a->description << "\n";

    // Fixed incoming raw damage per hit
    const float rawDamage = 25.0f;

    // Deterministic RNG
    std::mt19937 rng(1337);
    auto rollOne = [&](const DamageInstanceType& ri) -> float {
        std::uniform_real_distribution<float> dist(ri.damageRange.x, ri.damageRange.y);
        return dist(rng);
    };

    const float minRed = slash.damageRange.x + fire.damageRange.x; // 3
    const float maxRed = slash.damageRange.y + fire.damageRange.y; // 8

    // Compute and print expected values using helpers
    float expectedRed = computeExpectedReduction(*a);
    float expectedFinal = computeExpectedFinalDamage(rawDamage, *a);
    std::cout << "[ArmorExpected] AvgRed=" << expectedRed
              << ", ExpFinal=" << expectedFinal << "\n";

    // Quick sanity: expectedRed should equal average of ranges we set up
    REQUIRE(std::fabs(expectedRed - 0.5f * ((slash.damageRange.x + slash.damageRange.y) + (fire.damageRange.x + fire.damageRange.y))) < 1e-6f);

    for (int i = 0; i < 5; ++i) {
        float totalReduction = rollOne(slash) + rollOne(fire);
        float finalDamage = std::max(0.0f, rawDamage - totalReduction);
        std::cout << "[ArmorDefense] roll=" << i
                  << " reduction=" << totalReduction
                  << " final=" << finalDamage << "\n";

        // Bounds: finalDamage in [raw-maxRed, raw-minRed], clamped at 0
        REQUIRE(finalDamage <= rawDamage - minRed + 1e-3f);
        REQUIRE(finalDamage >= std::max(0.0f, rawDamage - maxRed) - 1e-3f);
    }
}

TEST_CASE("Random armor: no additive resistance below Exquisite tier", "[armor][random][rules]") {
    // Below Exquisite (band < 9 => level <= 45), random armor must have exactly one resistance
    for (int lvl : {1, 5, 10, 25, 45}) {
        for (int seed = 0; seed < 64; ++seed) {
            std::srand(seed);
            Ref<AvatarQuestArmor> a;
            createRandomArmor(lvl, a);
            REQUIRE(a != nullptr);
            REQUIRE(a->levelRequirement == lvl);
            REQUIRE(a->resistances.size() == 1);
        }
    }
}

TEST_CASE("Random armor: additive resistance only at Exquisite+ and properly scaled", "[armor][random][rules]") {
    // At Exquisite and above (band >= 9 => level >= 46):
    // - Resistances.size() is either 1 (no additive) or 2 (one additive)
    // - If present, additive must be a different type and scaled to 25%-50% of base
    auto approxEq = [](float a, float b) { return std::fabs(a - b) < 1e-5f; };

    for (int lvl : {46, 50, 60}) {
        bool sawAdditive = false;
        for (int seed = 100; seed < 260; ++seed) {
            std::srand(seed);
            Ref<AvatarQuestArmor> a;
            createRandomArmor(lvl, a);
            REQUIRE(a != nullptr);
            REQUIRE(a->levelRequirement == lvl);
            REQUIRE(a->resistances.size() >= 1);
            REQUIRE(a->resistances.size() <= 2);

            const auto& base = a->resistances[0];
            // Base range as per generator
            const float baseMin = 1.0f + lvl * 0.5f;
            const float baseMax = 3.0f + lvl * 1.5f;
            REQUIRE(approxEq(base.damageRange.x, baseMin));
            REQUIRE(approxEq(base.damageRange.y, baseMax));

            if (a->resistances.size() == 2) {
                sawAdditive = true;
                const auto& add = a->resistances[1];
                // Must be distinct type
                REQUIRE(add.damageType != base.damageType);
                // Proper scaling 25%-50% of base
                REQUIRE(approxEq(add.damageRange.x, baseMin * 0.25f));
                REQUIRE(approxEq(add.damageRange.y, baseMax * 0.5f));
            }
        }
        // Across a modest seed sweep, we should encounter at least one additive case
        REQUIRE(sawAdditive);
    }
}

#include "AvatarQuestNaming.h"
TEST_CASE("Armor tiers: sample names across all tier adjectives and armor types", "[armor][naming][tiers][console]") {
    // Pull actual tiers from naming logic to avoid hardcoding
    const auto& tiers = GetTierAdjectives();

    const Vector<std::pair<ArmorType, const char*>> types = {
        { ArmorType::Cloth,   "Cloth" },
        { ArmorType::Leather, "Leather" },
        { ArmorType::Chain,   "Chain" },
        { ArmorType::Plate,   "Plate" },
        { ArmorType::Shield,  "Shield" }
    };

    // Use a deterministic, single-resistance armor to produce names
    for (auto [atype, tname] : types) {
        for (size_t idx = 0; idx < tiers.size(); ++idx) {
            int level = static_cast<int>(idx) * 5 + 1; // band start
            Vector<DamageInstanceType> res;
            DamageInstanceType di{}; createResistance(DamageType::Damage_Slashing, { 1.f, 2.f }, di);
            res.push_back(di);

            Ref<AvatarQuestArmor> a; createArmor(res, level, atype, a);
            REQUIRE(a != nullptr);
            REQUIRE(a->armorName.rfind(tiers[idx] + String(" "), 0) == 0);
            std::cout << "[ArmorTierSample] type=" << tname
                      << " level=" << level
                      << " bandIdx=" << idx
                      << " name=" << a->armorName << "\n";
        }

        // Verify overflow suffix beyond the highest tier for each armor type
        {
            Vector<DamageInstanceType> res;
            DamageInstanceType di{}; createResistance(DamageType::Damage_Slashing, { 1.f, 2.f }, di);
            res.push_back(di);

            // +1 overflow
            {
                Ref<AvatarQuestArmor> a; createArmor(res, /*level*/90, atype, a);
                REQUIRE(a != nullptr);
                REQUIRE(a->armorName.find("Empyrean +1 ") != String::npos);
                std::cout << "[ArmorTierOverflow] type=" << tname << " level=90 name=" << a->armorName << "\n";
            }
            // +2 overflow
            {
                Ref<AvatarQuestArmor> a; createArmor(res, /*level*/95, atype, a);
                REQUIRE(a != nullptr);
                REQUIRE(a->armorName.find("Empyrean +2 ") != String::npos);
                std::cout << "[ArmorTierOverflow] type=" << tname << " level=95 name=" << a->armorName << "\n";
            }
            // +3 overflow
            {
                Ref<AvatarQuestArmor> a; createArmor(res, /*level*/100, atype, a);
                REQUIRE(a != nullptr);
                REQUIRE(a->armorName.find("Empyrean +3 ") != String::npos);
                std::cout << "[ArmorTierOverflow] type=" << tname << " level=100 name=" << a->armorName << "\n";
            }
        }
    }
}
