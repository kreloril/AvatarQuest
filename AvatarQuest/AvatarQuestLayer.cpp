

#include "AvatarQuestLayer.h"
#include "AvatarQuestPlayer.h"

#include <vector>
#include <cstring>

namespace AvatarQuest {

AvatarQuestLayer::AvatarQuestLayer() {}
AvatarQuestLayer::~AvatarQuestLayer() { shutDown(); }


/*const char* waterTextures[] = {
    "assets//box1//box1_1.png",
    "assets//box1//box1_2.png",
    "assets//box1//box1_3.png",
    "assets//box1//box1_4.png",
    "assets//box1//box1_5.png",
    "assets//box1//box1_6.png",

  
};*/

/*

const char* waterTextures[] = {
        "assets//tile_001.png",
         "assets//tile_002.png",
                 "assets//tile_003.png"


};
*/



Vector<TileMap::TileModel> tileModels = { 
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


Vector2 tileAdjacent[] = {
    {1,1},  // Up-Right
    {0,-1},  // Up
    {-1,1},  // Up-Left

    {0,1},   // Down
    {-1,0},  // Left
    {1,0},    // Right
    { 1,-1 },
    { -1,1},
    {-1,-1},
    {0,0}     // Center (player tile)
};

void AvatarQuestLayer::init()
{
	Renderer::setClearColor({ 0, 0, 0, 255 }); // Black clear color

    TileMap::initMap(_mapSize, _tileSize,
		tileModels.data(), (int)tileModels.size(), _mapIndex);

	TileMap::getMapTiles(_mapIndex, _tiles);   

	Vector<int> tileMap;
	int mapsize = _mapSize.x * _mapSize.y;
	tileMap.resize(mapsize, 0);

    for (int y = 0; y < _mapSize.y; ++y) {
        for (int x = 0; x < _mapSize.x; ++x) {
            int index = y * _mapSize.x + x;
			bool x1 = (x % 2) == 0;
			bool y1 = (y % 2) == 1;

            bool xy = y1 && x1 == 0;
        
			int tileIndex = xy ? 1 : 0;

            tileMap[index] = tileIndex; // All grass for now
        }
    }
    auto windowSize = Window::getWindowSize();
    // Create player camera
    TileVector startingPosition{ 64, 64 };
    Ref<PlayerCamera> pc;
    createPlayer(windowSize, startingPosition, pc);
    _playerCamera = pc;

    TileMap::setMapData(_mapIndex, _playerCamera->playerViewport, tileMap);
   

    // Load HUD font (try common relative paths based on run dir)
    if (!_hudFont) {
        const char* fontCandidates[] = {
            "assets/fonts/DroidSerifBold-aMPE.ttf",
            "../assets/fonts/DroidSerifBold-aMPE.ttf",
            "../../assets/fonts/DroidSerifBold-aMPE.ttf"
        };
        for (const char* p : fontCandidates) {
            _hudFont = Text::loadFont(p, 22);
            if (_hudFont) {
                SDL_Log("Loaded HUD font: %s", p);
                break;
            }
        }
        if (!_hudFont) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load HUD font (DroidSerifBold-aMPE.ttf) from known locations.");
        }
    }

#ifdef AVATARQUEST_ENABLE_AUDIO
    // Load a test SFX to validate audio using example asset
    if (!_testSfx) {
        const char* sfxCandidates[] = {
            "assets/wav/example.wav",
            "../assets/wav/example.wav",
            "../../assets/wav/example.wav"
        };
        for (const char* p : sfxCandidates) {
            _testSfx = Sound::loadSfx(p);
            if (_testSfx) {
                SDL_Log("Loaded test SFX: %s", p);
                break;
            }
        }
        if (!_testSfx) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load test SFX (assets/wav/example.wav) from known locations.");
        }
    }

    // Load a test MP3 music file
    if (!_testMusic) {
        const char* musicCandidates[] = {
            "assets/mp3/way-home.mp3",
            "../assets/mp3/way-home.mp3",
            "../../assets/mp3/way-home.mp3"
        };
        for (const char* p : musicCandidates) {
            _testMusic = Sound::loadMusic(p);
            if (_testMusic) {
                SDL_Log("Loaded test music: %s", p);
                break;
            }
        }
        if (!_testMusic) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load test music (assets/mp3/way-home.mp3) from known locations.");
        }
    }
