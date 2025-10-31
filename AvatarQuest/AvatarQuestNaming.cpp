#include "Common.h"
#include "AvatarQuestNaming.h"

namespace AvatarQuest {

namespace {

static const Vector<String>& TierAdjectives() {
    static Vector<String> tiers = {
        "Shoddy", "Worn", "Dull", "Rusty", "Cracked",
        "Common", "Sturdy", "Fine", "Keen", "Exquisite",
        "Epic", "Legendary", "Mythic", "Ascendant", "Transcendent",
        "Celestrial", "Empyrean"
    };
    return tiers;
}

// Weapon name tables keyed by damage type
static const UMap<DamageType, Vector<String>>& WeaponNounsByType() {
    static UMap<DamageType, Vector<String>> tbl = {
        { DamageType::Damage_Fire,        { "Flame Blade", "Ember Saber", "Inferno Edge", "Cinder Brand" } },
        { DamageType::Damage_Ice,         { "Frost Edge", "Glacier Blade", "Icebrand", "Coldsteel" } },
        { DamageType::Damage_Lightning,   { "Storm Spear", "Thunder Pike", "Bolt Javelin", "Tempest Lance" } },
        { DamageType::Damage_Poison,      { "Venom Fang", "Toxin Dirk", "Serpent Dagger", "Sting Knife" } },
        { DamageType::Damage_Bleeding,    { "Razor Shard", "Sawblade", "Barbed Cutter", "Bloodletter" } },
        { DamageType::Damage_Piercing,    { "Spike Javelin", "Pierce Spear", "Needle Rapier", "Lancer" } },
        { DamageType::Damage_Slashing,    { "Cutlass", "Scimitar", "Longsword", "Greatsword" } },
        { DamageType::Damage_Bludgeoning, { "Maul", "Warhammer", "Mace", "Flail" } },
        { DamageType::Damage_Magical,     { "Arcane Wand", "Mystic Staff", "Spellblade", "Focus" } }
    };
    return tbl;
}

// Armor base nouns keyed by armor type
static const UMap<ArmorType, Vector<String>>& ArmorNounsByType() {
    static UMap<ArmorType, Vector<String>> tbl = {
        { ArmorType::Cloth,   { "Robe", "Vestments", "Shroud" } },
        { ArmorType::Leather, { "Jerkin", "Leathers", "Mantle" } },
        { ArmorType::Chain,   { "Mail", "Hauberk", "Coat" } },
        { ArmorType::Plate,   { "Plate", "Cuirass", "Bulwark" } },
        { ArmorType::Shield,  { "Shield", "Kite Shield", "Tower Shield" } }
    };
    return tbl;
}

static String PickByLevel(const Vector<String>& options, int level) {
    if (options.empty()) return "Item";
    int idx = level >= 0 ? level % (int)options.size() : 0;
    return options[idx];
}

} // namespace

// Public read-only access to the tier adjective list for tests and tools.
const Vector<String>& GetTierAdjectives() {
    return TierAdjectives();
}

String GenerateWeaponName(const Vector<DamageInstanceType>& types, int level) {
    DamageType primary = types.empty() ? DamageType::Damage_Magical : types.front().damageType;
    const auto& nounsTbl = WeaponNounsByType();
    auto it = nounsTbl.find(primary);
    const Vector<String>& options = (it != nounsTbl.end()) ? it->second : Vector<String>{ "Weapon" };
    String noun = PickByLevel(options, level);

    int band = level > 0 ? (level - 1) / 5 : 0;
    const auto& tiers = TierAdjectives();
    int tierIdx = band < (int)tiers.size() ? band : (int)tiers.size() - 1;
    int overflow = band - ((int)tiers.size() - 1);
    if (overflow < 0) overflow = 0;

    // Place overflow suffix immediately after the tier adjective, e.g., "Empyrean +1 <Noun>"
    String name;
    if (overflow > 0) {
        name = tiers[tierIdx] + String(" +") + std::to_string(overflow) + String(" ") + noun;
    } else {
        name = tiers[tierIdx] + String(" ") + noun;
    }
    return name;
}

String GenerateArmorName(const Vector<DamageInstanceType>& /*resistances*/, int level, ArmorType type) {
    const auto& nounsTbl = ArmorNounsByType();
    auto it = nounsTbl.find(type);
    const Vector<String>& options = (it != nounsTbl.end()) ? it->second : Vector<String>{ "Armor" };
    String noun = PickByLevel(options, level);

    int band = level > 0 ? (level - 1) / 5 : 0;
    const auto& tiers = TierAdjectives();
    int tierIdx = band < (int)tiers.size() ? band : (int)tiers.size() - 1;
    int overflow = band - ((int)tiers.size() - 1);
    if (overflow < 0) overflow = 0;

    // Place overflow suffix immediately after the tier adjective, e.g., "Empyrean +1 <Noun>"
    String name;
    if (overflow > 0) {
        name = tiers[tierIdx] + String(" +") + std::to_string(overflow) + String(" ") + noun;
    } else {
        name = tiers[tierIdx] + String(" ") + noun;
    }
    return name;
}

String GenerateItemNameForDamageTypes(const Vector<DamageInstanceType>& types, int level, bool isWeapon) {
    if (isWeapon) return GenerateWeaponName(types, level);
    // Default armor type when not specified
    return GenerateArmorName(types, level, ArmorType::Leather);
}

}
