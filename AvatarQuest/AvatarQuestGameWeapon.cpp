#include "Common.h"
#include "AvatarQuestGameWeapon.h"
#include "AvatarQuestRP.h"
#include "AvatarQuestNaming.h"
#include "AvatarQuestSkills.h"
#include <sstream>
#include <iomanip>

namespace AvatarQuest {

	UMap<DamageType, String> damageTypeNames = {
		{ DamageType::Damage_Magical, "Magical" },
		{ DamageType::Damage_Fire, "Fire" },
		{ DamageType::Damage_Ice, "Ice" },
		{ DamageType::Damage_Lightning, "Lightning" },
		{ DamageType::Damage_Poison, "Poison" },
		{ DamageType::Damage_Bleeding, "Bleeding" },
		{ DamageType::Damage_Piercing, "Piercing" },
		{ DamageType::Damage_Slashing, "Slashing" },
		{ DamageType::Damage_Bludgeoning, "Bludgeoning" }
	};

	UMap<DamageType, DamageInstanceType> g_DamageTypeModels;

	// Attribute stringification is centralized in RP: attributeTypeToString

	static void buildWeaponDescription(const Ref<AvatarQuestWeapon>& weaponOut)
	{
		std::ostringstream oss;
		oss << weaponOut->weaponName << "\n";
		oss << "Level " << weaponOut->levelRequirement
			<< ", PSkill=" << skillTypeToString(weaponOut->primarySkillType)
			<< ", SSkill=" << skillTypeToString(weaponOut->secondarySkillType)
			<< ", Attr=" << attributeTypeToString(weaponOut->governingAttribute) << "\n";
		oss << "ASpd=" << weaponOut->attackSpeed
			<< ", Reach=" << weaponOut->attackReach
			<< ", Crit=" << weaponOut->criticalChance
			<< ", Durability=" << weaponOut->durability << "\n";
		float expectedPerHit = computeExpectedPerHit(*weaponOut);
		float dps = computeDPS(*weaponOut);
		{
			std::ostringstream dpsFmt;
			dpsFmt << std::fixed << std::setprecision(2) << dps;
			oss << "AvgDmg=" << expectedPerHit << ", DPS=" << dpsFmt.str() << "\n";
		}
		oss << "Damage:";
		for (const auto& di : weaponOut->damageTypes) {
			String typeName;
			auto it = damageTypeNames.find(di.damageType);
			if (it != damageTypeNames.end()) typeName = it->second; else typeName = "Unknown";
			oss << " [" << typeName << " " << di.damageRange.x << "-" << di.damageRange.y << "]";
		}
		// Explicitly list the skills the item confers/uses for clarity
		oss << "\nSkills: [" << skillTypeToString(weaponOut->primarySkillType)
			<< "] [" << skillTypeToString(weaponOut->secondarySkillType) << "]";
		if (!weaponOut->attributeBonuses.empty()) {
			oss << "\nBonuses:";
			for (const auto& b : weaponOut->attributeBonuses) {
				oss << " [" << attributeTypeToString(b.type) << " "
					<< (b.amount >= 0 ? "+" : "") << b.amount << "]";
			}
		}
		weaponOut->description = oss.str();
	}

	void createDamageTypes(DamageInstanceType* model, int count)
	{
		for (int i = 0; i < count; ++i) {
			g_DamageTypeModels[model[i].damageType] = model[i];
		}
	}

	void createDamageType(DamageType dmgType, Vector2 dmgRange, DamageInstanceType& outDmgType)
	{
		outDmgType.damageType = dmgType;
		outDmgType.damageRange = dmgRange;
		if (g_DamageTypeModels.find(dmgType) != g_DamageTypeModels.end()) {
			outDmgType.tileID = g_DamageTypeModels[dmgType].tileID;
		} else {
			outDmgType.tileID = -1;
		}
	}

	float computeDPS(const AvatarQuestWeapon& weapon)
	{
		float expectedPerHit = computeExpectedPerHit(weapon);
		return expectedPerHit * weapon.attackSpeed;
	}

	float computeExpectedPerHit(const AvatarQuestWeapon& weapon)
	{
		float avgDamage = 0.0f;
		for (const auto& di : weapon.damageTypes) {
			avgDamage += 0.5f * (di.damageRange.x + di.damageRange.y);
		}
		constexpr float kCritMultiplier = 1.5f;
		return avgDamage * (1.0f + weapon.criticalChance * (kCritMultiplier - 1.0f));
	}