#endif


}

void AvatarQuestLayer::render(float deltaTime)
{


 //   Renderer::renderMap(_mapIndex);
	TileMap::renderTiles(_visableTiles, _tiles);

	//Vector2 spriteSize = { 128.0f,128.0f };
   // auto windowSize = Window::getWindowSize(); // _playerWorldPosition
   // TileVector startPos = _playerWorldPosition;// tileLocFromWorldPos({ (float)windowSize.w / 2.0f, (float)windowSize.h / 2.0f });
    Vector2 startPosW = _playerCamera->playerCenterScreen; // WorldPosFromTileLoc(startPos);
//	Vector2 pos = { startPos.x , startPos.y * (float)spriteSize.y - 0 };
	Renderer::drawRect(startPosW.x , startPosW.y , (float)_tileSize.x, (float)_tileSize.y, { 0,255,0,255 });
    Renderer::drawRect(_mouseRect.x, _mouseRect.y, _mouseRect.w, _mouseRect.h, { 255,0,0,255 });

    // Draw a simple HUD overlay text in the top-left
    if (_hudFont) {
        const std::string hud = "WASD to move   •   Esc to quit";
        SDL_FRect size = Text::measure(_hudFont, hud);
        const float padX = 8.0f;
        const float padY = 6.0f;
        // Background for readability
        Renderer::drawFilledRect(8.0f - padX * 0.5f, 8.0f - padY * 0.5f, size.w + padX, size.h + padY, { 0,0,0,90 });
        Text::draw(_hudFont, hud, 8.0f, 8.0f, SDL_Color{ 255,255,255,255 });
    }
 //   Renderer::drawRect(tile->po, _playerstartY, tileSize, tileSize, { 0,255,0,255 });
}

void AvatarQuestLayer::shutDown()
{
	//Renderer::releaseImageFromBank(0);
    if (_hudFont) {
        Text::unloadFont(_hudFont);
        _hudFont = nullptr;
    }
#ifdef AVATARQUEST_ENABLE_AUDIO
    if (_testSfx) {
        Sound::unloadSfx(_testSfx);
        _testSfx = nullptr;
    }
    if (_testMusic) {
        Sound::unloadMusic(_testMusic);
        _testMusic = nullptr;
    }
#endif
}

