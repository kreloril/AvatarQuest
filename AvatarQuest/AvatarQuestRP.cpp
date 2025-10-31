#include "Common.h"
#include "AvatarQuestRP.h"
#include "AvatarQuestGameArmor.h" // for ArmorSlot
#include "AvatarQuestGameWeapon.h"
#include "AvatarQuestSkills.h" // for SkillType values

namespace AvatarQuest {

const char* attributeTypeToString(AttributeType a) {
	switch (a) {
		case AttributeType::Strength: return "Strength";
		case AttributeType::Dexterity: return "Dexterity";
		case AttributeType::Intelligence: return "Intelligence";
		case AttributeType::Endurance: return "Endurance";
		case AttributeType::Wisdom: return "Wisdom";
        case AttributeType::Level: return "Level";
        case AttributeType::HP: return "HP";
        case AttributeType::HPMax: return "HPMax";
        case AttributeType::MP: return "MP";
        case AttributeType::MPMax: return "MPMax";
        case AttributeType::Experience: return "Experience";
        case AttributeType::ExperienceToNextLevel: return "ExperienceToNextLevel";
		default: return "Attribute";
	}
}

bool passesAttributeCheck(const CharacterAttributes& attrs, AttributeType governing, int requiredLevel) {
	int value = getAttribute(attrs, governing, 10);
	return value >= requiredLevel;
}

int getAttribute(const CharacterAttributes& attrs, AttributeType a, int defaultValue) {
    int key = static_cast<int>(a);
    auto it = attrs.values.find(key);
    if (it != attrs.values.end()) return it->second;
    return defaultValue;
}

void setAttribute(CharacterAttributes& attrs, AttributeType a, int value) {
    int key = static_cast<int>(a);
    attrs.values[key] = value;
}

// Forward declare SkillType to avoid including skills header here; defined in RP.h
float getSkillRating(const CharacterSheet& cs, SkillType s, float defaultValue) {
    int key = static_cast<int>(s);
    auto it = cs.skillRatings.find(key);
    if (it != cs.skillRatings.end()) return it->second;
    return defaultValue;
}

void setSkillRating(CharacterSheet& cs, SkillType s, float rating) {
    int key = static_cast<int>(s);
    cs.skillRatings[key] = rating;
}

void equipPrimaryWeapon(CharacterSheet& cs, const Ref<AvatarQuestWeapon>& w) {
    cs.equipment.primaryWeapon = w;
}

void equipSecondaryWeapon(CharacterSheet& cs, const Ref<AvatarQuestWeapon>& w) {
    cs.equipment.secondaryWeapon = w;
}

const Ref<AvatarQuestWeapon>& getPrimaryWeapon(const CharacterSheet& cs) {
    return cs.equipment.primaryWeapon;
}

const Ref<AvatarQuestWeapon>& getSecondaryWeapon(const CharacterSheet& cs) {
    return cs.equipment.secondaryWeapon;
}

void equipArmor(CharacterSheet& cs, const Ref<AvatarQuestArmor>& a) {
    if (!a) return;
    switch (a->slot) {
        case ArmorSlot::Head: cs.equipment.head = a; break;
        case ArmorSlot::Chest: cs.equipment.chest = a; break;
        case ArmorSlot::Feet: cs.equipment.feet = a; break;
        case ArmorSlot::Hands: cs.equipment.hands = a; break;
        case ArmorSlot::Shoulders: cs.equipment.shoulders = a; break;
        default: break;
    }
}

Ref<AvatarQuestArmor> getArmor(const CharacterSheet& cs, ArmorSlot slot) {
    switch (slot) {
        case ArmorSlot::Head: return cs.equipment.head;
        case ArmorSlot::Chest: return cs.equipment.chest;
        case ArmorSlot::Feet: return cs.equipment.feet;
        case ArmorSlot::Hands: return cs.equipment.hands;
        case ArmorSlot::Shoulders: return cs.equipment.shoulders;
        default: return nullptr;
    }
}

int attributeModifier(int attributeScore) {
    // Center at 10; 1 point = +/-1 modifier
    return attributeScore - 10;
}

float attributeMultiplier(const CharacterSheet& cs, AttributeType a) {
    const int val = getAttribute(cs.attributes, a, 10);
    const int mod = attributeModifier(val);
    // 5% per point over/under 10
    return 1.0f + 0.05f * static_cast<float>(mod);
}

float attributeMultiplierEffective(const CharacterSheet& cs, AttributeType a) {
    const int val = getEffectiveAttribute(cs, a);
    const int mod = attributeModifier(val);
    return 1.0f + 0.05f * static_cast<float>(mod);
}

}

