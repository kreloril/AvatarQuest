#include "AvatarQuestGSWorld.h"
#include "AvatarQuest/Fonts.h"
#include "AvatarQuestGSSettings.h"
#include "BinaryIO.h"

using namespace AvatarQuest;

void GSWorld::onEnter() {
    _window = Window::getWindowSize();
    // Initialize map and player
    const TileVector mapSize{128,128};
    const TileVector tileSize{128,128};
    _map.init(mapSize, tileSize);
    _map.buildGeneratedTerrain();

    TileVector startingTile{64,64};
    Ref<PlayerCamera> pc;
    createPlayer(_window, startingTile, pc);
    _playerCamera = pc;

    // Initialize pause menu
    _pauseFont = Fonts::menu();
    _pauseMenu.setFont(_pauseFont);
    _pauseWindow.setColors(Renderer::Color{30,30,40,220}, Renderer::Color{200,200,220,255});
    _pauseWindow.setTitle(_pauseFont, "Paused");
    Vector<MenuItem> pitems{
        { "Resume",     AQStateId::None },
        { "Save Game",  AQStateId::None },
        { "Load Game",  AQStateId::None },
        { "Settings",   AQStateId::Settings },
        { "Main Menu",  AQStateId::MainMenu },
        { "Quit",       AQStateId::Quit }
    };
    _pauseMenu.setItems(pitems);

    // Initialize quest window + animated text
    _questFont = Fonts::ui();
    _questWindow.setColors(Renderer::Color{20,20,28,200}, Renderer::Color{180,180,200,255});
    _questWindow.setTitle(_questFont, "Quest Log");
    // Initial size/position; will be kept in sync during update()
    const float qw = (float)_window.w * 0.82f;
    const float qh = std::max(200.0f, (float)_window.h * 0.28f);
    const float qx = (float)_window.x + ((float)_window.w - qw) * 0.5f;
    const float qy = (float)_window.y + (float)_window.h - qh - 24.0f;
    _questWindow.setPosition(qx, qy);
    _questWindow.setSize(qw, qh);

    _questText.setFont(_questFont);
    _questText.setWordInterval(12.0f);
    _questText.setStartDelay(0.8f);
    _questText.setPadding(10.0f, 8.0f);
    _questText.setColors(Renderer::Color{0,0,0,0}, Renderer::Color{0,0,0,0}, SDL_Color{235,235,235,255});
    _questText.setText(
        "Guild Notice: Adventurer required for urgent expedition. "
        "Travel east beyond the whispering pines to the ruined watchtower. "
        "Recover the moonlit signet from the captain's vault and return before dawn. "
        "Beware the hollow ones after sundown; they gather where the veil is thin. "
        "Optional: map any hidden paths beneath the tower's courtyard; report anomalies to the quartermaster. "
        "Reward: hazard pay, moon-silver shard, and one favor of the guild."
    );
}