void AvatarQuestLayer::handleEvents(float /*delta*/, Game::GameEvents& events) {
    using Game::GameEvents;
    if (events.isEventType(GameEvents::EventType::KeyPress)) {
        // SDL keycodes are forwarded in keyEvent.keyCode
        if (events.keyEvent.keyCode == SDLK_ESCAPE) {
            Game::endGameLoop();
            return;
        }
    }

    switch (events.type) {

    case GameEvents::EventType::KeyRelease: {
		events.reset();
        if (_playerCamera) _playerCamera->currentMovement = PlayerMovement::Move_None;
        break;
    }

    case GameEvents::EventType::KeyPress: {

        switch (events.keyEvent.keyCode) {

			    // switch between tile sets
  /*          case SDLK_LEFTBRACKET: {
                if (selectedTileIndex != -1) {
				    auto& tilePos = m_visableTiles[selectedTileIndex];
				    int curentTileIndex = tileMap[selectedTileIndex];  
				    curentTileIndex = (curentTileIndex - 1 + (int)_tiles.size()) % (int)_tiles.size();
				    tileMap[selectedTileIndex] = curentTileIndex;
				    m_visableTiles[selectedTileIndex].tileIndex = curentTileIndex;
                }
			    events.reset();

                break;
            }

            case SDLK_RIGHTBRACKET: {
                if (selectedTileIndex != -1) {
                    auto& tilePos = m_visableTiles[selectedTileIndex];
                    int curentTileIndex = tileMap[selectedTileIndex];
                    curentTileIndex = (curentTileIndex + 1 + (int)_tiles.size()) % (int)_tiles.size();
				    tileMap[selectedTileIndex] = curentTileIndex;
                    m_visableTiles[selectedTileIndex].tileIndex = curentTileIndex;
                }
			    events.reset();
                break;
            }
            */
            case SDLK_W: {
                if (_playerCamera) _playerCamera->currentMovement = PlayerMovement::Move_Up;
            //    events.reset();
				break;
            }
            case SDLK_S: {
                if (_playerCamera) _playerCamera->currentMovement = PlayerMovement::Move_Down;
             //   events.reset();
				break;
            }
            case SDLK_A: {
                if (_playerCamera) _playerCamera->currentMovement = PlayerMovement::Move_Left;
             //   events.reset();
                break;
            }
            case SDLK_D: {
                if (_playerCamera) _playerCamera->currentMovement = PlayerMovement::Move_Right;
              //  events.reset();
                break;
            }

#ifdef AVATARQUEST_ENABLE_AUDIO
                        case SDLK_SPACE: {
                                if (_testSfx) {
                                        // Play once at full volume
                                        Sound::playSfx(_testSfx, /*loops*/0, /*channel*/-1, /*volume*/128);
                                }
                                break;
                        }
            case SDLK_P: {
                if (_testMusic) {
                    // Loop indefinitely at medium-high volume
                    Sound::playMusic(_testMusic, /*loops*/-1, /*volume*/100);
                }
                break;
            }
            case SDLK_O: {
                // Fade out music over 1 second
                Sound::stopMusic(1000);
                break;
            }
#endif




              default:break;
            }

        break;
    }


    case GameEvents::EventType::MouseButtonPress: {

        switch (events.mouseButtonEvent.button) {

        case GameEvents::MouseButton::Left: {
            events.reset();
    /*        Vector2 mousPs = {(float)events.mouseButtonEvent.x, (float)events.mouseButtonEvent.y};
            for (int i = 0; i < (int)m_visableTiles.size(); ++i) {

                auto& tilePos = m_visableTiles[i];
				auto& tile = _tiles[tilePos.tileIndex];
                auto pos = tilePos.position;
                auto tileSize = tile->getSize();

                bool insideX = mousPs.x >= (tilePos.position.x - tileSize.x * 0.5f) && mousPs.x <= (tilePos.position.x + tileSize.x * 0.5f);
                bool insideY = mousPs.y >= (tilePos.position.y - tileSize.y * 0.5f) && mousPs.y <= (tilePos.position.y + tileSize.y * 0.5f);

                if (insideX && insideY) {
					selectedTileIndex = i;
                    _mouseRect = { tilePos.position.x - tileSize.x * 0.5f,tilePos.position.y - tileSize.y * 0.5f,tileSize.x,tileSize.y };
                    break;
                }
                else {
                    selectedTileIndex = -1;
					_mouseRect = { 0,0,0,0 };
                }

            }
            */
        }
				break;

            case GameEvents::MouseButton::Right:
                events.reset();
                // Handle right mouse button press
                break;
            case GameEvents::MouseButton::Middle:
                events.reset();
                // Handle middle mouse button press
                break;
            default:
				break;

        }

       
        break;
    }

    default: 
        break;
    }




   /* else if (events.isEventType(events.type == GameEvents::EventType::MouseButtonPress)) {
		Vector2 mousPs = { (float)events.mouseButtonEvent.x, (float)events.mouseButtonEvent.y };
        for (int i = 0; i < (int)m_visableTiles.size(); ++i) {

			auto& tilePos = m_visableTiles[i];
			auto pos = tilePos.position;
			auto tileSize = tilePos.tile->getSize();

			bool insideX = mousPs.x >= (tilePos.position.x - tileSize.x * 0.5f) && mousPs.x <= (tilePos.position.x + tileSize.x * 0.5f);
			bool insideY = mousPs.y >= (tilePos.position.y - tileSize.y * 0.5f) && mousPs.y <= (tilePos.position.y + tileSize.y * 0.5f);

            if (insideX && insideY) {
				_mouseRect = { pos.x,pos.y,tileSize.x,tileSize.y };
                break;
            }

        }
    }*/
}

