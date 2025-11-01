#pragma once

#include "Common.h"
#include "AvatarQuestRP.h"      // CharacterSheet, AttributeBonus, CurrencyBundle
#include "AvatarQuestSkills.h"  // SkillType

namespace AvatarQuest {

// Lightweight, data-first archetype definition used for character generation.
// Skill targets are normalized [0,1] goals applied to a fresh character.
struct AvatarQuestClassArchetype {
    String id;                    // stable id (lowercase, hyphenated)
    String name;                  // display name
    String description;           // short summary used in UI

    // Desired skill ratings for a fresh character (key = static_cast<int>(SkillType))
    UMap<int, float> skillTargets;

    // Flat attribute adjustments applied to the character (relative to current values)
    Vector<AttributeBonus> attributeBonuses;

    // Starting money
    CurrencyBundle startingFunds;
};

// Registry API
// Returns all built-in archetypes (stable order for UI listing)
const Vector<AvatarQuestClassArchetype>& getAllArchetypes();
// Find by stable id; returns nullptr if not found
const AvatarQuestClassArchetype* getArchetypeById(std::string_view id);

// Suggest archetypes from a user's skill preference map (same shape as CharacterSheet::skillRatings)
// Uses a simple dot product score between preferences and archetype skillTargets
Vector<const AvatarQuestClassArchetype*> suggestArchetypesFromSkillPrefs(const UMap<int, float>& skillPrefs, size_t maxResults = 3);

// Apply an archetype to a character sheet:
// - Sets/overwrites skill ratings to archetype targets
// - Applies attribute bonuses
// - Grants starting funds
// - Creates simple starter equipment based on dominant skills
// - Resets level/pools sensibly for a fresh character
void applyArchetype(CharacterSheet& cs, const AvatarQuestClassArchetype& arc);

}
