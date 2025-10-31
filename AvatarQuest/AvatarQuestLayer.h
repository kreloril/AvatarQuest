#pragma once
#include "Common.h"
#include "AvatarQuestPlayer.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif



namespace AvatarQuest {

class AvatarQuestLayer : public Game::UILayer {
public:
    AvatarQuestLayer();
    ~AvatarQuestLayer() override;

    void init() override;
    void update(float deltaTime) override;
    void render(float deltaTime) override;
    void shutDown() override;
    void handleEvents(float delta, Game::GameEvents& events) override;

private:

    SDL_FRect _mouseRect;
	int _mapIndex= -1;
    float playerVelocity = 0.0f;
	Ref<AvatarQuest::PlayerCamera> _playerCamera;
	// HUD text font (Droid Serif Bold, or fallback)
	Text::Font* _hudFont = nullptr;
	

	const TileVector _mapSize = { 128, 128 };
	const TileVector _tileSize = { 128, 128 };
	
	
	UMap<TileVector,int> _visableTileMap;
	Vector<TileMap::TileTransform> _visableTiles;
	VectorRef<TileMap::Tile> _tiles;
	TileVector tileLocFromWorldPos(const Vector2& worldPos);
	Vector2 WorldPosFromTileLoc(const TileVector& tileLoc);
	Vector2 tileAtScreenCenter(const Vector2& playerWorldPos, const TileVector& tileSize);

	bool movePlayer(PlayerMovement moveDir, float deltaTime);
	
	void buildVisibleTilesRect(SDL_FRect& windowSize,
		TileVector tileSize,       // in pixels
		const TileVector& playerTilePosition,
		int mapIndex,
		Vector<TileMap::TileTransform>& outVisible,
		UMap<TileVector, int>& vMap);

#ifdef AVATARQUEST_ENABLE_AUDIO
    // Test sound effect for quick validation
    Sound::Sfx* _testSfx = nullptr;
	// Test music track (MP3)
	Sound::Music* _testMusic = nullptr;
	// Simple runtime volume controls
	int _sfxVolume = 128;   // 0..128
	int _musicVolume = 128; // 0..128
#endif
};

Ref<Game::UILayer> CreateAvatarQuestLayer();

}

#include "AvatarQuestRP.h"