bool  AvatarQuestLayer::movePlayer(PlayerMovement moveDir, float deltaTime)
{

        if (moveDir == PlayerMovement::Move_None) {
		return false;
    }
    

    int distance = 1;
    int newX = _playerCamera ? _playerCamera->playerTilePosition.x : 0;
    int newY = _playerCamera ? _playerCamera->playerTilePosition.y : 0;
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      switch (moveDir) {
    case PlayerMovement::Move_Up: {
        newY -= distance;
        break;
    }
    case PlayerMovement::Move_Down: {

        newY += distance;
        break;
    }
    case PlayerMovement::Move_Left: {
        newX -= distance;
        break;
    }

    case PlayerMovement::Move_Right: {
        newX += distance;
        break;
    }
    default: return false;

    }

    if (_playerCamera) {
        _playerCamera->playerTilePosition.x = newX;
        _playerCamera->playerTilePosition.y = newY;
        _playerCamera->playerWorldPosition = WorldPosFromTileLoc(_playerCamera->playerTilePosition);
    }
    return true;

}



Vector2 AvatarQuestLayer::tileAtScreenCenter(const Vector2& playerWorldPos, const TileVector& tileSize)
{
    return { std::floor(playerWorldPos.x / tileSize.x),
              std::floor(playerWorldPos.y / tileSize.y) };
}

TileVector AvatarQuestLayer::tileLocFromWorldPos(const Vector2& worldPos)
{
/*	Vector2 startingpos = {_playerViewport.x,_playerViewport.y};
	Vector2 tileLoc = (worldPos - startingpos) / (float)tileSize;
    return tileLoc;*/

    const float tw = std::max(1.0f, (float)_tileSize.x);
    const float th = std::max(1.0f, (float)_tileSize.y);


    // Map a world position to an absolute tile index in the world grid
    return { (int)std::floor(worldPos.x / tw), (int)std::floor(worldPos.y / th) };
}

Vector2 AvatarQuestLayer::WorldPosFromTileLoc(const TileVector& tileLoc)
{
//	Vector2 startingpos = { _playerViewport.x,_playerViewport.y };/
//	Vector2 tLoc = (tileLoc * (float)tileSize) + startingpos;
//    return tLoc;
    const float tw = std::max(1.0f, (float)_tileSize.x);
    const float th = std::max(1.0f, (float)_tileSize.y);
    return { tileLoc.x * tw, tileLoc.y * th };
}

constexpr int kGuardTiles = 1;

