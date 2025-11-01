#include "AvatarQuestMap.h"

using namespace AvatarQuest;

void WorldMap::init(const TileVector& mapSize, const TileVector& tileSize)
{
	_mapSize = mapSize;
	_tileSize = tileSize;

	// Minimal tile models (two grass variants)
	static Vector<TileMap::TileModel> tileModels = {
		{
			.properties = TileMap::TileProperties::Passable,
			.assetPath = "assets//grass4_2.png",
			.frameDuration = 0.2f,
			.tint = { {255,255,255,255} },
			.tileRects = { {0,0,128,128} }
		},
		{
			.properties = TileMap::TileProperties::SlowProgress,
			.assetPath = "assets//grass4_1.png",
			.frameDuration = 0.2f,
			.tint = { {255,255,255,255} },
			.tileRects = { {0,0,128,128} }
		}
	};

	TileMap::initMap(_mapSize, _tileSize, tileModels.data(), (int)tileModels.size(), _mapIndex);
	TileMap::getMapTiles(_mapIndex, _tiles);
}

void WorldMap::buildGeneratedTerrain()
{
	Vector<int> tileMap;
	const int count = _mapSize.x * _mapSize.y;
	tileMap.resize(count, 0);

	for (int y = 0; y < _mapSize.y; ++y) {
		for (int x = 0; x < _mapSize.x; ++x) {
			const int index = y * _mapSize.x + x;
			const bool x1 = (x % 2) == 0;
			const bool y1 = (y % 2) == 1;
			const bool xy = y1 && x1 == 0;
			const int tileIndex = xy ? 1 : 0;
			tileMap[index] = tileIndex;
		}
	}
	SDL_FRect dummy{};
	(void)dummy;
	// Upload to engine TileMap
	TileMap::setMapData(_mapIndex, dummy, tileMap);
}

void WorldMap::updateVisible(const SDL_FRect& windowSize, const PlayerCamera& camera)
{
	buildVisibleTilesRect(windowSize, camera.playerTilePosition, camera, _visibleTiles, _visIndex);
}

void WorldMap::render()
{
	TileMap::renderTiles(_visibleTiles, _tiles);
}

TileVector WorldMap::tileLocFromWorldPos(const Vector2& worldPos) const
{
	const float tw = std::max(1.0f, (float)_tileSize.x);
	const float th = std::max(1.0f, (float)_tileSize.y);
	return { (int)std::floor(worldPos.x / tw), (int)std::floor(worldPos.y / th) };
}

Vector2 WorldMap::worldPosFromTileLoc(const TileVector& tileLoc) const
{
	const float tw = std::max(1.0f, (float)_tileSize.x);
	const float th = std::max(1.0f, (float)_tileSize.y);
	return { tileLoc.x * tw, tileLoc.y * th };
}

static constexpr int kGuardTiles = 1;

void WorldMap::buildVisibleTilesRect(const SDL_FRect& windowSize,
									 const TileVector& playerTilePosition,
									 const PlayerCamera& camera,
									 Vector<TileMap::TileTransform>& outVisible,
									 UMap<TileVector, int>& vMap)
{
	outVisible.clear();
	vMap.clear();

	const float tw = (float)_tileSize.x;
	const float th = (float)_tileSize.y;

	const int cols = (int)std::ceil(windowSize.w / tw) + 2 * kGuardTiles;
	const int rows = (int)std::ceil(windowSize.h / th) + 2 * kGuardTiles;
	const int halfCols = cols / 2;
	const int halfRows = rows / 2;

	const int mapW = _mapSize.x;
	const int mapH = _mapSize.y;

	int startX = std::max(0, playerTilePosition.x - halfCols);
	int startY = std::max(0, playerTilePosition.y - halfRows);
	startX = std::min(startX, std::max(0, mapW - cols));
	startY = std::min(startY, std::max(0, mapH - rows));

	const int tilesX = std::min(cols, mapW - startX);
	const int tilesY = std::min(rows, mapH - startY);

	if (tilesX > 0 && tilesY > 0) {
		outVisible.reserve(tilesX * tilesY);
	}

	const float anchorX = camera.playerCenterScreen.x;
	const float anchorY = camera.playerCenterScreen.y;

	const float startPx = anchorX - ((playerTilePosition.x - startX) + 0.5f) * tw;
	const float startPy = anchorY - ((playerTilePosition.y - startY) + 0.5f) * th;

	for (int jy = 0; jy < tilesY; ++jy) {
		const int ty = startY + jy;
		const float yPos = startPy + jy * th;

		for (int jx = 0; jx < tilesX; ++jx) {
			const int tx = startX + jx;
			const float xPos = startPx + jx * tw;
			TileVector tv = { tx, ty };
			int tileIndex = -1;
			TileMap::getMapIndex(_mapIndex, tv, tileIndex);
			if (tileIndex < 0) continue;

			Vector2 pos = { xPos, yPos };
			{
				const float x2 = pos.x + tw;
				const float y2 = pos.y + th;
				if (x2 < windowSize.x || y2 < windowSize.y || pos.x > (windowSize.x + windowSize.w) || pos.y > (windowSize.y + windowSize.h)) {
					continue;
				}
			}

			TileMap::TileTransform tt;
			tt.position = pos;
			tt.scale = { 1.0f, 1.0f };
			tt.rotation = 0.0f;
			tt.tileIndex = tileIndex;

			vMap[tv] = (int)outVisible.size();
			outVisible.push_back(tt);
		}
	}
}
