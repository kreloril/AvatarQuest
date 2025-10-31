#pragma once

#include "Common.h"
#include "AvatarQuestRP.h" // for AttributeType

namespace AvatarQuest {
	// Core gameplay skill types. Keep values stable for save-game compatibility.
	enum struct SkillType : int {
		None = 0,
		// Core combat actions
		Attack,
		Dodge,
		Parry,
		Riposte,
		Block,
		// Weapon skills
		Swords,
		Axes,
		Spears,
		Daggers,
		Bows,
		Staves,
		Unarmed,
		// Defense/armor
		Shields,
		LightArmor,
		HeavyArmor,
		// Magic schools
		FireMagic,
		IceMagic,
		LightningMagic,
		PoisonMagic,
		Healing,
		// Non-combat
		Smithing,
		Alchemy,
		Stealth,
		Speech,
		Crafting,
        
		COUNT // Sentinel; keep last
	};

	// Generic result of using a skill. Value can represent damage, healing, etc.
	struct SkillUseResult {
		bool success = true;
		float value = 0.0f;
		String message;
	};

	struct AvatarQuestSkill {
		SkillType type = SkillType::None;
		int levelRequirement = 1;
		// Skill rating expressed as a fraction [0,1]. Example: 0.75 == 75%.
		float rating = 0.0f;
		// Governing attribute used for checks/gating
		AttributeType governingAttribute = AttributeType::Dexterity;
		String name;
		String description; // generated details string

		using ExecuteFn = std::function<SkillUseResult(const AvatarQuestSkill&)>;
		ExecuteFn execute; // optional; if empty, a default behavior is used
	};

	// Integer id helpers
	constexpr int toInt(SkillType s) { return static_cast<int>(s); }

	// String conversions (stable identifiers)
	const char* skillTypeToString(SkillType s);
	bool skillTypeFromString(const char* name, SkillType& out);

	// Enumeration utilities (excludes None and COUNT)
	const Vector<SkillType>& allSkillTypes();
	int skillTypeCount();


	// Factory and executor
	void createSkill(SkillType type, int levelRequirement, float rating, Ref<AvatarQuestSkill>& outSkill);
	SkillUseResult useSkill(const AvatarQuestSkill& skill);

	// Stateless helpers to favor UMap<SkillType,float> on CharacterSheet
	// Retrieve normalized rating [0,1] from a CharacterSheet for a given skill
	float skillRating(const struct CharacterSheet& cs, SkillType type, float defaultValue = 0.0f);
	// Does the character have non-trivial proficiency in the skill?
	bool hasProficiency(const struct CharacterSheet& cs, SkillType type, float minRating = 0.01f);
	// Generic evaluation hook (deterministic) via per-skill evaluator callback registry.
	// Overload without difficulty delegates to difficulty=0.0f
	float evaluateSkillEffect(SkillType type, float inputValue, const struct CharacterSheet& cs);
	// Difficulty-aware variant: difficulty in [0,1] shifts effective rating; factor = 1 + 0.25*(rating - difficulty)
	float evaluateSkillEffect(SkillType type, float inputValue, const struct CharacterSheet& cs, float difficulty);
	// Deterministic skill check with explicit roll [0,1]; probability uses per-skill checker if present, else default = clamp01(0.5 + (rating - difficulty))
	bool skillCheck(SkillType type, const struct CharacterSheet& cs, float difficulty, float roll01);

	// Registry API: install custom evaluation/check handlers per skill type
	using SkillEvalFn = std::function<float(SkillType, float, const struct CharacterSheet&, float)>; // (type, input, cs, difficulty)
	using SkillCheckFn = std::function<bool(SkillType, const struct CharacterSheet&, float, float)>; // (type, cs, difficulty, roll01)
	void registerSkillEvaluator(SkillType type, SkillEvalFn fn);
	void registerSkillChecker(SkillType type, SkillCheckFn fn);
	void clearAllSkillHandlers(); // clears both evaluator and checker registries
}