void AvatarQuestLayer::buildVisibleTilesRect(SDL_FRect& windowSize,
    TileVector tileSize,       // in pixels
    const TileVector& playerTilePosition,
    int mapIndex,
    Vector<TileMap::TileTransform>& outVisible,
    UMap<TileVector, int>& vMap)
{
    outVisible.clear();
    vMap.clear();

    const float tw = (float)tileSize.x;
    const float th = (float)tileSize.y;

    // --- tiles that fit across and down (plus a small guard) ---
    const int cols = (int)std::ceil(windowSize.w / tw) + 2 * kGuardTiles;
    const int rows = (int)std::ceil(windowSize.h / th) + 2 * kGuardTiles;
    const int halfCols = cols / 2;
    const int halfRows = rows / 2;

    // --- clamp the tile scan to the map bounds, keeping full coverage when possible ---
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

    // --- screen anchor: where the player's tile center is drawn (SCREEN pixels) ---
    // IMPORTANT: _playerCenterScreen must be the actual screen center of the window rect,
    // e.g.  { windowRect.x + windowRect.w*0.5f, windowRect.y + windowRect.h*0.5f }
    const float anchorX = _playerCamera ? _playerCamera->playerCenterScreen.x : 0.0f;
    const float anchorY = _playerCamera ? _playerCamera->playerCenterScreen.y : 0.0f;

    // --- top-left pixel of the first tile we will draw ---
    // player�s tile center is at anchor; move left/up by (#tiles from start to player + 0.5 tile)
    const float startPx = anchorX - ((playerTilePosition.x - startX) + 0.5f) * tw;
    const float startPy = anchorY - ((playerTilePosition.y - startY) + 0.5f) * th;

    // --- emit tiles in strict tile order (row-major) ---
    for (int jy = 0; jy < tilesY; ++jy) {
        const int ty = startY + jy;
        const float yPos = startPy + jy * th;

        for (int jx = 0; jx < tilesX; ++jx) {
            const int tx = startX + jx;
            const float xPos = startPx + jx * tw;
            TileVector tv = { tx, ty };
            int tileIndex = -1;
            TileMap::getMapIndex(mapIndex,tv, tileIndex);
            if (tileIndex < 0) continue; // empty/OOB

			Vector2 pos = { xPos, yPos };

            // cull anything outside the window rect (optional; comment out to keep guard tiles)
            {
                const float x2 = pos.x + tw;
                const float y2 = pos.y + th;
                if (x2 < windowSize.x || y2 < windowSize.y || pos.x > (windowSize.x + windowSize.w) || pos.y > (windowSize.y + windowSize.h)) {
                    // skip fully off-screen tiles
                    continue;
                }
			}

            TileMap::TileTransform tt;
            tt.position = pos;   // SCREEN space (already includes any window border via _playerCenterScreen)
            tt.scale = { 1.0f, 1.0f };
            tt.rotation = 0.0f;
            tt.tileIndex = tileIndex;

            vMap[tv] = (int)outVisible.size(); // tile -> transform index
            outVisible.push_back(tt);
        }
    }




  /*  const float tw = static_cast<float>(tileSize.x);
    const float th = static_cast<float>(tileSize.y);

	const float sw = windowSize.w - windowSize.x;
	const float sh = windowSize.h - windowSize.y;

	const float tilesW = sw / tw;
	const float tilesH = sh / th;

	const float ftileWHalfW = tilesW / 2.0f;
	const float ftileHHalfH = tilesH / 2.0f;



    TileVector tileAtCenter = tileLocFromWorldPos(_playerCenterScreen);

    int halfTileWidth = static_cast<int>(std::floor(ftileWHalfW));
    int halfTileHeight = static_cast<int>(std::floor(ftileHHalfH));

    

    TileVector startTile = { _playerTilePosition.x - tileAtCenter.x, _playerTilePosition.y - tileAtCenter.y };

	Vector2 tilePosition = _playerCenterScreen;
	tilePosition.x = _playerCenterScreen.x - (halfTileWidth * tw);
	tilePosition.y = _playerCenterScreen.y - (halfTileHeight * th);
    TileMap::TileTransform tt;
	tt.position = tilePosition;
	tt.scale = { 1.0f, 1.0f };
	int index = -1;
	TileMap::getMapIndex(mapIndex, startTile, index);
	tt.tileIndex = index;
	outVisible.push_back(tt);
    vMap[startTile] = static_cast<int>(outVisible.size()); // tile -> transform index
   */
  //  TileTrans


    // ---- 1) Camera offset in pixels (world -> screen)
    // Player tile center in world px: (tx + 0.5, ty + 0.5) * tileSize
 /*   const float playerCenterWorldX = (playerTilePosition.x + 0.5f) * tw;
    const float playerCenterWorldY = (playerTilePosition.y + 0.5f) * th;

    // Camera top-left in world px = where (0,0) on screen maps to
    // Put player's tile center exactly at _playerCenterScreen.
    const float camX = playerCenterWorldX - _playerCenterScreen.x;
    const float camY = playerCenterWorldY - _playerCenterScreen.y;

    // ---- 2) Visible tile index range from camera + screen size (+ guard)
    int tx0 = static_cast<int>(std::floor(camX / tw)) - kGuardTiles;
    int ty0 = static_cast<int>(std::floor(camY / th)) - kGuardTiles;
    int tx1 = static_cast<int>(std::ceil((camX + windowSize.w) / tw)) + kGuardTiles - 1;
    int ty1 = static_cast<int>(std::ceil((camY + windowSize.h) / th)) + kGuardTiles - 1;

    // Clamp to map bounds
    const int mapW = _tileSize.x;
    const int mapH = _tileSize.y;
    tx0 = std::max(0, std::min(tx0, mapW - 1));
    ty0 = std::max(0, std::min(ty0, mapH - 1));
    tx1 = std::max(0, std::min(tx1, mapW - 1));
    ty1 = std::max(0, std::min(ty1, mapH - 1));

    if (tx1 >= tx0 && ty1 >= ty0) {
        outVisible.reserve((tx1 - tx0 + 1) * (ty1 - ty0 + 1));
    }
    
    // emit in strict location order: rows (y) then columns (x)
    for (int ty = ty0; ty <= ty1; ++ty) {
        for (int tx = tx0; tx <= tx1; ++tx) {

            int tileIndex = -1;
			TileVector tv = { tx, ty };
            TileMap::getMapIndex(mapIndex, tv, tileIndex);
         
            if (tileIndex < 0) continue;

            const float worldX = tx * tw;
            const float worldY = ty * th;

            TileMap::TileTransform tt;
            tt.position.x = worldX - camX;  // screen-space top-left
            tt.position.y = worldY - camY;
            tt.scale = { 1.0f, 1.0f };
            tt.rotation = 0.0f;
            tt.tileIndex = tileIndex;

            // Optional: only keep tiles that actually intersect the screen rect.
            // Comment this block out if you WANT the guard tiles offscreen.
            {
                const float x2 = tt.position.x + tw;
                const float y2 = tt.position.y + th;
                if (x2 < 0.0f || y2 < 0.0f || tt.position.x > windowSize.w || tt.position.y > windowSize.h) {
                    // skip fully off-screen tiles
                    continue;
                }
            }


            vMap[tv] = static_cast<int>(outVisible.size()); // tile -> transform index
            outVisible.push_back(tt);
        }
    }

    */

}


