#pragma once

#include "Common.h"
#include "Renderer.h"
#include "TileBank.h"

namespace AvatarQuest {

// Simple manager for a tile-based portrait sheet (PNG atlas).
// Loads a single image and exposes tile rects and drawing helpers.
class PortraitManager {
public:
    PortraitManager() = default;

    // Load the portrait sheet and set tile size in pixels.
    // Returns false on error.
    bool load(const char* path, int tileW, int tileH);
    // Overload with explicit grid size (columns/rows)
    bool load(const char* path, int tileW, int tileH, int columns, int rows);

    // Query
    bool isLoaded() const { return (bool)_tile; }
    int tileWidth() const { return _tileW; }
    int tileHeight() const { return _tileH; }
    int columns() const { return _cols; }
    int rows() const { return _rows; }
    int tileCount() const { return _cols * _rows; }

    // Compute the SDL_FRect for a given tile index (row-major). Returns an empty rect if invalid.
    SDL_FRect tileRect(int index) const;

    // Access underlying TileMap::Tile for UI binding
    Ref<TileMap::Tile> tile() const { return _tile; }

    // Convenience draw: render the portrait tile at position with scale/rotation and tint.
    void draw(int index, Vector2 position, Vector2 scale = {1.0f, 1.0f}, float rotation = 0.0f,
              Renderer::Color tint = {255,255,255,255}) const;

private:
    Ref<TileMap::Tile> _tile;
    int _imageIndex = -1;
    int _tileW = 0;
    int _tileH = 0;
    int _cols = 0;
    int _rows = 0;
};

} // namespace AvatarQuest
