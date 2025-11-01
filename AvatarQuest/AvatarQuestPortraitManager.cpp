#include "AvatarQuestPortraitManager.h"
#include "TileBank.h"

namespace AvatarQuest {

bool PortraitManager::load(const char* path, int tileW, int tileH) {
    // Default to 4x4 grid for 16-tile sheets
    return load(path, tileW, tileH, 4, 4);
}

bool PortraitManager::load(const char* path, int tileW, int tileH, int columns, int rows) {
    if (!path || tileW <= 0 || tileH <= 0 || columns <= 0 || rows <= 0) return false;
    _tile.reset();
    _imageIndex = -1;
    if (!TileMap::loadImageForTileBank(path, _imageIndex)) {
        return false;
    }
    // Build full grid rects
    _tileW = tileW; _tileH = tileH;
    _cols = columns; _rows = rows;

    TileMap::TileModel model{};
    model.assetPath = path;
    model.properties = TileMap::TileProperties::Passable;
    model.frameDuration = 0.2f;
    model.tileRects.reserve(_cols * _rows);
    model.tint.reserve(_cols * _rows);
    for (int y = 0; y < _rows; ++y) {
        for (int x = 0; x < _cols; ++x) {
            model.tileRects.push_back(SDL_FRect{ (float)(x * tileW), (float)(y * tileH), (float)tileW, (float)tileH });
            model.tint.push_back(Renderer::Color{255,255,255,255});
        }
    }
    if (!TileMap::createTileFromBank(_imageIndex, model, _tile)) {
        return false;
    }
    return true;
}

SDL_FRect PortraitManager::tileRect(int index) const {
    if (!isLoaded()) return SDL_FRect{0,0,0,0};
    // If cols/rows are not yet known, infer a conservative grid of 1 row based on current tileW/H
    int cols = _cols > 0 ? _cols : 1;
    int rows = _rows > 0 ? _rows : 1;
    const int count = cols * rows;
    if (index < 0 || index >= count) return SDL_FRect{0,0,0,0};
    const int tx = index % cols;
    const int ty = index / cols;
    SDL_FRect r{ 0.0f + tx * (float)_tileW,
                 0.0f + ty * (float)_tileH,
                 (float)_tileW, (float)_tileH };
    return r;
}

void PortraitManager::draw(int index, Vector2 position, Vector2 scale, float rotation, Renderer::Color tint) const {
    if (!isLoaded()) return;
    _tile->activeFrame = std::min(index, (int)_tile->tileRects.size() - 1);
    _tile->tint.resize(_tile->tileRects.size(), tint);
    _tile->tint[_tile->activeFrame] = tint;

    TileMap::TileTransform tf{};
    tf.position = position;
    tf.scale = scale;
    tf.rotation = rotation;
    TileMap::renderTile(tf, _tile);
}

} // namespace AvatarQuest
