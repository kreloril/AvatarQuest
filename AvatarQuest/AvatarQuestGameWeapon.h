#pragma once

#include "Common.h"
#include "AvatarQuestSkills.h"
#include "AvatarQuestRP.h"

namespace AvatarQuest
{
	enum struct DamageType {
		
		Damage_Magical,
		Damage_Fire,
		Damage_Ice,
		Damage_Lightning,
		Damage_Poison,
		Damage_Bleeding,
		Damage_Piercing,
		Damage_Slashing,
		Damage_Bludgeoning

	};

	struct DamageInstanceType {
		int tileID = -1;
		DamageType damageType = DamageType::Damage_Piercing;
		Vector2 damageRange;
	};

	// represents the data used for weapons in Avatar Quest
	struct AvatarQuestWeapon {
		int tileID = -1;           // top-level tile for weapon graphic
		SkillType primarySkillType = SkillType::Attack;   // typically Attack
		SkillType secondarySkillType = SkillType::Unarmed; // e.g., Swords, Spears, etc.
		// Governing attribute that provides damage scaling for this weapon
		AttributeType governingAttribute = AttributeType::Strength;
		Vector<DamageInstanceType> damageTypes;
		float attackSpeed = 1.0f;
		int attackReach = 1;
		float criticalChance = 0.05f;
		float durability = 100.0f;
		String weaponName;
		String description; // generated details string
		int levelRequirement = 1;
		// Attribute bonuses applied when equipped
		Vector<AttributeBonus> attributeBonuses;

	};
	// 
	void createDamageTypes(DamageInstanceType* model, int count);
	void createDamageType(DamageType dmgType, Vector2 dmgRange, DamageInstanceType& outDmgType);
	void createWeapon(Vector<DamageInstanceType>& damageType, int level, Ref<AvatarQuestWeapon>& weaponOut);
	void createRandomWeapon(int playerLevel, Ref<AvatarQuestWeapon>& weaponOut);

	// Creates a weapon configured to maximize DPS under the current model.
	// Strategy:
	// - Uses ALL available damage types to maximize average damage per hit
	// - Uses per-type damage ranges at the top end for the given level
	// - attackSpeed/crit/durability are derived from level by createWeapon
	void createMaxDPSWeapon(int level, Ref<AvatarQuestWeapon>& weaponOut);

	// Compute expected-value DPS for a weapon using the same model as descriptions:
	// avg(range) per type, crit multiplier 1.5, then multiply by attackSpeed.
	float computeDPS(const AvatarQuestWeapon& weapon);

	// Compute expected damage per hit (pre-attackSpeed) used by DPS and descriptions.
	// avg(range) per type with crit expected value using crit multiplier 1.5.
	float computeExpectedPerHit(const AvatarQuestWeapon& weapon);

	// Optional: apply attack skill as a simple multiplier to base damage for demonstration.
	float applyAttackSkillToDamage(const AvatarQuestWeapon& weapon, const struct CharacterSheet& attacker, float baseDamage);
	// Difficulty-aware overload; difficulty in [0,1]
	float applyAttackSkillToDamage(const AvatarQuestWeapon& weapon, const struct CharacterSheet& attacker, float baseDamage, float difficulty);
	float applyAttackSkillToDamage(const AvatarQuestWeapon& weapon, const struct CharacterSheet& attacker, float baseDamage, float difficulty);

}