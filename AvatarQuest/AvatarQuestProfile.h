#pragma once

#include "Common.h"
#include "AvatarQuestRP.h"

namespace AvatarQuest {

// Minimal global holder for the current player's character sheet created in Char Creation
// This avoids large refactors; GSWorld can read from here later.
void SetCurrentCharacter(const CharacterSheet& cs);
const CharacterSheet& GetCurrentCharacter();
CharacterSheet& GetCurrentCharacterForEdit();

}
