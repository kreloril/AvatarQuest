#include "AvatarQuestPortraits.h"

namespace AvatarQuest {
namespace Portraits {

static PortraitManager g_pmMale;
static PortraitManager g_pmFemale;
static bool g_loaded = false;

bool EnsureLoaded() {
    if (g_loaded) return true;
    bool okM = g_pmMale.load("assets/Tiles/MalePortraits.png", 128, 128, 4, 4);
    bool okF = g_pmFemale.load("assets/Tiles/FemalePortraits.png", 128, 128, 4, 4);
    g_loaded = okM && okF;
    return g_loaded;
}

Ref<TileMap::Tile> GetTile(bool isMale) {
    EnsureLoaded();
    return isMale ? g_pmMale.tile() : g_pmFemale.tile();
}

void Render(const CharacterSheet& cs, Vector2 position, Vector2 scale, float rotation, Renderer::Color tint) {
    if (!EnsureLoaded()) return;
    const int idx = std::max(0, std::min(cs.portraitIndex, PortraitCount() - 1));
    if (cs.isMale) g_pmMale.draw(idx, position, scale, rotation, tint);
    else           g_pmFemale.draw(idx, position, scale, rotation, tint);
}

int PortraitCount() {
    EnsureLoaded();
    // Both sheets are same size; either one's count is fine
    return g_pmMale.isLoaded() ? g_pmMale.tileCount() : 16;
}

} // namespace Portraits
} // namespace AvatarQuest