	static SkillType mapDamageTypeToWeaponSkill(DamageType dt) {
		switch (dt) {
			case DamageType::Damage_Slashing: return SkillType::Swords;
			case DamageType::Damage_Piercing: return SkillType::Spears;
			case DamageType::Damage_Bludgeoning: return SkillType::Staves;
			default: return SkillType::Staves; // elemental/magical -> staves as proxy
		}
	}

	// Mapping is centralized in RP now: governingAttributeForSkill(SkillType)

	static DamageType mapSkillToPrimaryDamageType(SkillType s) {
		switch (s) {
			case SkillType::Swords:   return DamageType::Damage_Slashing;
			case SkillType::Spears:   return DamageType::Damage_Piercing;
			case SkillType::Staves:   return DamageType::Damage_Bludgeoning;
			case SkillType::Axes:     return DamageType::Damage_Bludgeoning; // mauls/axes as blunt
			case SkillType::Daggers:  return DamageType::Damage_Piercing;
			case SkillType::Bows:     return DamageType::Damage_Piercing;
			case SkillType::Unarmed:  return DamageType::Damage_Bludgeoning;
			default:                  return DamageType::Damage_Bludgeoning;
		}
	}

	float applyAttackSkillToDamage(const AvatarQuestWeapon& weapon, const CharacterSheet& attacker, float baseDamage)
	{
		// Use stateless evaluation from SkillBank helpers (Attack scales by rating)
		float v = evaluateSkillEffect(SkillType::Attack, baseDamage, attacker);
		// Apply governing attribute multiplier for the weapon's skill
		float attrMul = getSkillAttributeMultiplier(attacker, weapon.secondarySkillType);
		return v * attrMul;
	}

