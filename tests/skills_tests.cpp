#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "Common.h"
#include "AvatarQuestSkills.h"
#include <unordered_set>

using namespace AvatarQuest;

TEST_CASE("Skills: round-trip string conversions", "[skills]") {
    // None should parse and stringify
    SkillType s = SkillType::None;
    REQUIRE(std::string(skillTypeToString(s)) == "None");

    SkillType out;
    REQUIRE(skillTypeFromString("None", out));
    REQUIRE(out == SkillType::None);
    REQUIRE(skillTypeFromString("none", out));
    REQUIRE(out == SkillType::None);

    // All listed skills should round-trip and be unique by name
    std::unordered_set<std::string> seen;
    for (SkillType sk : allSkillTypes()) {
        const char* n = skillTypeToString(sk);
        REQUIRE(n != nullptr);
        REQUIRE(std::strlen(n) > 0);
        REQUIRE_FALSE(std::string(n) == "None");
        REQUIRE(seen.insert(n).second); // name uniqueness

        SkillType parsed;
        REQUIRE(skillTypeFromString(n, parsed));
        REQUIRE(parsed == sk);

        // Case-insensitive parse
        std::string lower = n;
        std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c){ return (char)std::tolower(c); });
        REQUIRE(skillTypeFromString(lower.c_str(), parsed));
        REQUIRE(parsed == sk);
    }
}

TEST_CASE("Skills: counts and ids are sane", "[skills]") {
    REQUIRE(skillTypeCount() == (int)allSkillTypes().size());

    // Basic id sanity: None == 0, COUNT is after all last exposed skill id
    REQUIRE(toInt(SkillType::None) == 0);
    REQUIRE(toInt(SkillType::COUNT) > 0);

    // The ids for listed skills should be within [1, COUNT)
    for (SkillType sk : allSkillTypes()) {
        int id = toInt(sk);
        REQUIRE(id >= 1);
        REQUIRE(id < toInt(SkillType::COUNT));
    }
}

TEST_CASE("Skills: create and execute", "[skills]") {
    Ref<AvatarQuestSkill> sk;
    createSkill(SkillType::Swords, /*level*/5, /*rating*/0.8f, sk);
    REQUIRE(sk);
    REQUIRE(sk->type == SkillType::Swords);
    REQUIRE(sk->levelRequirement == 5);
    REQUIRE(sk->rating == Catch::Approx(0.8f));
    REQUIRE(std::string(sk->name).find("Swords") != std::string::npos);

    // Override execute to custom behavior
    sk->execute = [](const AvatarQuestSkill& s) {
        SkillUseResult r;
        r.success = true;
        r.value = s.rating * 100.0f + s.levelRequirement; // arbitrary formula
        r.message = String("Swing ") + s.name;
        return r;
    };

    SkillUseResult res = useSkill(*sk);
    REQUIRE(res.success);
    REQUIRE(res.value == Catch::Approx(0.8f * 100.0f + 5.0f));
    REQUIRE(res.message.find("Swing") != std::string::npos);
}
