#pragma once

#include "AvatarQuestGameWeapon.h" // for DamageType and DamageInstanceType
#include "AvatarQuestRP.h"

// forward include for skill references
#include "AvatarQuestSkills.h"

namespace AvatarQuest {

	enum struct ArmorType {
		Cloth,
		Leather,
		Chain,
		Plate,
		Shield
	};

	enum struct ArmorSlot {
		Head,
		Chest,
		Feet,
		Hands,
		Shoulders
	};

	// Represents the data used for armor in Avatar Quest
	struct AvatarQuestArmor {
		// Required armor proficiency skill type (e.g., LightArmor/HeavyArmor/Shields)
		// If the wearer lacks this skill, resistances are not effective.
		SkillType armorSkillType = SkillType::None;
		Vector<DamageInstanceType> resistances; // reuse DamageInstanceType to describe reduction ranges
		ArmorType armorType = ArmorType::Leather; // new: armor category
		ArmorSlot slot = ArmorSlot::Chest;       // new: equipment slot
		int armorClass = 0;          // generic defense score
		float dodgeChance = 0.0f;    // chance to avoid a hit
		float durability = 100.0f;   // durability points
		String armorName;            // display name
		String description;          // generated details string
		int levelRequirement = 1;    // required level
		// Attribute bonuses applied when equipped
		Vector<AttributeBonus> attributeBonuses;
	};

	// Resistance model registration (similar to weapon damage models)
	void createResistanceTypes(DamageInstanceType* model, int count);
	void createResistance(DamageType resistType, Vector2 resistRange, DamageInstanceType& outResist);

	// Armor creation utilities
	void createArmor(Vector<DamageInstanceType>& resistances, int level, Ref<AvatarQuestArmor>& armorOut);
	// Overload that allows specifying ArmorType
	void createArmor(Vector<DamageInstanceType>& resistances, int level, ArmorType type, Ref<AvatarQuestArmor>& armorOut);
	// Overload that allows specifying ArmorType and Slot
	void createArmor(Vector<DamageInstanceType>& resistances, int level, ArmorType type, ArmorSlot slot, Ref<AvatarQuestArmor>& armorOut);
	void createRandomArmor(int playerLevel, Ref<AvatarQuestArmor>& armorOut);

	// Compute helpers for armor
	// Expected reduction per hit is the sum of averages across resistance ranges.
	float computeExpectedReduction(const AvatarQuestArmor& armor);
	// Expected final damage after reduction and dodge (expected value):
	// E[final] = (1 - dodgeChance) * max(0, rawDamage - expectedReduction)
	float computeExpectedFinalDamage(float rawDamage, const AvatarQuestArmor& armor);

	// Optional: expected final damage incorporating armor proficiency.
	// If the wearer lacks proficiency in armorSkillType, resistances are not effective (gating behavior).
	float computeExpectedFinalDamageWithSkills(float rawDamage, const AvatarQuestArmor& armor, const struct CharacterSheet& wearer);

}