	float applyAttackSkillToDamage(const AvatarQuestWeapon& weapon, const CharacterSheet& attacker, float baseDamage, float difficulty)
	{
		float v = evaluateSkillEffect(SkillType::Attack, baseDamage, attacker, difficulty);
		float attrMul = getSkillAttributeMultiplier(attacker, weapon.secondarySkillType);
		return v * attrMul;
	}
	void createWeapon(Vector<DamageInstanceType>& damageType, int level, Ref<AvatarQuestWeapon>& weaponOut)
	{
		weaponOut = std::make_shared<AvatarQuestWeapon>();
		weaponOut->damageTypes = damageType;
		weaponOut->levelRequirement = level;
		weaponOut->attackSpeed = 1.0f + (level * 0.1f);
		weaponOut->attackReach = 1 + (level / 5);
		weaponOut->criticalChance = 0.05f + (level * 0.01f);
		weaponOut->durability = 100.0f + (level * 10.0f);
		// Assign a top-level tile based on the first damage type model if available
		weaponOut->tileID = (!weaponOut->damageTypes.empty()) ? weaponOut->damageTypes.front().tileID : -1;
		// Enforce loot rule: below Exquisite tier, strip any additive damage types provided by callers
		{
			int band = level > 0 ? (level - 1) / 5 : 0;
			if (band < 9 && weaponOut->damageTypes.size() > 1) {
				// Keep only the primary damage type (front)
				DamageInstanceType primary = weaponOut->damageTypes.front();
				weaponOut->damageTypes.clear();
				weaponOut->damageTypes.push_back(primary);
			}
		}
		weaponOut->weaponName = GenerateWeaponName(weaponOut->damageTypes, level);

	// Assign default skill types (no instances)
	weaponOut->primarySkillType = SkillType::Attack;
	weaponOut->secondarySkillType = weaponOut->damageTypes.empty() ? SkillType::Unarmed : mapDamageTypeToWeaponSkill(weaponOut->damageTypes.front().damageType);
	// Governing attribute derived from weapon skill (centralized in RP)
	weaponOut->governingAttribute = governingAttributeForSkill(weaponOut->secondarySkillType);

	// Append attribute epithet to name (non-breaking change to naming flavor)
	auto attrName = attributeTypeToString(weaponOut->governingAttribute);
	if (attrName && *attrName) {
		weaponOut->weaponName += String(" of ") + attrName;
	}

		// Build description
		buildWeaponDescription(weaponOut);
	}
	void createRandomWeapon(int playerLevel, Ref<AvatarQuestWeapon>& weaponOut)
	{
		// Choose a single weapon skill and its canonical damage type
		SkillType skills[] = { SkillType::Swords, SkillType::Spears, SkillType::Staves, SkillType::Axes, SkillType::Daggers, SkillType::Bows, SkillType::Unarmed };
		int skillCount = static_cast<int>(sizeof(skills)/sizeof(skills[0]));
		SkillType chosen = skills[rand() % skillCount];
		DamageType dmgType = mapSkillToPrimaryDamageType(chosen);

		Vector<DamageInstanceType> damageTypes;
		Vector2 dmgRange = { 5.0f + playerLevel * 2.0f, 10.0f + playerLevel * 3.0f };
		DamageInstanceType dmgInstance;
		createDamageType(dmgType, dmgRange, dmgInstance);
		damageTypes.push_back(dmgInstance);

		createWeapon(damageTypes, playerLevel, weaponOut);
		// Force secondary skill to chosen and rebuild description/name suffix
		weaponOut->secondarySkillType = chosen;
		weaponOut->governingAttribute = governingAttributeForSkill(chosen);
		{
			auto attrName = attributeTypeToString(weaponOut->governingAttribute);
			if (attrName && *attrName) {
				if (weaponOut->weaponName.find(" of ") == String::npos) {
					weaponOut->weaponName += String(" of ") + attrName;
				}
			}
		}
		// Optionally add at most one damage additive, only at Exquisite tier and above
		// Tier bands are 5 levels wide; Exquisite is band >= 9 -> level >= 46
		{
			int band = playerLevel > 0 ? (playerLevel - 1) / 5 : 0;
			if (band >= 9) {
				// 50% chance to roll an additive; capped to 1
				if ((rand() % 2) == 1) {
					// Choose an additive type different from primary
					DamageType primary = weaponOut->damageTypes.front().damageType;
					DamageType allTypes[] = {
						DamageType::Damage_Fire,
						DamageType::Damage_Ice,
						DamageType::Damage_Lightning,
						DamageType::Damage_Poison,
						DamageType::Damage_Bleeding,
						DamageType::Damage_Piercing,
						DamageType::Damage_Slashing,
						DamageType::Damage_Bludgeoning,
						DamageType::Damage_Magical
					};
					// Try up to N picks to get a different one
					DamageType addType = primary;
					for (int tries = 0; tries < 5; ++tries) {
						DamageType candidate = allTypes[rand() % (int)(sizeof(allTypes)/sizeof(allTypes[0]))];
						if (candidate != primary) { addType = candidate; break; }
					}
					// Scale additive to be a fraction of base
					Vector2 base = weaponOut->damageTypes.front().damageRange;
					Vector2 addRange = { base.x * 0.25f, base.y * 0.5f };
					DamageInstanceType addInst{};
					createDamageType(addType, addRange, addInst);
					weaponOut->damageTypes.push_back(addInst);
				}
			}
		}
		buildWeaponDescription(weaponOut);
		// Randomly assign 0-2 attribute bonuses
		int bonusCount = rand() % 3; // already capped to a max of 2 attributes
		if (bonusCount > 0) {
			// Allowed attributes for random bonuses
			AttributeType allowed[] = {
				AttributeType::Strength, AttributeType::Dexterity, AttributeType::Intelligence,
				AttributeType::Endurance, AttributeType::Wisdom, AttributeType::HPMax, AttributeType::MPMax
			};
			int allowedCount = static_cast<int>(sizeof(allowed)/sizeof(allowed[0]));
			for (int i = 0; i < bonusCount; ++i) {
				AttributeType at = allowed[rand() % allowedCount];
				int amt = 1 + (playerLevel / 5);
				weaponOut->attributeBonuses.push_back({at, amt});
			}
			// Rebuild description to include bonuses
			buildWeaponDescription(weaponOut);
		}
	}

	void createMaxDPSWeapon(int level, Ref<AvatarQuestWeapon>& weaponOut)
	{
		// Build a single-type weapon with boosted ranges to exceed random DPS
		Vector<DamageInstanceType> damageTypes;
		Vector2 base = { 5.0f + level * 2.0f, 10.0f + level * 3.0f };
		Vector2 boosted = { base.x * 1.5f, base.y * 1.5f };
		DamageInstanceType inst{};
		createDamageType(DamageType::Damage_Slashing, boosted, inst);
		damageTypes.push_back(inst);
		createWeapon(damageTypes, level, weaponOut);
	}
}