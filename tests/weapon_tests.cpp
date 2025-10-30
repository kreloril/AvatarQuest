#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "Common.h"
#include "AvatarQuestGameWeapon.h"
#include <random>
#include "AvatarQuestNaming.h"
#include <iostream>
#include <cstring>
#include <cstdlib>

using namespace AvatarQuest;

TEST_CASE("GameWeapon: createDamageType uses models for tileID", "[weapon][damage]") {
    // Prepare a couple of damage models with tile IDs
    DamageInstanceType models[2];
    models[0].damageType = DamageType::Damage_Fire;   models[0].tileID = 42;  models[0].damageRange = { 1, 2 };
    models[1].damageType = DamageType::Damage_Ice;    models[1].tileID = 77;  models[1].damageRange = { 3, 4 };

    createDamageTypes(models, 2);

    DamageInstanceType out{};
    createDamageType(DamageType::Damage_Fire, { 10, 20 }, out);
    REQUIRE(out.damageType == DamageType::Damage_Fire);
    REQUIRE(out.damageRange.x == 10.0f);
    REQUIRE(out.damageRange.y == 20.0f);
    REQUIRE(out.tileID == 42); // picked from model

    createDamageType(DamageType::Damage_Poison, { 5, 6 }, out);
    REQUIRE(out.damageType == DamageType::Damage_Poison);
    REQUIRE(out.tileID == -1); // not in models
}

TEST_CASE("Weapon: description samples for all damage types", "[weapon][description][console]") {
    using Pair = std::pair<DamageType, const char*>;
    Vector<Pair> types = {
        {DamageType::Damage_Fire, "Fire"},
        {DamageType::Damage_Ice, "Ice"},
        {DamageType::Damage_Lightning, "Lightning"},
        {DamageType::Damage_Poison, "Poison"},
        {DamageType::Damage_Bleeding, "Bleeding"},
        {DamageType::Damage_Piercing, "Piercing"},
        {DamageType::Damage_Slashing, "Slashing"},
        {DamageType::Damage_Bludgeoning, "Bludgeoning"},
        {DamageType::Damage_Magical, "Magical"}
    };
    Vector<int> levels = {1, 10, 25};

    for (auto [t, label] : types) {
        for (int lvl : levels) {
            Vector<DamageInstanceType> dmg;
            DamageInstanceType primary{}; createDamageType(t, { 5.f, 10.f }, primary);
            DamageInstanceType extra{};   createDamageType(DamageType::Damage_Slashing, { 2.f, 6.f }, extra);
            dmg.push_back(primary);
            dmg.push_back(extra);

            Ref<AvatarQuestWeapon> w; createWeapon(dmg, lvl, w);
            REQUIRE(w != nullptr);
            REQUIRE_FALSE(w->description.empty());

            // Basic sanity of description contains key labels
            REQUIRE(w->description.find("Level ") != String::npos);
            REQUIRE(w->description.find("ASpd=") != String::npos);
            REQUIRE(w->description.find("Reach=") != String::npos);
            REQUIRE(w->description.find("Crit=") != String::npos);
            REQUIRE(w->description.find("Durability=") != String::npos);
            REQUIRE(w->description.find("AvgDmg=") != String::npos);
            REQUIRE(w->description.find("DPS=") != String::npos);
            REQUIRE(w->description.find("Damage:") != String::npos);

            // Verify AvgDmg and DPS numeric values match our calculation helpers
            const std::string& desc = w->description;
            auto findToken = [&](const char* token, char endDelim) -> float {
                size_t p = desc.find(token);
                REQUIRE(p != std::string::npos);
                p += std::strlen(token);
                size_t e = desc.find(endDelim, p);
                if (e == std::string::npos) e = desc.size();
                std::string num = desc.substr(p, e - p);
                return std::strtof(num.c_str(), nullptr);
            };
            float avgParsed = findToken("AvgDmg=", ',');
            float dpsParsed = findToken("DPS=", '\n');

            float avgExpected = computeExpectedPerHit(*w);
            float dpsExpected = computeDPS(*w);
            REQUIRE(avgParsed == Catch::Approx(avgExpected).margin(1e-4f));
            REQUIRE(dpsParsed == Catch::Approx(dpsExpected).margin(0.01f)); // 2-decimal rounding in description

            std::cout << "[WeaponDescSample] type=" << label
                      << " lvl=" << lvl << "\n"
                      << w->description << "\n";
        }
    }
}

