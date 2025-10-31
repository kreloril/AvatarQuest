#pragma once

#include "Common.h"
#include "AvatarQuestCurrency.h"

namespace AvatarQuest {

// Core character attributes for checks and scaling
enum struct AttributeType : int {
	Strength = 0,
	Dexterity,
	Intelligence,
	Endurance,
	Wisdom,
	// Extended attributes/pools and progression moved under AttributeType
	Level,
	HP,
	HPMax,
	MP,
	MPMax,
	Experience,
	ExperienceToNextLevel,
	COUNT
};

// Attribute bonus attached to equipment (weapons/armor)
// amount can be positive or negative and applies additively to the base attribute in CharacterSheet
struct AttributeBonus {
	AttributeType type;
	int amount;
};

// Attributes stored as a flexible map keyed by AttributeType id
struct CharacterAttributes {
	// key = static_cast<int>(AttributeType), value = attribute score
	UMap<int, int> values;
};

// Forward declarations to avoid circular dependencies
struct AvatarQuestSkill;
enum struct SkillType : int;
// Mapping from skill to its governing attribute (centralized here with attributes)
AttributeType governingAttributeForSkill(SkillType type);
struct AvatarQuestArmor;
struct AvatarQuestWeapon;
enum struct ArmorSlot;

// Utility: get a human-readable attribute name
const char* attributeTypeToString(AttributeType a);

// Basic attribute gate: does character meet a requirement on a governing attribute?
bool passesAttributeCheck(const CharacterAttributes& attrs, AttributeType governing, int requiredLevel);

// Accessors for attribute map
int getAttribute(const CharacterAttributes& attrs, AttributeType a, int defaultValue = 10);
void setAttribute(CharacterAttributes& attrs, AttributeType a, int value);

// Basic character sheet with attributes and skill ratings
struct CharacterSheet {
	String name;
	CharacterAttributes attributes;
	// Skill ratings normalized to [0,1], key is static_cast<int>(SkillType)
	UMap<int, float> skillRatings;
	// Equipped items
	struct Equipment {
		Ref<AvatarQuestWeapon> primaryWeapon;
		Ref<AvatarQuestWeapon> secondaryWeapon;
		Ref<AvatarQuestArmor> head;
		Ref<AvatarQuestArmor> chest;
		Ref<AvatarQuestArmor> feet;
		Ref<AvatarQuestArmor> hands;
		Ref<AvatarQuestArmor> shoulders;
	} equipment;
	// Wallet/currency carried by this character
	CurrencyBundle wallet;
};

// Skill rating helpers (convenience overloads)
float getSkillRating(const CharacterSheet& cs, SkillType s, float defaultValue = 0.0f);
void setSkillRating(CharacterSheet& cs, SkillType s, float rating);

// Equip helpers
void equipPrimaryWeapon(CharacterSheet& cs, const Ref<AvatarQuestWeapon>& w);
void equipSecondaryWeapon(CharacterSheet& cs, const Ref<AvatarQuestWeapon>& w);
const Ref<AvatarQuestWeapon>& getPrimaryWeapon(const CharacterSheet& cs);
const Ref<AvatarQuestWeapon>& getSecondaryWeapon(const CharacterSheet& cs);

void equipArmor(CharacterSheet& cs, const Ref<AvatarQuestArmor>& a);
Ref<AvatarQuestArmor> getArmor(const CharacterSheet& cs, ArmorSlot slot);

// Attribute bonus helpers
// Returns a signed modifier relative to 10: e.g., 12 -> +2, 8 -> -2
int attributeModifier(int attributeScore);
// Returns a multiplicative damage/efficiency factor: 1 + 0.05 * modifier
float attributeMultiplier(const CharacterSheet& cs, AttributeType a);
// Like attributeMultiplier, but uses effective attribute (base + equipment bonuses)
float attributeMultiplierEffective(const CharacterSheet& cs, AttributeType a);

// Effective attributes include equipment bonuses from all equipped items
int getEffectiveAttribute(const CharacterSheet& cs, AttributeType a);
// Skill bonus multiplier from governing attribute (includes equipment)
float getSkillAttributeMultiplier(const CharacterSheet& cs, SkillType skill);

// Convenience accessors for moved attributes/pools/progression
int getLevel(const CharacterSheet& cs);            void setLevel(CharacterSheet& cs, int value);
int getHP(const CharacterSheet& cs);               void setHP(CharacterSheet& cs, int value);
int getHPMax(const CharacterSheet& cs);            void setHPMax(CharacterSheet& cs, int value);
int getMP(const CharacterSheet& cs);               void setMP(CharacterSheet& cs, int value);
int getMPMax(const CharacterSheet& cs);            void setMPMax(CharacterSheet& cs, int value);
int getExperience(const CharacterSheet& cs);       void setExperience(CharacterSheet& cs, int value);
int getExperienceToNextLevel(const CharacterSheet& cs); void setExperienceToNextLevel(CharacterSheet& cs, int value);

// Derived calculators for pools and progression
int computeMaxHP(const CharacterSheet& cs);
int computeMaxMP(const CharacterSheet& cs);
int computeExperienceToNextLevel(const CharacterSheet& cs);

// Recalculate and write HPMax/MPMax based on current attributes; optionally refill HP/MP
void recalcDerivedPools(CharacterSheet& cs, bool refillOnLevelUp = true);

// Leveling API
void levelUp(CharacterSheet& cs);
void addExperience(CharacterSheet& cs, int amount);

// Wallet helpers on CharacterSheet
CurrencyBundle& getWallet(CharacterSheet& cs);
const CurrencyBundle& getWallet(const CharacterSheet& cs);
void addFunds(CharacterSheet& cs, CurrencyType t, int64_t amount);
void addFunds(CharacterSheet& cs, const CurrencyBundle& amount);
bool spendFunds(CharacterSheet& cs, const CurrencyBundle& cost);

// Transfer helpers
// Transfer a mixed bundle from one character to another. Returns false if the
// source cannot afford the non-convertible counts or value portion.
bool transferFunds(CharacterSheet& from, CharacterSheet& to, const CurrencyBundle& amount);

}


