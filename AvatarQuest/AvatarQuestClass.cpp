#include "Common.h"
#include "AvatarQuestClass.h"
#include "AvatarQuestGameWeapon.h"
#include "AvatarQuestGameArmor.h"
#include "AvatarQuestCurrency.h"
#include <algorithm>
#include <cctype>

namespace AvatarQuest {

// --- helpers ---------------------------------------------------------------
static inline String toLowerCopy(const String& s) {
    String out = s;
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
    return out;
}

static inline float clamp01(float v) {
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

static float dot(const UMap<int, float>& a, const UMap<int, float>& b) {
    float acc = 0.0f;
    if (a.size() < b.size()) {
        for (const auto& kv : a) {
            auto it = b.find(kv.first);
            if (it != b.end()) acc += kv.second * it->second;
        }
    } else {
        for (const auto& kv : b) {
            auto it = a.find(kv.first);
            if (it != a.end()) acc += kv.second * it->second;
        }
    }
    return acc;
}

// Pick a rough dominant skill family for starter gear selection
static SkillType pickDominantSkill(const UMap<int, float>& targets) {
    float best = 0.0f;
    int bestKey = static_cast<int>(SkillType::Attack);
    for (const auto& kv : targets) {
        if (kv.second > best) { best = kv.second; bestKey = kv.first; }
    }
    return static_cast<SkillType>(bestKey);
}

// Create very simple starter gear based on dominant skill
static void giveStarterGear(CharacterSheet& cs, SkillType dominant) {
    // Weapon
    {
        Ref<AvatarQuestWeapon> w;
        createMaxDPSWeapon(/*level*/1, w);
        // Nudge names/skills to match archetype flavor
        switch (dominant) {
            case SkillType::Swords: w->secondarySkillType = SkillType::Swords; w->weaponName = "Starter Sword"; w->governingAttribute = AttributeType::Dexterity; break;
            case SkillType::Axes: w->secondarySkillType = SkillType::Axes; w->weaponName = "Starter Axe"; w->governingAttribute = AttributeType::Strength; break;
            case SkillType::Spears: w->secondarySkillType = SkillType::Spears; w->weaponName = "Starter Spear"; w->governingAttribute = AttributeType::Strength; break;
            case SkillType::Daggers: w->secondarySkillType = SkillType::Daggers; w->weaponName = "Starter Dagger"; w->governingAttribute = AttributeType::Dexterity; break;
            case SkillType::Bows: w->secondarySkillType = SkillType::Bows; w->weaponName = "Starter Bow"; w->governingAttribute = AttributeType::Dexterity; break;
            case SkillType::Staves: w->secondarySkillType = SkillType::Staves; w->weaponName = "Apprentice Staff"; w->governingAttribute = AttributeType::Intelligence; break;
            case SkillType::FireMagic: case SkillType::IceMagic: case SkillType::LightningMagic: case SkillType::PoisonMagic:
                w->secondarySkillType = SkillType::Staves; w->weaponName = "Apprentice Staff"; w->governingAttribute = AttributeType::Intelligence; break;
            case SkillType::Unarmed: w->secondarySkillType = SkillType::Unarmed; w->weaponName = "Wrapped Fists"; w->governingAttribute = AttributeType::Dexterity; break;
            default: w->secondarySkillType = SkillType::Attack; w->weaponName = "Rusty Blade"; w->governingAttribute = AttributeType::Strength; break;
        }
        equipPrimaryWeapon(cs, w);
    }
    // Light chest armor as default
    {
        Vector<DamageInstanceType> resists; resists.reserve(2);
        DamageInstanceType r1; createResistance(DamageType::Damage_Slashing, {1,2}, r1); resists.push_back(r1);
        DamageInstanceType r2; createResistance(DamageType::Damage_Piercing, {1,2}, r2); resists.push_back(r2);
        Ref<AvatarQuestArmor> a;
        createArmor(resists, /*level*/1, ArmorType::Leather, ArmorSlot::Chest, a);
        a->armorName = "Worn Leather Vest";
        a->armorSkillType = SkillType::LightArmor;
        equipArmor(cs, a);
    }
}

// --- registry --------------------------------------------------------------
static Vector<AvatarQuestClassArchetype>& registry() {
    static Vector<AvatarQuestClassArchetype> g;
    if (!g.empty()) return g;

    // Helper lambdas
    auto addArc = [&](const char* id, const char* name, const char* desc, std::initializer_list<std::pair<SkillType,float>> skills,
                      std::initializer_list<AttributeBonus> attrs, int64_t gold) {
        AvatarQuestClassArchetype a;
        a.id = id; a.name = name; a.description = desc;
        for (auto& kv : skills) a.skillTargets[static_cast<int>(kv.first)] = clamp01(kv.second);
        for (auto& b : attrs) a.attributeBonuses.push_back(b);
        if (gold > 0) createCurrency(CurrencyType::Gold, gold, /*tile*/-1, a.startingFunds);
        g.push_back(std::move(a));
    };

    // Warrior: melee strength + endurance
    addArc("warrior","Warrior","Front-line fighter favoring axes/spears and heavy defense",
        {{SkillType::Attack,0.6f},{SkillType::Axes,0.7f},{SkillType::Spears,0.5f},{SkillType::HeavyArmor,0.6f},{SkillType::Shields,0.5f}},
        {{AttributeType::Strength, +2},{AttributeType::Endurance, +2}}, 50);

    // Rogue: dexterity melee, stealth, daggers/bows
    addArc("rogue","Rogue","Agile skirmisher with daggers and stealth",
        {{SkillType::Daggers,0.8f},{SkillType::Backstab,0.8f},{SkillType::DoubleAttack,0.5f},{SkillType::Stealth,0.7f},{SkillType::Lockpicking,0.7f},{SkillType::TrapSetting,0.5f},{SkillType::Bows,0.4f},{SkillType::LightArmor,0.5f}},
        {{AttributeType::Dexterity, +2}}, 60);

    // Ranger: bows, light armor, survival-ish via speech/crafting light
    addArc("ranger","Ranger","Hunter adept with bows and light armor",
        {{SkillType::Bows,0.8f},{SkillType::Archery,0.9f},{SkillType::LightArmor,0.6f},{SkillType::TrapSetting,0.4f},{SkillType::Spears,0.3f}},
        {{AttributeType::Dexterity, +1},{AttributeType::Endurance, +1}}, 55);

    // Mage: elemental schools + intelligence
    addArc("mage","Mage","Student of the arcane with elemental focus",
        {{SkillType::FireMagic,0.7f},{SkillType::IceMagic,0.6f},{SkillType::LightningMagic,0.6f},{SkillType::Staves,0.4f}},
        {{AttributeType::Intelligence, +3}}, 40);

    // Cleric: healing, wisdom, light armor + staves
    addArc("cleric","Cleric","Devoted healer wielding light armor and staves",
        {{SkillType::Healing,0.8f},{SkillType::Staves,0.5f},{SkillType::LightArmor,0.5f}},
        {{AttributeType::Wisdom, +2},{AttributeType::Endurance, +1}}, 50);

    // Artificer: crafting/alchemy, intelligence
    addArc("artificer","Artificer","Tinker who excels at crafting and alchemy",
        {{SkillType::Crafting,0.8f},{SkillType::Alchemy,0.7f},{SkillType::TrapMaking,0.6f},{SkillType::Cooking,0.5f},{SkillType::Speech,0.3f}},
        {{AttributeType::Intelligence, +2}}, 65);

    // Brawler: unarmed, dexterity/endurance
    addArc("brawler","Brawler","Close-quarters fighter relying on fists",
        {{SkillType::Unarmed,0.8f},{SkillType::DoubleAttack,0.6f},{SkillType::Dodge,0.5f},{SkillType::LightArmor,0.3f}},
        {{AttributeType::Dexterity, +1},{AttributeType::Endurance, +2}}, 45);

    return g;
}

const Vector<AvatarQuestClassArchetype>& getAllArchetypes() { return registry(); }

const AvatarQuestClassArchetype* getArchetypeById(std::string_view id) {
    String needle = toLowerCopy(String(id));
    for (const auto& a : registry()) {
        if (toLowerCopy(a.id) == needle) return &a;
    }
    return nullptr;
}

Vector<const AvatarQuestClassArchetype*> suggestArchetypesFromSkillPrefs(const UMap<int, float>& skillPrefs, size_t maxResults) {
    struct Scored { const AvatarQuestClassArchetype* arc; float score; };
    Vector<Scored> scored; scored.reserve(registry().size());
    for (const auto& a : registry()) {
        float s = dot(skillPrefs, a.skillTargets);
        scored.push_back({ &a, s });
    }
    std::sort(scored.begin(), scored.end(), [](const Scored& l, const Scored& r){ return l.score > r.score; });
    Vector<const AvatarQuestClassArchetype*> out;
    size_t n = std::min(maxResults, scored.size());
    for (size_t i = 0; i < n; ++i) out.push_back(scored[i].arc);
    return out;
}

void applyArchetype(CharacterSheet& cs, const AvatarQuestClassArchetype& arc) {
    // Reset baseline sensible defaults
    setLevel(cs, 1);
    setExperience(cs, 0);
    setExperienceToNextLevel(cs, computeExperienceToNextLevel(cs));

    // Apply attribute bonuses relative to current values
    for (const auto& b : arc.attributeBonuses) {
        int cur = getAttribute(cs.attributes, b.type, 10);
        setAttribute(cs.attributes, b.type, cur + b.amount);
    }

    // Apply skills
    for (const auto& kv : arc.skillTargets) {
        SkillType s = static_cast<SkillType>(kv.first);
        setSkillRating(cs, s, clamp01(kv.second));
    }

    // Starting funds
    addBundle(cs.wallet, arc.startingFunds);

    // Pools
    recalcDerivedPools(cs, /*refillOnLevelUp=*/true);

    // Starter gear based on dominant skill target
    SkillType dominant = pickDominantSkill(arc.skillTargets);
    giveStarterGear(cs, dominant);
}

}
