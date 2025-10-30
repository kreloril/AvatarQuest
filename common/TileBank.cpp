#include "Common.h"


int hashFileName(const char* filename) {
	// Simple hash function (djb2)
	unsigned long hash = 5381;
	int c;
	while ((c = *filename++)) {
		hash = ((hash << 5) + hash) + c; // hash * 33 + c
	}
	return static_cast<int>(hash);
}

static UMap<int, Ref<Renderer::Image>> g_imageBank;

bool TileMap::loadFromModelArray(const TileMap::TileModel* models, int count, VectorRef<TileMap::Tile>& tiles)
{
	for (int i =0; i < count; ++i) {
		const auto& model = models[i];
		int imageIndex = -1;
		if (!loadImageForTileBank(model.assetPath.c_str(), imageIndex)) {
			return false;
		}
		Ref<TileMap::Tile> newTile;
		if (!createTileFromBank(imageIndex, const_cast<TileMap::TileModel&>(model), newTile)) {
			return false;
		}

		tiles.push_back(newTile);
	}
	return true;
}

bool TileMap::loadImageForTileBank(const char* filename, int& outImageIndex)
{
    if (!filename || !*filename)
		return false;

	int hash = hashFileName(filename);
	
	auto it = g_imageBank.find(hash);
	if (it != g_imageBank.end()) {
		outImageIndex = hash;
		return true; // Already loaded
	}
	Ref<Renderer::Image> newImage;
	if (!loadImageFromFile(filename, newImage)) {
		return false;
	}
	g_imageBank[hash] = newImage;
	outImageIndex = hash;

    return true;
}

bool TileMap::createTileFromBank(int imageIndex, TileModel& properties, Ref<TileMap::Tile>& outTile)
{
	
	auto it = g_imageBank.find(imageIndex);
	if (it == g_imageBank.end()) {
		return false; // Image not found
	}

	outTile = CreateRef<TileMap::Tile>();
	
	outTile->properties = properties.properties;
	outTile->frameDuration = properties.frameDuration;
	outTile->tint = properties.tint;
	outTile->tileRects = properties.tileRects;
	outTile->imageIndex = imageIndex;
	outTile->activeFrame = 0;
	outTile->frameTime = 0.0f;

	return true;
}


void TileMap::releaseImageFromBank(int imageIndex)
{
	auto it = g_imageBank.find(imageIndex);
	if (it != g_imageBank.end()) {
		g_imageBank.erase(it);
	}

}

void TileMap::renderTile(TileTransform& transform, const Ref<Tile>& tile)
{

	auto it = g_imageBank.find(tile->imageIndex);
	if (it == g_imageBank.end()) {
		return; // Image not found
	}
	auto& store = it->second;
	if (!store) return;
	if (tile->tileRects.empty()) return;
	drawImageFromRect(transform.position, transform.scale, transform.rotation, store, tile->tileRects[tile->activeFrame], tile->tint[tile->activeFrame]);
}

void TileMap::renderTiles(Vector<TileTransform>& positions, VectorRef<Tile>& tiles)
{
	for (auto& transform : positions) {
		auto& tile = tiles[transform.tileIndex];
		renderTile(transform, tile);
	}
}

void TileMap::updateTileSets(float deltaTime, Vector<TileTransform>& positions, VectorRef<Tile>& tiles)
{
	for (auto& transform : positions) {
			auto& tile = tiles[transform.tileIndex];

			tile->frameTime += deltaTime;
			if (tile->frameTime >= tile->frameDuration) {
				tile->frameTime = 0.0f;
				tile->activeFrame = (tile->activeFrame + 1) % tile->tileRects.size();
			}
	}
}

