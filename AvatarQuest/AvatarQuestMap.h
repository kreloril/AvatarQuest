#pragma once

#include "Common.h"
#include "TileMap.h"
#include "AvatarQuestPlayer.h"

namespace AvatarQuest {

// Lightweight wrapper around engine TileMap to keep map logic out of the layer
class WorldMap {
public:
    void init(const TileVector& mapSize, const TileVector& tileSize);
    // For now, procedurally generate a checker and upload into TileMap
    void buildGeneratedTerrain();

    void updateVisible(const SDL_FRect& windowSize, const PlayerCamera& camera);
    void render();

    // Helpers
    TileVector tileLocFromWorldPos(const Vector2& worldPos) const;
    Vector2    worldPosFromTileLoc(const TileVector& tileLoc) const;

    // Accessors
    const TileVector& mapSize() const { return _mapSize; }
    const TileVector& tileSize() const { return _tileSize; }

private:
    void buildVisibleTilesRect(const SDL_FRect& windowSize,
                               const TileVector& playerTilePosition,
                               const PlayerCamera& camera,
                               Vector<TileMap::TileTransform>& outVisible,
                               UMap<TileVector, int>& vMap);

    TileVector _mapSize{128,128};
    TileVector _tileSize{128,128};
    int _mapIndex = -1;

    VectorRef<TileMap::Tile> _tiles;
    Vector<TileMap::TileTransform> _visibleTiles;
    UMap<TileVector, int> _visIndex;
};

}