TEST_CASE("GameWeapon: createWeapon populates fields", "[weapon][create]") {
    Vector<DamageInstanceType> dmg;
    DamageInstanceType d{};
    createDamageType(DamageType::Damage_Slashing, { 5, 15 }, d);
    dmg.push_back(d);

    Ref<AvatarQuestWeapon> w;
    createWeapon(dmg, /*level*/10, w);
    REQUIRE(w != nullptr);

    // Skill types should be set by default
    REQUIRE(w->primarySkillType == SkillType::Attack);
    // Slashing maps to Swords as the weapon skill
    REQUIRE(w->secondarySkillType == SkillType::Swords);
    REQUIRE(w->damageTypes.size() == 1);
    REQUIRE(w->levelRequirement == 10);

    // Derived fields
    REQUIRE(std::fabs(w->attackSpeed - (1.0f + 10 * 0.1f)) < 1e-6f); // ~2.0
    REQUIRE(w->attackReach == 1 + (10 / 5));                        // 3
    REQUIRE(std::fabs(w->criticalChance - (0.05f + 10 * 0.01f)) < 1e-6f); // ~0.15
    REQUIRE(std::fabs(w->durability - (100.0f + 10 * 10.0f)) < 1e-6f);    // ~200

    // Name should be generated and non-empty
    REQUIRE_FALSE(w->weaponName.empty());
}

TEST_CASE("GameWeapon: createRandomWeapon deterministic with seed", "[weapon][random]") {
    // Fix the seed to make results deterministic across runs
    std::srand(12345);

    Ref<AvatarQuestWeapon> w1;
    createRandomWeapon(7, w1);

    std::srand(12345);
    Ref<AvatarQuestWeapon> w2;
    createRandomWeapon(7, w2);

    REQUIRE(w1 != nullptr);
    REQUIRE(w2 != nullptr);

    // Same seed, same results
    // Same seed, same results -> names should match
    REQUIRE(w1->weaponName == w2->weaponName);
    REQUIRE(w1->damageTypes.size() == w2->damageTypes.size());
    REQUIRE(w1->levelRequirement == 7);
}

TEST_CASE("Random weapon: below Exquisite has no additive damage type", "[weapon][random][rules]") {
    // Levels < 46 (band < 9) must never receive an additive damage type
    int level = 30;
    // Try a range of seeds to be robust; behavior should always be exactly 1 damage type
    for (int seed = 0; seed < 25; ++seed) {
        std::srand(seed);
        Ref<AvatarQuestWeapon> w;
        createRandomWeapon(level, w);
        REQUIRE(w != nullptr);
        REQUIRE(w->damageTypes.size() == 1);
    }
}

TEST_CASE("Random weapon: Exquisite+ may add at most one additive type (distinct)", "[weapon][random][rules]") {
    // Levels >= 46 (band >= 9) may get a single additive type by RNG, but never more than one
    int level = 50;
    // Validate across a spread of seeds
    for (int seed = 100; seed < 150; ++seed) {
        std::srand(seed);
        Ref<AvatarQuestWeapon> w;
        createRandomWeapon(level, w);
        REQUIRE(w != nullptr);
        REQUIRE(w->damageTypes.size() >= 1);
        REQUIRE(w->damageTypes.size() <= 2);
        if (w->damageTypes.size() == 2) {
            REQUIRE(w->damageTypes[0].damageType != w->damageTypes[1].damageType);
        }
    }
}

TEST_CASE("Random weapon: exactly two skills and correct governing attribute", "[weapon][random][skills]") {
    std::srand(4242);
    Ref<AvatarQuestWeapon> w;
    createRandomWeapon(12, w);
    REQUIRE(w != nullptr);
    // Primary is always Attack; secondary is a concrete weapon skill
    REQUIRE(w->primarySkillType == SkillType::Attack);
    REQUIRE(w->secondarySkillType != SkillType::None);
    // Governing attribute must match secondary skill mapping
    REQUIRE(w->governingAttribute == governingAttributeForSkill(w->secondarySkillType));
}

TEST_CASE("Random weapon: name includes attribute epithet", "[weapon][naming]") {
    std::srand(777);
    Ref<AvatarQuestWeapon> w;
    createRandomWeapon(9, w);
    REQUIRE(w != nullptr);
    // Ensure the name carries an " of <Attr>" epithet (we don't assert exact attribute text here)
    REQUIRE(w->weaponName.find(" of ") != std::string::npos);
}

TEST_CASE("GameWeapon: console damage output for test weapon", "[weapon][damage][console]") {
    // Build a deterministic test weapon with two damage types
    Vector<DamageInstanceType> dmg;
    DamageInstanceType slash{}; createDamageType(DamageType::Damage_Slashing, { 5.f, 15.f }, slash);
    DamageInstanceType fire{};  createDamageType(DamageType::Damage_Fire,      { 2.f,  6.f }, fire);
    dmg.push_back(slash);
    dmg.push_back(fire);

    Ref<AvatarQuestWeapon> w;
    createWeapon(dmg, /*level*/8, w);
    REQUIRE(w != nullptr);
    // Show generated weapon name
    std::cout << "[WeaponName] " << w->weaponName << "\n";
    REQUIRE_FALSE(w->description.empty());
    std::cout << "[WeaponDesc]\n" << w->description << "\n";

    // Deterministic RNG for reproducible output
    std::mt19937 rng(1337);
    auto rollOne = [&](const DamageInstanceType& di) -> float {
        std::uniform_real_distribution<float> dist(di.damageRange.x, di.damageRange.y);
        return dist(rng);
    };
    auto rollCrit = [&]() -> bool {
        std::uniform_real_distribution<float> dist01(0.0f, 1.0f);
        return dist01(rng) < w->criticalChance;
    };

    const float minSum = slash.damageRange.x + fire.damageRange.x;
    const float maxSum = slash.damageRange.y + fire.damageRange.y;
    const float critMul = 1.5f;

    // Roll a few samples and print them; also assert reasonable bounds
    for (int i = 0; i < 5; ++i) {
        float total = rollOne(slash) + rollOne(fire);
        bool crit = rollCrit();
        float finalDmg = crit ? total * critMul : total;
        // Print to console (visible with verbose ctest, and in most IDE test runners)
        std::cout << "[WeaponDamage] roll=" << i
                  << " total=" << total
                  << (crit ? " (CRIT)" : "")
                  << " final=" << finalDmg << "\n";

        REQUIRE(finalDmg >= minSum);
        REQUIRE(finalDmg <= maxSum * critMul + 1e-3f);
    }
}

