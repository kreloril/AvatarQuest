#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "Common.h"
#include "AvatarQuestGameWeapon.h"
#include "AvatarQuestGameArmor.h"
#include "AvatarQuestSkills.h"

using namespace AvatarQuest;

TEST_CASE("Combat skills: attack and defense influence expected values", "[combat][skills]") {
    // Simple weapon: slashing and fire
    Vector<DamageInstanceType> dmg;
    DamageInstanceType slash{}; createDamageType(DamageType::Damage_Slashing, { 10.f, 10.f }, slash);
    DamageInstanceType fire{};  createDamageType(DamageType::Damage_Fire,      {  0.f,  0.f }, fire);
    dmg.push_back(slash);
    dmg.push_back(fire);

    Ref<AvatarQuestWeapon> w;
    createWeapon(dmg, /*level*/10, w);
    REQUIRE(w);

    // Base expected per hit (no skill scaling)
    const float baseHit = computeExpectedPerHit(*w);
    // Build an attacker sheet with some Attack rating
    CharacterSheet attacker{}; attacker.skillRatings[(int)SkillType::Attack] = 0.5f;
    // Install Attack evaluator for the test (no global defaults)
    clearAllSkillHandlers();
    registerSkillEvaluator(SkillType::Attack, [](SkillType, float input, const CharacterSheet& cs, float difficulty){
        if (difficulty < 0.0f) difficulty = 0.0f;
        if (difficulty > 1.0f) difficulty = 1.0f;
        float r = skillRating(cs, SkillType::Attack, 0.0f);
        return input * (1.0f + 0.25f * (r - difficulty));
    });
    // Attack skill scales by up to +25% linearly with rating via registered handler
    const float scaled = applyAttackSkillToDamage(*w, attacker, baseHit);
    REQUIRE(scaled >= baseHit);

    // Difficulty-aware scaling: higher difficulty should reduce the effect
    const float easy = applyAttackSkillToDamage(*w, attacker, baseHit, /*difficulty*/0.0f);
    const float hard = applyAttackSkillToDamage(*w, attacker, baseHit, /*difficulty*/0.8f);
    REQUIRE(easy >= scaled - 1e-6f);
    REQUIRE(hard <= scaled + 1e-6f);
    // At high difficulty > rating, factor can be less than 1.0, reducing below base
    REQUIRE(hard <= baseHit);
    // With difficulty above rating, effect should reduce below the no-difficulty scaling
    const float scaledHard = applyAttackSkillToDamage(*w, attacker, baseHit, /*difficulty*/0.75f);
    REQUIRE(scaledHard < scaled);
    // And it can dip below base when difficulty significantly exceeds rating
    const float scaledVeryHard = applyAttackSkillToDamage(*w, attacker, baseHit, /*difficulty*/1.0f);
    REQUIRE(scaledVeryHard <= baseHit + 1e-6f);

    // Armor with decent block
    Vector<DamageInstanceType> res;
    DamageInstanceType r{}; createResistance(DamageType::Damage_Slashing, { 2.f, 2.f }, r);
    res.push_back(r);

    Ref<AvatarQuestArmor> a;
    createArmor(res, /*level*/15, ArmorType::Plate, a);
    REQUIRE(a);

    const float raw = 20.0f;
    const float noSkills = computeExpectedFinalDamage(raw, *a);
    // Wearer without proficiency -> should gate resistances off
    CharacterSheet wearerNo{}; wearerNo.skillRatings[(int)SkillType::HeavyArmor] = 0.0f;
    CharacterSheet wearerYes{}; wearerYes.skillRatings[(int)SkillType::HeavyArmor] = 1.0f;
    const float withSkillsNo = computeExpectedFinalDamageWithSkills(raw, *a, wearerNo);
    const float withSkillsYes = computeExpectedFinalDamageWithSkills(raw, *a, wearerYes);

    // Defensive proficiency should not increase expected damage; with proficiency should be <= baseline
    REQUIRE(withSkillsYes <= noSkills + 1e-4f);
    // Without proficiency, expect equal or worse than baseline (no resistance benefit)
    REQUIRE(withSkillsNo >= noSkills - 1e-4f);

    // Deterministic skillCheck using explicit roll (default checker)
    REQUIRE(skillCheck(SkillType::Attack, attacker, /*difficulty*/0.4f, /*roll*/0.2f) == true);
    REQUIRE(skillCheck(SkillType::Attack, attacker, /*difficulty*/0.9f, /*roll*/0.9f) == false);
}

