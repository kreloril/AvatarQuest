#pragma once

#include "Common.h"
#include "AvatarQuestGameWeapon.h" // for DamageType/DamageInstanceType
#include "AvatarQuestGameArmor.h"  // for ArmorType

namespace AvatarQuest {

// Generate a readable weapon name based on damage types and level.
String GenerateWeaponName(const Vector<DamageInstanceType>& types, int level);

// Generate a readable armor name based on resistances, level, and armor category.
String GenerateArmorName(const Vector<DamageInstanceType>& resistances, int level, ArmorType type);

// Expose the tier adjective list used in naming logic (read-only).
// Useful for tests to avoid hardcoding tier names and to stay in sync with production.
const Vector<String>& GetTierAdjectives();

// Back-compat shared entrypoint used earlier; routes to weapon/armor generators.
String GenerateItemNameForDamageTypes(const Vector<DamageInstanceType>& types, int level, bool isWeapon);

}
