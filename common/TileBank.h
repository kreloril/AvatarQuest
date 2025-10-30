#pragma once

namespace TileMap {

  //  struct Renderer::Color;

     enum struct TileProperties {
      Passable = 0,
      Blocking = 1,
      Water = 2,
      SlowProgress = 4,
      Poisonous = 8,
      Fire = 16,
      Ice = 32,
      Window = 64,   
      Door = 128,
      Stairs = 256,
      Teleport = 512,
      Treasure = 1024
    };

     struct TileModel {
         TileProperties properties;
		 String assetPath; // Path to the image asset
         float frameDuration = 0.2f; // seconds per frame
         Vector<Renderer::Color> tint;
         Vector<SDL_FRect> tileRects;
     };

     struct Tile {
         TileProperties properties = TileProperties::Passable;
         float frameDuration = 0.2f; // seconds per frame
         Vector<Renderer::Color> tint;
         Vector<SDL_FRect> tileRects;
         float frameTime = 0.0f;
         int imageIndex = 0; // Index into the image bank
         int activeFrame = 0;

         template<typename type>
         bool isPropertySet(type prop) const {
             return (static_cast<int>(properties) & static_cast<int>(prop)) != 0;
		 }

         Vector2 getSize() const {
             if (tileRects.empty()) return { 0.0f, 0.0f };
             return { tileRects[activeFrame].w, tileRects[activeFrame].h };
         }
     };

     struct TileTransform {
         Vector2 position;
         Vector2 scale = { 1.0f,1.0f };
         float rotation = 0; // in degrees
		 int tileIndex = -1; // Index into the tile array
	 };
    
     bool loadFromModelArray(const TileMap::TileModel* models, int count, VectorRef<TileMap::Tile>& tiles);
	 bool loadImageForTileBank(const char* filename, int& outImageIndex);
	 bool createTileFromBank(int imageIndex, TileModel&, Ref<Tile>& outTile);
	 void releaseImageFromBank(int imageIndex);
	 void renderTile(TileTransform& transform, const Ref<Tile>& tiles);
     void renderTiles(Vector<TileTransform>& positions, VectorRef<Tile>& tiles);
	 void updateTileSets(float deltaTime, Vector<TileTransform>& positions,VectorRef<Tile>& );

}