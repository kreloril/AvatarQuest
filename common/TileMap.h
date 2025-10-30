#pragma once


namespace TileMap {



	bool initMap(TileVector mapSize, TileVector tileSize, TileMap::TileModel* tiles, int tileCount, int& mapId);
	bool setMapData(int mapId, SDL_FRect& viewPort, const Vector<int>& mapData);
	void setMapIndex(int mapId, TileVector& tv, int tileIndex);
	void getMapIndex(int mapId, TileVector& tv, int& tileIndex);
	void getMapTiles(int mapId, Vector<Ref<TileMap::Tile>>& outTiles);

	bool loadMap(TileVector mapSize, TileVector tileSize, TileMap::TileModel* tiles, int tileCount, const char* mapDataFile, int& mapId);
	bool saveMap(const char* mapDataFile, int mapId);
	bool updateMap(float deltaTime, Vector2& viewPosition, int& mapId);
	void renderMap(int mapId);
	void shutDownMap(int& mapId);
    
}

