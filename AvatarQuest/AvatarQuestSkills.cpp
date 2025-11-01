#include "Common.h"
#include "AvatarQuestSkills.h"
#include "AvatarQuestRP.h"
#include <algorithm>
#include <cctype>

namespace AvatarQuest {

// List of user-exposed skill types (excludes None and COUNT)
static Vector<SkillType> g_allSkillTypes = {
	SkillType::Attack,
	SkillType::Dodge,
	SkillType::Parry,
	SkillType::Riposte,
	SkillType::Block,
	SkillType::Backstab,
	SkillType::DoubleAttack,
	SkillType::Swords,
	SkillType::Axes,
	SkillType::Spears,
	SkillType::Daggers,
	SkillType::Bows,
	SkillType::Archery,
	SkillType::Staves,
	SkillType::Unarmed,
	SkillType::Shields,
	SkillType::LightArmor,
	SkillType::HeavyArmor,
	SkillType::FireMagic,
	SkillType::IceMagic,
	SkillType::LightningMagic,
	SkillType::PoisonMagic,
	SkillType::Healing,
	SkillType::Smithing,
	SkillType::Alchemy,
	SkillType::Stealth,
	SkillType::Lockpicking,
	SkillType::TrapSetting,
	SkillType::TrapMaking,
	SkillType::Cooking,
	SkillType::Speech,
	SkillType::Crafting,
};

static const char* kSkillTypeNames[] = {
	"None",           // 0
	"Attack",
	"Dodge",
	"Parry",
	"Riposte",
	"Block",
	"Backstab",
	"DoubleAttack",
	"Swords",
	"Axes",
	"Spears",
	"Daggers",
	"Bows",
	"Archery",
	"Staves",
	"Unarmed",
	"Shields",
	"LightArmor",
	"HeavyArmor",
	"FireMagic",
	"IceMagic",
	"LightningMagic",
	"PoisonMagic",
	"Healing",
	"Smithing",
	"Alchemy",
	"Stealth",
	"Lockpicking",
	"TrapSetting",
	"TrapMaking",
	"Cooking",
	"Speech",
	"Crafting",
};

static_assert(static_cast<int>(SkillType::COUNT) == (sizeof(kSkillTypeNames) / sizeof(kSkillTypeNames[0])),
			  "SkillType names table must match enum count");

const Vector<SkillType>& allSkillTypes() {
	return g_allSkillTypes;
}

int skillTypeCount() {
	return static_cast<int>(g_allSkillTypes.size());
}

const char* skillTypeToString(SkillType s) {
	const int id = toInt(s);
	if (id < 0 || id >= static_cast<int>(SkillType::COUNT)) return "Unknown";
	return kSkillTypeNames[id];
}

static String toLowerCopy(const char* s) {
	String out = s ? s : "";
	std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
	return out;
}

bool skillTypeFromString(const char* name, SkillType& out) {
	if (!name) return false;
	String needle = toLowerCopy(name);
	for (int i = 0; i < static_cast<int>(SkillType::COUNT); ++i) {
		String cand = toLowerCopy(kSkillTypeNames[i]);
		if (cand == needle) {
			out = static_cast<SkillType>(i);
			return true;
		}
	}
	return false;
}

void createSkill(SkillType type, int levelRequirement, float rating, Ref<AvatarQuestSkill>& outSkill) {
	if (!outSkill) outSkill = CreateRef<AvatarQuestSkill>();
	outSkill->type = type;
	outSkill->levelRequirement = levelRequirement;
	outSkill->rating = rating;
	outSkill->name = skillTypeToString(type);
	// Assign a default governing attribute based on skill type
	switch (type) {
		case SkillType::Attack:
		case SkillType::DoubleAttack:
		case SkillType::Axes:
		case SkillType::Spears:
		case SkillType::Staves:
		case SkillType::HeavyArmor:
		case SkillType::Shields:
			outSkill->governingAttribute = AttributeType::Strength; break;
		case SkillType::Swords:
		case SkillType::Daggers:
		case SkillType::Bows:
		case SkillType::Archery:
		case SkillType::Unarmed:
		case SkillType::Dodge:
		case SkillType::Parry:
		case SkillType::Riposte:
		case SkillType::LightArmor:
		case SkillType::Stealth:
		case SkillType::Lockpicking:
		case SkillType::TrapSetting:
		case SkillType::Backstab:
			outSkill->governingAttribute = AttributeType::Dexterity; break;
		case SkillType::FireMagic:
		case SkillType::IceMagic:
		case SkillType::LightningMagic:
		case SkillType::PoisonMagic:
		case SkillType::Alchemy:
		case SkillType::Crafting:
		case SkillType::TrapMaking:
		case SkillType::Cooking:
			outSkill->governingAttribute = AttributeType::Intelligence; break;
		case SkillType::Healing:
		case SkillType::Speech:
			outSkill->governingAttribute = AttributeType::Wisdom; break;
		default:
			outSkill->governingAttribute = AttributeType::Dexterity; break;
	}
	// Simple description similar to other modules
	char buf[256];
	std::snprintf(buf, sizeof(buf), "Type=%s, Level=%d, Rating=%.2f",
				  outSkill->name.c_str(), outSkill->levelRequirement, outSkill->rating);
	outSkill->description = buf;

	// Default execute if none provided: return rating as value
	if (!outSkill->execute) {
		outSkill->execute = [](const AvatarQuestSkill& self) -> SkillUseResult {
			SkillUseResult r;
			r.success = true;
			r.value = self.rating; // normalized impact
			r.message = String("Executed ") + self.name;
			return r;
		};
	}
}

SkillUseResult useSkill(const AvatarQuestSkill& skill) {
	if (skill.execute) return skill.execute(skill);
	// Fallback behavior if somehow empty
	SkillUseResult r;
	r.success = true;
	r.value = skill.rating;
	r.message = String("Executed ") + skill.name;
	return r;
}

// Stateless helpers backed by CharacterSheet's UMap of ratings
float skillRating(const CharacterSheet& cs, SkillType type, float defaultValue) {
	int key = static_cast<int>(type);
	auto it = cs.skillRatings.find(key);
	if (it != cs.skillRatings.end()) return it->second;
	return defaultValue;
}

bool hasProficiency(const CharacterSheet& cs, SkillType type, float minRating) {
	return skillRating(cs, type, 0.0f) >= minRating;
}

// Registry for evaluation and checks
static UMap<int, SkillEvalFn>& evalRegistry() {
	static UMap<int, SkillEvalFn> reg; return reg;
}
static UMap<int, SkillCheckFn>& checkRegistry() {
	static UMap<int, SkillCheckFn> reg; return reg;
}

static inline float clamp01(float v) {
	if (v < 0.0f) return 0.0f;
	if (v > 1.0f) return 1.0f;
	return v;
}

// No default handler installation here; call sites or tests should register handlers as needed.

float evaluateSkillEffect(SkillType type, float inputValue, const CharacterSheet& cs) {
	return evaluateSkillEffect(type, inputValue, cs, /*difficulty*/0.0f);
}


float evaluateSkillEffect(SkillType type, float inputValue, const CharacterSheet& cs, float difficulty) {
	const int key = static_cast<int>(type);
	auto it = evalRegistry().find(key);
	if (it != evalRegistry().end()) {
		return it->second(type, inputValue, cs, difficulty);
	}
	// Default: passthrough
	(void)cs; (void)difficulty;
	return inputValue;
}

bool skillCheck(SkillType type, const CharacterSheet& cs, float difficulty, float roll01) {
	const int key = static_cast<int>(type);
	auto it = checkRegistry().find(key);
	if (it != checkRegistry().end()) {
		return it->second(type, cs, difficulty, roll01);
	}
	// Default: generic probability rule
	const float rating = skillRating(cs, type, 0.0f);
	const float prob = clamp01(0.5f + (rating - clamp01(difficulty)));
	return roll01 < prob;
}

void registerSkillEvaluator(SkillType type, SkillEvalFn fn) {
	evalRegistry()[static_cast<int>(type)] = std::move(fn);
}

void registerSkillChecker(SkillType type, SkillCheckFn fn) {
	checkRegistry()[static_cast<int>(type)] = std::move(fn);
}

void clearAllSkillHandlers() {
	evalRegistry().clear();
	checkRegistry().clear();
}

}

// Mapping moved to RP module (governingAttributeForSkill)