AQStateId GSWorld::handleEvents(float /*delta*/, Game::GameEvents& events) {
    using Game::GameEvents;
    // Handle pause state input first
    if (_paused) {
        // Layout pause window based on menu metrics
        SDL_Rect ws = _window;
        const float cxScreen = (float)ws.x + (float)ws.w * 0.5f;
        const float cyScreen = (float)ws.y + (float)ws.h * 0.5f;
        float maxW = 0.0f; float lineH = 0.0f; const int N = _pauseMenu.itemCount();
        for (int i = 0; i < N; ++i) {
            const MenuItem* it = _pauseMenu.itemAt(i);
            if (!it) continue;
            SDL_FRect m = Text::measure(_pauseFont, it->label.c_str());
            maxW = std::max(maxW, m.w);
            lineH = std::max(lineH, m.h);
        }
    const float padX = 12.0f; const float padY = 6.0f; const float gap = 6.0f; const float contentMargin = 36.0f;
        float contentW = maxW + 2 * padX + contentMargin;
        float contentH = (float)N * lineH + ((float)N - 1.0f) * gap + 2 * padY + contentMargin;
    float winW = std::max(contentW, 640.0f);
    float winH = std::max(contentH + 8.0f, 440.0f);
        _pauseWindow.setSize(winW, winH);
        _pauseWindow.setPosition(cxScreen - winW * 0.5f, cyScreen - winH * 0.5f);
        SDL_FRect cr = _pauseWindow.contentRect();
        const float cx = cr.x + cr.w * 0.5f;
        const float cy = cr.y + cr.h * 0.5f;
        if (events.isEventType(GameEvents::EventType::KeyPress)) {
            switch (events.keyEvent.keyCode) {
            case SDLK_ESCAPE:
                _paused = false; events.reset(); return AQStateId::None;
            case SDLK_RETURN:
            case SDLK_KP_ENTER: {
                int sel = _pauseMenu.selected();
                const MenuItem* it = _pauseMenu.itemAt(sel);
                if (it) {
                    events.reset();
                    // Handle custom actions for Save/Load/Resume
                    if (it->label == "Resume") { _paused = false; return AQStateId::None; }
                    if (it->label == "Save Game") { saveGame(); return AQStateId::None; }
                    if (it->label == "Load Game") { loadGame(); _paused = false; return AQStateId::None; }
                    // Otherwise, follow next (MainMenu or Quit)
                    if (it->next == AQStateId::Settings) { GSSettings::SetBackTarget(AQStateId::World); }
                    return it->next;
                }
                break;
            }
            default: break;
            }
        }
        // First, delegate to menu (supports KB nav) and also mouse hit-testing within the pause window
        AQStateId next = _pauseMenu.handleEventsAt(events, cx, cy);
        if (next != AQStateId::None) {
            if (next == AQStateId::Settings) { GSSettings::SetBackTarget(AQStateId::World); }
            return next;
        }
        // If an item was activated (even if next==None), perform custom actions
        int act = _pauseMenu.lastActivatedIndex();
        if (act >= 0) {
            const MenuItem* it = _pauseMenu.itemAt(act);
            _pauseMenu.clearActivation();
            if (it) {
                if (it->label == "Resume") { _paused = false; return AQStateId::None; }
                if (it->label == "Save Game") { saveGame(); return AQStateId::None; }
                if (it->label == "Load Game") { loadGame(); _paused = false; return AQStateId::None; }
                if (it->next == AQStateId::Settings) { GSSettings::SetBackTarget(AQStateId::World); return AQStateId::Settings; }
                return it->next; // Main Menu or Quit
            }
        }

        // No transition
        return AQStateId::None;
    }
    // Click inside quest window reveals all text
    if (events.isEventType(GameEvents::EventType::MouseButtonPress) && events.isMouseButton(GameEvents::MouseButton::Left)) {
        SDL_FRect cr = _questWindow.contentRect();
        const float mx = (float)events.mouseButtonEvent.x;
        const float my = (float)events.mouseButtonEvent.y;
        if (mx >= cr.x && my >= cr.y && mx <= cr.x + cr.w && my <= cr.y + cr.h) {
            _questText.revealAll();
            events.reset();
            return AQStateId::None;
        }
    }

    if (events.isEventType(GameEvents::EventType::KeyRelease)) {
        events.reset();
        if (_playerCamera) _playerCamera->currentMovement = PlayerMovement::Move_None;
        return AQStateId::None;
    }

    if (events.isEventType(GameEvents::EventType::KeyPress)) {
        switch (events.keyEvent.keyCode) {
        case SDLK_ESCAPE:
        case SDLK_P:
            _paused = true; events.reset(); break;
        case SDLK_W:
            if (_playerCamera) _playerCamera->currentMovement = PlayerMovement::Move_Up;
            events.reset();
            break;
        case SDLK_S:
            if (_playerCamera) _playerCamera->currentMovement = PlayerMovement::Move_Down;
            events.reset();
            break;
        case SDLK_A:
            if (_playerCamera) _playerCamera->currentMovement = PlayerMovement::Move_Left;
            events.reset();
            break;
        case SDLK_D:
            if (_playerCamera) _playerCamera->currentMovement = PlayerMovement::Move_Right;
            events.reset();
            break;
        case SDLK_C: {
            // Placeholder: trigger combat state
            events.reset();
            return AQStateId::Combat;
        }
        default: break;
        }
    }
    return AQStateId::None;
}

bool GSWorld::movePlayer(PlayerMovement moveDir, float /*deltaTime*/) {
    if (moveDir == PlayerMovement::Move_None || !_playerCamera) return false;
    int newX = _playerCamera->playerTilePosition.x;
    int newY = _playerCamera->playerTilePosition.y;
    switch (moveDir) {
    case PlayerMovement::Move_Up:    newY -= 1; break;
    case PlayerMovement::Move_Down:  newY += 1; break;
    case PlayerMovement::Move_Left:  newX -= 1; break;
    case PlayerMovement::Move_Right: newX += 1; break;
    default: break;
    }
    _playerCamera->playerTilePosition = { newX, newY };
    _playerCamera->playerWorldPosition = _map.worldPosFromTileLoc(_playerCamera->playerTilePosition);
    return true;
}

