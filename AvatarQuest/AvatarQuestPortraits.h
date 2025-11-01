#pragma once

#include "Common.h"
#include "Renderer.h"
#include "TileBank.h"
#include "AvatarQuestPortraitManager.h"
#include "AvatarQuestRP.h"

namespace AvatarQuest {
namespace Portraits {

// Ensure the portrait atlases are loaded into the TileMap bank
bool EnsureLoaded();

// Get the shared TileMap::Tile for the selected gender
Ref<TileMap::Tile> GetTile(bool isMale);

// Convenience: render the character sheet's selected portrait
void Render(const CharacterSheet& cs,
            Vector2 position,
            Vector2 scale = {1.0f, 1.0f},
            float rotation = 0.0f,
            Renderer::Color tint = {255,255,255,255});

// How many portraits are available per gender
int PortraitCount();

} // namespace Portraits
} // namespace AvatarQuest