TEST_CASE("Weapon: Max DPS factory builds strictly higher DPS than random at same level", "[weapon][dps]") {
    // Choose a representative level
    int level = 20;

    // Create max DPS weapon
    Ref<AvatarQuestWeapon> wMax;
    createMaxDPSWeapon(level, wMax);
    REQUIRE(wMax != nullptr);

    auto computeDps = [](const Ref<AvatarQuestWeapon>& w) {
        return computeDPS(*w);
    };

    // Build a few random weapons at same level and ensure max DPS beats them
    std::srand(1337);
    for (int i = 0; i < 3; ++i) {
        Ref<AvatarQuestWeapon> wr;
        createRandomWeapon(level, wr);
        REQUIRE(wr != nullptr);
        REQUIRE(computeDps(wMax) > computeDps(wr));
    }

    // Print the max DPS weapon for visibility
    std::cout << "[WeaponMaxDPS] level=" << level << "\n" << wMax->description << "\n";
}

TEST_CASE("GameWeapon: top-level tileID comes from first damage type", "[weapon][tile]") {
    Vector<DamageInstanceType> dmg;
    DamageInstanceType first{}; first.damageType = DamageType::Damage_Fire; first.damageRange = { 1, 2 }; first.tileID = 321;
    DamageInstanceType second{}; second.damageType = DamageType::Damage_Ice; second.damageRange = { 3, 4 }; second.tileID = 654;
    dmg.push_back(first);
    dmg.push_back(second);

    Ref<AvatarQuestWeapon> w; createWeapon(dmg, /*level*/3, w);
    REQUIRE(w != nullptr);
    REQUIRE(w->tileID == 321);
}

TEST_CASE("Weapon tiers: sample names across all tier adjectives", "[weapon][naming][tiers][console]") {
    // Use the production tier list directly to avoid drift
    const auto& tiers = GetTierAdjectives();

    // Use a deterministic, non-random weapon constructed from a single damage type
    for (size_t idx = 0; idx < tiers.size(); ++idx) {
        int level = static_cast<int>(idx) * 5 + 1; // band start: 1,6,11,...,81
        Vector<DamageInstanceType> dmg;
        DamageInstanceType di{}; createDamageType(DamageType::Damage_Slashing, { 5.f, 10.f }, di);
        dmg.push_back(di);

        Ref<AvatarQuestWeapon> w; createWeapon(dmg, level, w);
        REQUIRE(w != nullptr);
        // Name should start with the expected tier adjective
        REQUIRE(w->weaponName.rfind(tiers[idx] + String(" "), 0) == 0);
        // Print a concise sample for logs
        std::cout << "[WeaponTierSample] level=" << level
                  << " bandIdx=" << idx
                  << " name=" << w->weaponName << "\n";
    }

    // Also verify overflow suffix increments beyond the highest tier
    {
        Vector<DamageInstanceType> dmg;
        DamageInstanceType di{}; createDamageType(DamageType::Damage_Slashing, { 5.f, 10.f }, di);
        dmg.push_back(di);

        // +1 overflow (levels 86..90) -> "Empyrean +1"
        {
            Ref<AvatarQuestWeapon> w; createWeapon(dmg, /*level*/90, w);
            REQUIRE(w != nullptr);
            REQUIRE(w->weaponName.find("Empyrean +1 ") != String::npos);
            std::cout << "[WeaponTierOverflow] level=90 name=" << w->weaponName << "\n";
        }
        // +2 overflow (91..95)
        {
            Ref<AvatarQuestWeapon> w; createWeapon(dmg, /*level*/95, w);
            REQUIRE(w != nullptr);
            REQUIRE(w->weaponName.find("Empyrean +2 ") != String::npos);
            std::cout << "[WeaponTierOverflow] level=95 name=" << w->weaponName << "\n";
        }
        // +3 overflow (96..100)
        {
            Ref<AvatarQuestWeapon> w; createWeapon(dmg, /*level*/100, w);
            REQUIRE(w != nullptr);
            REQUIRE(w->weaponName.find("Empyrean +3 ") != String::npos);
            std::cout << "[WeaponTierOverflow] level=100 name=" << w->weaponName << "\n";
        }
    }
}