TEST_CASE("applyAttackSkillToDamage scales with governing attribute multiplier", "[combat][skills][attributes]") {
    // Build two single-type weapons that map to different governing attributes via skill
    Vector<DamageInstanceType> dmgSlash; // Swords -> Dexterity
    DamageInstanceType slash{}; createDamageType(DamageType::Damage_Slashing, { 10.f, 10.f }, slash);
    dmgSlash.push_back(slash);
    Ref<AvatarQuestWeapon> wDex; createWeapon(dmgSlash, /*level*/5, wDex);
    REQUIRE(wDex);
    REQUIRE(wDex->secondarySkillType == SkillType::Swords);

    Vector<DamageInstanceType> dmgBlunt; // Staves -> Strength
    DamageInstanceType blunt{}; createDamageType(DamageType::Damage_Bludgeoning, { 10.f, 10.f }, blunt);
    dmgBlunt.push_back(blunt);
    Ref<AvatarQuestWeapon> wStr; createWeapon(dmgBlunt, /*level*/5, wStr);
    REQUIRE(wStr);
    REQUIRE(wStr->secondarySkillType == SkillType::Staves);

    // Base expected per hit is the same for both (10 flat damage)
    const float baseHit = computeExpectedPerHit(*wDex);
    REQUIRE(baseHit == Catch::Approx(computeExpectedPerHit(*wStr)).margin(1e-6f));

    // Install an Attack evaluator that returns the input unchanged to isolate attribute multiplier
    clearAllSkillHandlers();
    registerSkillEvaluator(SkillType::Attack, [](SkillType, float input, const CharacterSheet&, float){
        return input; // identity scaling for test
    });

    // Attacker with high Dexterity and baseline Strength
    CharacterSheet dexFav{};
    setAttribute(dexFav.attributes, AttributeType::Dexterity, 14); // +4 mod => 1.20x
    setAttribute(dexFav.attributes, AttributeType::Strength, 10);  // +0 mod => 1.00x

    float dmgDexFav_Dex = applyAttackSkillToDamage(*wDex, dexFav, baseHit);
    float dmgDexFav_Str = applyAttackSkillToDamage(*wStr, dexFav, baseHit);
    // Expect 1.20x vs 1.00x
    REQUIRE(dmgDexFav_Dex == Catch::Approx(baseHit * 1.20f).margin(1e-6f));
    REQUIRE(dmgDexFav_Str == Catch::Approx(baseHit * 1.00f).margin(1e-6f));
    REQUIRE(dmgDexFav_Dex > dmgDexFav_Str);

    // Attacker with high Strength and baseline Dexterity
    CharacterSheet strFav{};
    setAttribute(strFav.attributes, AttributeType::Dexterity, 10);
    setAttribute(strFav.attributes, AttributeType::Strength, 14); // +4 mod => 1.20x

    float dmgStrFav_Dex = applyAttackSkillToDamage(*wDex, strFav, baseHit);
    float dmgStrFav_Str = applyAttackSkillToDamage(*wStr, strFav, baseHit);
    REQUIRE(dmgStrFav_Dex == Catch::Approx(baseHit * 1.00f).margin(1e-6f));
    REQUIRE(dmgStrFav_Str == Catch::Approx(baseHit * 1.20f).margin(1e-6f));
    REQUIRE(dmgStrFav_Str > dmgStrFav_Dex);
}