void GSWorld::update(float delta) {
    (void)delta;
    if (!_paused && _playerCamera && movePlayer(_playerCamera->currentMovement, delta)) {
        _playerCamera->currentMovement = PlayerMovement::Move_None;
    }
    _window = Window::getWindowSize();
    // Keep quest window anchored to bottom center and fit to window
    {
        const float qw = (float)_window.w * 0.82f;
        const float qh = std::max(200.0f, (float)_window.h * 0.28f);
        const float qx = (float)_window.x + ((float)_window.w - qw) * 0.5f;
        const float qy = (float)_window.y + (float)_window.h - qh - 24.0f;
        _questWindow.setPosition(qx, qy);
        _questWindow.setSize(qw, qh);
        SDL_FRect cr = _questWindow.contentRect();
        _questText.setPosition(cr.x, cr.y);
        _questText.setSize(cr.w, cr.h);
    }
    if (!_paused) {
        _questText.update(delta);
    }
    if (_playerCamera) {
        SDL_FRect wf{ (float)_window.x, (float)_window.y, (float)_window.w, (float)_window.h };
        _map.updateVisible(wf, *_playerCamera);
    }
}

void GSWorld::render(float /*delta*/) {
    _map.render();
    if (_playerCamera) {
        const float w = (float)_map.tileSize().x;
        const float h = (float)_map.tileSize().y;
    Renderer::drawRect(_playerCamera->playerCenterScreen.x,
               _playerCamera->playerCenterScreen.y,
               w, h, Renderer::Color{0,255,0,255});
    }
    // Quest window and animated text
    _questWindow.render();
    _questText.render();
    if (_paused) {
        // Dim background
        SDL_Rect ws = _window;
        Renderer::drawFilledRect((float)ws.x, (float)ws.y, (float)ws.w, (float)ws.h, Renderer::Color{0,0,0,140});
        // Layout window same as in handleEvents and render inside it
        const float cxScreen = (float)ws.x + (float)ws.w * 0.5f;
        const float cyScreen = (float)ws.y + (float)ws.h * 0.5f;
        float maxW = 0.0f; float lineH = 0.0f; const int N = _pauseMenu.itemCount();
        for (int i = 0; i < N; ++i) {
            const MenuItem* it = _pauseMenu.itemAt(i);
            if (!it) continue;
            SDL_FRect m = Text::measure(_pauseFont, it->label.c_str());
            maxW = std::max(maxW, m.w);
            lineH = std::max(lineH, m.h);
        }
    const float padX = 12.0f; const float padY = 6.0f; const float gap = 6.0f; const float contentMargin = 36.0f;
        float contentW = maxW + 2 * padX + contentMargin;
        float contentH = (float)N * lineH + ((float)N - 1.0f) * gap + 2 * padY + contentMargin;
    float winW = std::max(contentW, 640.0f);
    float winH = std::max(contentH + 8.0f, 440.0f);
        _pauseWindow.setSize(winW, winH);
        _pauseWindow.setPosition(cxScreen - winW * 0.5f, cyScreen - winH * 0.5f);
        _pauseWindow.render();
        SDL_FRect cr = _pauseWindow.contentRect();
        const float cx = cr.x + cr.w * 0.5f;
        const float cy = cr.y + cr.h * 0.5f;
        _pauseMenu.render(cx, cy);
    }
}

// ---------------- persistence ----------------
static constexpr std::uint32_t kSaveMagic = 0x41565131; // 'AVQ1'
static constexpr std::uint32_t kSaveVersion = 1;
static const char* kSavePath = "savegame.bin";

bool GSWorld::saveGame() {
    if (!_playerCamera) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Save] No player camera"); return false; }
    Util::BinStream bs;
    bs.writeU32(kSaveMagic);
    bs.writeU32(kSaveVersion);
    bs.writeI32(_playerCamera->playerTilePosition.x);
    bs.writeI32(_playerCamera->playerTilePosition.y);
    const bool ok = bs.saveFile(kSavePath);
    SDL_Log(ok ? "[Save] Saved game to %s" : "[Save] Failed to save game to %s", kSavePath);
    return ok;
}

bool GSWorld::loadGame() {
    Util::BinStream bs;
    if (!bs.loadFile(kSavePath)) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Load] Could not open %s", kSavePath); return false; }
    std::uint32_t magic = 0, ver = 0;
    if (!bs.readU32(magic) || magic != kSaveMagic) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Load] Bad magic in %s", kSavePath); return false; }
    if (!bs.readU32(ver) || ver != kSaveVersion) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Load] Bad version in %s", kSavePath); return false; }
    std::int32_t tx = 0, ty = 0;
    if (!bs.readI32(tx) || !bs.readI32(ty)) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Load] Truncated file %s", kSavePath); return false; }
    if (!_playerCamera) { SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Load] No player camera"); return false; }
    _playerCamera->playerTilePosition = { (int)tx, (int)ty };
    _playerCamera->playerWorldPosition = _map.worldPosFromTileLoc(_playerCamera->playerTilePosition);
    SDL_Log("[Load] Loaded player tile (%d,%d) from %s", (int)tx, (int)ty, kSavePath);
    return true;
}
