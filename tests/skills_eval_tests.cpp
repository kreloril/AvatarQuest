#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "Common.h"
#include "AvatarQuestSkills.h"
#include "AvatarQuestRP.h"

using namespace AvatarQuest;

TEST_CASE("Skill evaluation: difficulty-aware Attack factor matches formula", "[skills][evaluate]") {
    clearAllSkillHandlers();
    // Register standard Attack evaluator used across combat
    registerSkillEvaluator(SkillType::Attack, [](SkillType, float input, const CharacterSheet& cs, float difficulty){
        if (difficulty < 0.0f) difficulty = 0.0f;
        if (difficulty > 1.0f) difficulty = 1.0f;
        float r = skillRating(cs, SkillType::Attack, 0.0f);
        return input * (1.0f + 0.25f * (r - difficulty));
    });
    CharacterSheet cs{};
    cs.skillRatings[(int)SkillType::Attack] = 0.5f; // 50%
    const float base = 100.0f;

    // difficulty = 0 -> factor = 1 + 0.25*(0.5 - 0) = 1.125
    float outEasy = evaluateSkillEffect(SkillType::Attack, base, cs, 0.0f);
    REQUIRE(outEasy == Catch::Approx(112.5f).margin(1e-4f));

    // difficulty = 0.8 -> factor = 1 + 0.25*(0.5 - 0.8) = 0.925
    float outHard = evaluateSkillEffect(SkillType::Attack, base, cs, 0.8f);
    REQUIRE(outHard == Catch::Approx(92.5f).margin(1e-4f));
}

TEST_CASE("Skill evaluation: unregistered skill passthrough by default", "[skills][evaluate]") {
    clearAllSkillHandlers(); // ensure no handlers registered
    CharacterSheet cs{}; // no rating set for Smithing
    const float base = 42.0f;
    float out = evaluateSkillEffect(SkillType::Smithing, base, cs, 0.9f);
    // Default is passthrough for unregistered skills
    REQUIRE(out == Catch::Approx(base).margin(1e-6f));
}

TEST_CASE("Skill registry: custom evaluator overrides default", "[skills][evaluate][registry]") {
    clearAllSkillHandlers();
    // Register custom evaluator for Swords that scales by +10% at rating=1.0
    registerSkillEvaluator(SkillType::Swords, [](SkillType, float input, const CharacterSheet& cs, float difficulty){
        (void)difficulty; // ignore difficulty in this custom rule
        float r = skillRating(cs, SkillType::Swords, 0.0f);
        return input * (1.0f + 0.10f * r);
    });

    CharacterSheet cs{};
    cs.skillRatings[(int)SkillType::Swords] = 0.7f; // 70%

    const float base = 50.0f;
    float out = evaluateSkillEffect(SkillType::Swords, base, cs, 0.5f);
    // 10% * 0.7 = 7% boost
    REQUIRE(out == Catch::Approx(53.5f).margin(1e-4f));
}

TEST_CASE("Skill check: default probability rule", "[skills][check]") {
    clearAllSkillHandlers();
    CharacterSheet cs{}; cs.skillRatings[(int)SkillType::Parry] = 0.3f;
    // prob = clamp01(0.5 + (0.3 - 0.2)) = 0.6 -> roll 0.55 succeeds, 0.65 fails
    REQUIRE(skillCheck(SkillType::Parry, cs, 0.2f, 0.55f) == true);
    REQUIRE(skillCheck(SkillType::Parry, cs, 0.2f, 0.65f) == false);
}

TEST_CASE("Skill check: custom checker overrides default", "[skills][check][registry]") {
    clearAllSkillHandlers();
    // Custom checker: success if rating > difficulty regardless of roll
    registerSkillChecker(SkillType::Dodge, [](SkillType, const CharacterSheet& cs, float difficulty, float roll){
        (void)roll; return skillRating(cs, SkillType::Dodge, 0.0f) > difficulty; 
    });
    CharacterSheet cs{}; cs.skillRatings[(int)SkillType::Dodge] = 0.4f;
    REQUIRE(skillCheck(SkillType::Dodge, cs, 0.3f, 0.99f) == true);
    REQUIRE(skillCheck(SkillType::Dodge, cs, 0.5f, 0.01f) == false);
}
