#include "AvatarQuestProfile.h"

namespace AvatarQuest {

static CharacterSheet g_currentCharacter{};

void SetCurrentCharacter(const CharacterSheet& cs) {
    g_currentCharacter = cs;
}

const CharacterSheet& GetCurrentCharacter() {
    return g_currentCharacter;
}

CharacterSheet& GetCurrentCharacterForEdit() {
    return g_currentCharacter;
}

}