namespace AvatarQuest {

// Helpers to access pools/progression stored in attributes map with sensible defaults
int getLevel(const CharacterSheet& cs) { return getAttribute(cs.attributes, AttributeType::Level, 1); }
void setLevel(CharacterSheet& cs, int value) { setAttribute(cs.attributes, AttributeType::Level, value); }

int getHP(const CharacterSheet& cs) { return getAttribute(cs.attributes, AttributeType::HP, 0); }
void setHP(CharacterSheet& cs, int value) { setAttribute(cs.attributes, AttributeType::HP, value); }

int getHPMax(const CharacterSheet& cs) { return getAttribute(cs.attributes, AttributeType::HPMax, 0); }
void setHPMax(CharacterSheet& cs, int value) { setAttribute(cs.attributes, AttributeType::HPMax, value); }

int getMP(const CharacterSheet& cs) { return getAttribute(cs.attributes, AttributeType::MP, 0); }
void setMP(CharacterSheet& cs, int value) { setAttribute(cs.attributes, AttributeType::MP, value); }

int getMPMax(const CharacterSheet& cs) { return getAttribute(cs.attributes, AttributeType::MPMax, 0); }
void setMPMax(CharacterSheet& cs, int value) { setAttribute(cs.attributes, AttributeType::MPMax, value); }

int getExperience(const CharacterSheet& cs) { return getAttribute(cs.attributes, AttributeType::Experience, 0); }
void setExperience(CharacterSheet& cs, int value) { setAttribute(cs.attributes, AttributeType::Experience, value); }

int getExperienceToNextLevel(const CharacterSheet& cs) { return getAttribute(cs.attributes, AttributeType::ExperienceToNextLevel, 100); }
void setExperienceToNextLevel(CharacterSheet& cs, int value) { setAttribute(cs.attributes, AttributeType::ExperienceToNextLevel, value); }

// Sum equipment bonuses for a given attribute
static int sumEquipmentBonuses(const CharacterSheet& cs, AttributeType a) {
    int total = 0;
    auto addFromWeapon = [&](const Ref<AvatarQuestWeapon>& w) {
        if (!w) return;
        for (const auto& b : w->attributeBonuses) {
            if (b.type == a) total += b.amount;
        }
    };
    addFromWeapon(getPrimaryWeapon(cs));
    addFromWeapon(getSecondaryWeapon(cs));
    auto addFromArmor = [&](const Ref<AvatarQuestArmor>& ar) {
        if (!ar) return;
        for (const auto& b : ar->attributeBonuses) {
            if (b.type == a) total += b.amount;
        }
    };
    addFromArmor(getArmor(cs, ArmorSlot::Head));
    addFromArmor(getArmor(cs, ArmorSlot::Chest));
    addFromArmor(getArmor(cs, ArmorSlot::Feet));
    addFromArmor(getArmor(cs, ArmorSlot::Hands));
    addFromArmor(getArmor(cs, ArmorSlot::Shoulders));
    return total;
}

int getEffectiveAttribute(const CharacterSheet& cs, AttributeType a) {
    int baseDefault = 10;
    // Pools/progression attributes have different defaults
    switch (a) {
        case AttributeType::Level: baseDefault = 1; break;
        case AttributeType::HP:
        case AttributeType::HPMax:
        case AttributeType::MP:
        case AttributeType::MPMax:
        case AttributeType::Experience:
        case AttributeType::ExperienceToNextLevel:
            baseDefault = (a == AttributeType::ExperienceToNextLevel) ? 100 : 0;
            break;
        default:
            baseDefault = 10; break;
    }
    const int base = getAttribute(cs.attributes, a, baseDefault);
    const int bonus = sumEquipmentBonuses(cs, a);
    return base + bonus;
}

AttributeType governingAttributeForSkill(SkillType type) {
    switch (type) {
        case SkillType::Attack:
        case SkillType::Axes:
        case SkillType::Spears:
        case SkillType::Staves:
        case SkillType::HeavyArmor:
        case SkillType::Shields:
            return AttributeType::Strength;
        case SkillType::Swords:
        case SkillType::Daggers:
        case SkillType::Bows:
        case SkillType::Unarmed:
        case SkillType::Dodge:
        case SkillType::Parry:
        case SkillType::Riposte:
        case SkillType::LightArmor:
        case SkillType::Stealth:
            return AttributeType::Dexterity;
        case SkillType::FireMagic:
        case SkillType::IceMagic:
        case SkillType::LightningMagic:
        case SkillType::PoisonMagic:
        case SkillType::Alchemy:
        case SkillType::Crafting:
            return AttributeType::Intelligence;
        case SkillType::Healing:
        case SkillType::Speech:
            return AttributeType::Wisdom;
        default:
            return AttributeType::Dexterity;
    }
}

}

