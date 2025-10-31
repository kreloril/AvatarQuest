
#include "Common.h"
#include <SDL3/SDL_audio.h>
#include "AvatarQuestGameArmor.h"
#include "AvatarQuestNaming.h"
#include "AvatarQuestSkills.h"
#include "AvatarQuestRP.h"
#include <sstream>


namespace AvatarQuest {

	static UMap<DamageType, DamageInstanceType> g_ResistanceModels;
	static UMap<DamageType, String> damageTypeNamesArmor = {
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

	static const char* ToString(ArmorType t) {
		switch (t) {
			case ArmorType::Cloth: return "Cloth";
			case ArmorType::Leather: return "Leather";
			case ArmorType::Chain: return "Chain";
			case ArmorType::Plate: return "Plate";
			case ArmorType::Shield: return "Shield";
			default: return "Armor";
		}
	}

	static const char* ToString(ArmorSlot s) {
		switch (s) {
			case ArmorSlot::Head: return "Head";
			case ArmorSlot::Chest: return "Chest";
			case ArmorSlot::Feet: return "Feet";
			case ArmorSlot::Hands: return "Hands";
			case ArmorSlot::Shoulders: return "Shoulders";
			default: return "Slot";
		}
	}

	static void buildArmorDescription(const Ref<AvatarQuestArmor>& armorOut) {
		std::ostringstream oss;
		oss << armorOut->armorName << "\n";
		oss << "Level " << armorOut->levelRequirement
			<< ", Skill=" << skillTypeToString(armorOut->armorSkillType) << "\n";
		oss << "Type=" << ToString(armorOut->armorType)
			<< ", Slot=" << ToString(armorOut->slot) << "\n";
		oss << "AC=" << armorOut->armorClass
			<< ", Dodge=" << armorOut->dodgeChance
			<< ", Durability=" << armorOut->durability << "\n";
		oss << "Resists:";
		for (const auto& r : armorOut->resistances) {
			String typeName;
			auto it = damageTypeNamesArmor.find(r.damageType);
			typeName = (it != damageTypeNamesArmor.end()) ? it->second : String("Unknown");
			oss << " [" << typeName << " " << r.damageRange.x << "-" << r.damageRange.y << "]";
		}
		if (!armorOut->attributeBonuses.empty()) {
			oss << "\nBonuses:";
			for (const auto& b : armorOut->attributeBonuses) {
				oss << " [" << attributeTypeToString(b.type) << " "
					<< (b.amount >= 0 ? "+" : "") << b.amount << "]";
			}
		}
		armorOut->description = oss.str();
	}

	void createResistanceTypes(DamageInstanceType* model, int count)
	{
		for (int i = 0; i < count; ++i) {
			g_ResistanceModels[model[i].damageType] = model[i];
		}
	}

	void createResistance(DamageType resistType, Vector2 resistRange, DamageInstanceType& outResist)
	{
		outResist.damageType = resistType;
		outResist.damageRange = resistRange;
		if (g_ResistanceModels.find(resistType) != g_ResistanceModels.end()) {
			outResist.tileID = g_ResistanceModels[resistType].tileID;
		} else {
			outResist.tileID = -1;
		}
	}

	float computeExpectedReduction(const AvatarQuestArmor& armor)
	{
		float sum = 0.0f;
		for (const auto& r : armor.resistances) {
			sum += 0.5f * (r.damageRange.x + r.damageRange.y);
		}
		return sum;
	}

	float computeExpectedFinalDamage(float rawDamage, const AvatarQuestArmor& armor)
	{
		float expectedReduction = computeExpectedReduction(armor);
		float reduced = rawDamage - expectedReduction;
		if (reduced < 0.0f) reduced = 0.0f;
		return (1.0f - armor.dodgeChance) * reduced;
	}

    float computeExpectedFinalDamageWithSkills(float rawDamage, const AvatarQuestArmor& armor, const CharacterSheet& wearer)
	{
		// If wearer lacks armor proficiency skill, resistances provide no benefit.
		float expectedReduction = hasProficiency(wearer, armor.armorSkillType) ? computeExpectedReduction(armor) : 0.0f;
		float reduced = rawDamage - expectedReduction;
		if (reduced < 0.0f) reduced = 0.0f;
		// In this initial model, armorSkill gates resistances only.
		float out = (1.0f - armor.dodgeChance) * reduced;
		if (out < 0.0f) out = 0.0f;

	//	SDL_PutAudioStreamPlanarData(nullptr, nullptr, 0); // suppress unused parameter warning

		return out;
	}

	void createArmor(Vector<DamageInstanceType>& resistances, int level, Ref<AvatarQuestArmor>& armorOut)
	{
		armorOut = std::make_shared<AvatarQuestArmor>();
		armorOut->resistances = resistances;
		armorOut->levelRequirement = level;
		// Default armor type by level band
		if (level < 5)      armorOut->armorType = ArmorType::Cloth;
		else if (level < 10)armorOut->armorType = ArmorType::Leather;
		else if (level < 20)armorOut->armorType = ArmorType::Chain;
		else                armorOut->armorType = ArmorType::Plate;

		// Derived fields (simple progression)
		armorOut->armorClass = 5 + (level * 2);
		armorOut->dodgeChance = 0.02f + (level * 0.005f);
		armorOut->durability = 150.0f + (level * 12.0f);
		armorOut->armorName = GenerateArmorName(armorOut->resistances, level, armorOut->armorType);

		// Assign required armor proficiency skill type based on armor type
		switch (armorOut->armorType) {
			case ArmorType::Cloth:
			case ArmorType::Leather:
				armorOut->armorSkillType = SkillType::LightArmor; break;
			case ArmorType::Chain:
			case ArmorType::Plate:
				armorOut->armorSkillType = SkillType::HeavyArmor; break;
			case ArmorType::Shield:
				armorOut->armorSkillType = SkillType::Shields; break;
			default: armorOut->armorSkillType = SkillType::None; break;
		}

		// Build description
		buildArmorDescription(armorOut);
	}

	void createArmor(Vector<DamageInstanceType>& resistances, int level, ArmorType type, Ref<AvatarQuestArmor>& armorOut)
	{
		createArmor(resistances, level, armorOut);
		armorOut->armorType = type;
		armorOut->armorName = GenerateArmorName(armorOut->resistances, level, armorOut->armorType);
		// Update armorSkillType based on explicit type
		switch (armorOut->armorType) {
			case ArmorType::Cloth:
			case ArmorType::Leather:
				armorOut->armorSkillType = SkillType::LightArmor; break;
			case ArmorType::Chain:
			case ArmorType::Plate:
				armorOut->armorSkillType = SkillType::HeavyArmor; break;
			case ArmorType::Shield:
				armorOut->armorSkillType = SkillType::Shields; break;
			default: armorOut->armorSkillType = SkillType::None; break;
		}
		// refresh description after name/type change
		buildArmorDescription(armorOut);
	}

	void createArmor(Vector<DamageInstanceType>& resistances, int level, ArmorType type, ArmorSlot slot, Ref<AvatarQuestArmor>& armorOut)
	{
		createArmor(resistances, level, armorOut);
		armorOut->armorType = type;
		armorOut->slot = slot;
		armorOut->armorName = GenerateArmorName(armorOut->resistances, level, armorOut->armorType);
		// Update armorSkillType based on explicit type
		switch (armorOut->armorType) {
			case ArmorType::Cloth:
			case ArmorType::Leather:
				armorOut->armorSkillType = SkillType::LightArmor; break;
			case ArmorType::Chain:
			case ArmorType::Plate:
				armorOut->armorSkillType = SkillType::HeavyArmor; break;
			case ArmorType::Shield:
				armorOut->armorSkillType = SkillType::Shields; break;
			default: armorOut->armorSkillType = SkillType::None; break;
		}
		buildArmorDescription(armorOut);
	}

	void createRandomArmor(int playerLevel, Ref<AvatarQuestArmor>& armorOut)
	{
		Vector<DamageInstanceType> resistances;
		// Choose a single primary resistance type
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
		const int typeCount = static_cast<int>(sizeof(allTypes)/sizeof(allTypes[0]));
		DamageType primaryType = allTypes[rand() % typeCount];
		Vector2 baseRange = { 1.0f + playerLevel * 0.5f, 3.0f + playerLevel * 1.5f };
		DamageInstanceType baseRes{};
		createResistance(primaryType, baseRange, baseRes);
		resistances.push_back(baseRes);

		// Optional single additive resistance only at Exquisite tier and above (band >= 9 => level >= 46)
		{
			int band = playerLevel > 0 ? (playerLevel - 1) / 5 : 0;
			if (band >= 9) {
				// 50% chance to roll one additive; must be distinct from primary
				if ((rand() % 2) == 1) {
					DamageType addType = primaryType;
					for (int tries = 0; tries < 5; ++tries) {
						DamageType candidate = allTypes[rand() % typeCount];
						if (candidate != primaryType) { addType = candidate; break; }
					}
					Vector2 addRange = { baseRange.x * 0.25f, baseRange.y * 0.5f };
					DamageInstanceType addRes{};
					createResistance(addType, addRange, addRes);
					resistances.push_back(addRes);
				}
			}
		}

		createArmor(resistances, playerLevel, armorOut);
		// Randomly assign 0-2 attribute bonuses
		int bonusCount = rand() % 3;
		if (bonusCount > 0) {
			AttributeType allowed[] = {
				AttributeType::Strength, AttributeType::Dexterity, AttributeType::Intelligence,
				AttributeType::Endurance, AttributeType::Wisdom, AttributeType::HPMax, AttributeType::MPMax
			};
			int allowedCount = static_cast<int>(sizeof(allowed)/sizeof(allowed[0]));
			for (int i = 0; i < bonusCount; ++i) {
				AttributeType at = allowed[rand() % allowedCount];
				int amt = 1 + (playerLevel / 6);
				armorOut->attributeBonuses.push_back({at, amt});
			}
			buildArmorDescription(armorOut);
		}
	}

}