// Ensure we call the event handler from update so layer can react to events
void AvatarQuestLayer::update(float deltaTime) {
    Game::GameEvents& events = Game::getGameEvents();  
//	Renderer::updateMap(deltaTime, _playerCamera->playerWorldPosition, _mapIndex);
    handleEvents(deltaTime, events);
    Vector2 oldPos = _playerCamera ? _playerCamera->playerWorldPosition : Vector2{};
    TileVector lastTilePos = _playerCamera ? _playerCamera->playerTilePosition : TileVector{};
    auto lastDirection = _playerCamera ? _playerCamera->currentMovement : PlayerMovement::Move_None;  
	auto windowSize = Window::getWindowSize();

    if (_playerCamera && movePlayer(_playerCamera->currentMovement, deltaTime)) {
        _playerCamera->currentMovement = PlayerMovement::Move_None;
		events.reset();
  //      Vector2 newTilLoc =  tileLocFromWorldPos(_playerWorldPosition);
     //   Vector2 newWorldLoc = WorldPosFromTileLoc(newTilLoc);
   /*     TileVector startPos = tileLocFromWorldPos({(float)windowSize.w / 2.0f, (float)windowSize.h / 2.0f});

        Vector2 WstartPos = WorldPosFromTileLoc(startPos);

     //   Vector2 tileSCreen = WorldPosFromTileLoc(;
		auto ir = _visableTileMap.find(startPos);
		int tileVisibleIndex = -1;
		if (ir != _visableTileMap.end()) {
            tileVisibleIndex = ir->second;
        }
        bool revert = false;
  
        if (tileVisibleIndex == -1) {
            revert = true;
        } else  {
        //    auto& 
            //	_playerTilePosition = tileLocFromWorldPos(_playerWorldPosition);

                // check if we are on a valid tile
            int tileIndex = _visableTiles[tileVisibleIndex].tileIndex;
            Ref<Renderer::Tile>& tile = _tiles[tileIndex];
   //         Renderer::getMapIndex(_mapIndex, tileSCreen.toIntX(), tileSCreen.toIntY(), tileIndex);
            if (tileIndex < 0) {
                _playerWorldPosition = oldPos; // revert to old pos
                _playerTilePosition = tileLocFromWorldPos(_playerWorldPosition);
            }

        }

        if (revert) {
            _playerWorldPosition = oldPos; // revert to old pos
            _playerTilePosition = tileLocFromWorldPos(_playerWorldPosition);
        }
        */
    }

	

    buildVisibleTilesRect(_playerCamera->playerViewport,
       _tileSize,
        _playerCamera->playerTilePosition,
        _mapIndex,
		_visableTiles,_visableTileMap);
}

Ref<Game::UILayer> CreateAvatarQuestLayer() {
    return CreateRef<AvatarQuestLayer>();
}

}