namespace AvatarQuest {

float getSkillAttributeMultiplier(const CharacterSheet& cs, SkillType skill) {
    AttributeType governing = governingAttributeForSkill(skill);
    return attributeMultiplierEffective(cs, governing);
}

// Simple pool formulas; adjust as desired for gameplay
int computeMaxHP(const CharacterSheet& cs) {
    const int lvl = getLevel(cs);
    const int endu = getEffectiveAttribute(cs, AttributeType::Endurance);
    // Base 50 + 10 per level + 5 per Endurance
    return 50 + lvl * 10 + endu * 5;
}

int computeMaxMP(const CharacterSheet& cs) {
    const int lvl = getLevel(cs);
    const int intel = getEffectiveAttribute(cs, AttributeType::Intelligence);
    // Base 30 + 8 per level + 5 per Intelligence
    return 30 + lvl * 8 + intel * 5;
}

int computeExperienceToNextLevel(const CharacterSheet& cs) {
    const int lvl = getLevel(cs);
    // Linear for now: 100 * level
    return 100 * lvl;
}

void recalcDerivedPools(CharacterSheet& cs, bool refillOnLevelUp) {
    // Preserve current percentage when not refilling
    int curHP = getHP(cs);
    int curMP = getMP(cs);
    int oldHPMax = getHPMax(cs);
    int oldMPMax = getMPMax(cs);

    int newHPMax = computeMaxHP(cs);
    int newMPMax = computeMaxMP(cs);
    setHPMax(cs, newHPMax);
    setMPMax(cs, newMPMax);

    if (refillOnLevelUp) {
        setHP(cs, newHPMax);
        setMP(cs, newMPMax);
    } else {
        // Maintain ratio
        int newHP = (oldHPMax > 0) ? (curHP * newHPMax) / oldHPMax : newHPMax;
        int newMP = (oldMPMax > 0) ? (curMP * newMPMax) / oldMPMax : newMPMax;
        if (newHP > newHPMax) newHP = newHPMax;
        if (newMP > newMPMax) newMP = newMPMax;
        setHP(cs, newHP);
        setMP(cs, newMP);
    }
}

void levelUp(CharacterSheet& cs) {
    setLevel(cs, getLevel(cs) + 1);
    setExperienceToNextLevel(cs, computeExperienceToNextLevel(cs));
    recalcDerivedPools(cs, /*refillOnLevelUp=*/true);
}

void addExperience(CharacterSheet& cs, int amount) {
    if (amount <= 0) return;
    int xp = getExperience(cs) + amount;
    int req = getExperienceToNextLevel(cs);
    while (xp >= req) {
        xp -= req;
        levelUp(cs);
        req = getExperienceToNextLevel(cs);
    }
    setExperience(cs, xp);
}

}

namespace AvatarQuest {

// Wallet helpers
CurrencyBundle& getWallet(CharacterSheet& cs) { return cs.wallet; }
const CurrencyBundle& getWallet(const CharacterSheet& cs) { return cs.wallet; }

void addFunds(CharacterSheet& cs, CurrencyType t, int64_t amount) {
    addCurrency(cs.wallet, t, amount);
}

void addFunds(CharacterSheet& cs, const CurrencyBundle& amount) {
    addBundle(cs.wallet, amount);
}

bool spendFunds(CharacterSheet& cs, const CurrencyBundle& cost) {
    return spendBundle(cs.wallet, cost);
}

bool transferFunds(CharacterSheet& from, CharacterSheet& to, const CurrencyBundle& amount) {
    // Validate request: negative amounts make no sense here
    for (const auto& kv : amount.amounts) {
        if (kv.second < 0) return false;
    }
    // Check affordability including non-convertibles and value
    if (!canAfford(from.wallet, amount)) return false;
    // Spend from source (will handle both non-convertibles and value portion)
    CurrencyBundle snapshot = from.wallet; // keep a copy for atomicity on unexpected errors
    if (!spendBundle(from.wallet, amount)) {
        from.wallet = snapshot; // restore if something changed concurrently
        return false;
    }
    // Add to destination
    addBundle(to.wallet, amount);
    return true;
}

}