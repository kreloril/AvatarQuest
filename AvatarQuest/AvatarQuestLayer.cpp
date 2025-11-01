#include "AvatarQuestLayer.h"
#include "AvatarQuestGSTitleScreen.h"
#include "AvatarQuestGSCharCreation.h"
#include "AvatarQuestGSWorld.h"
#include "AvatarQuestGSCombat.h"
#include "AvatarQuestGSMainMenu.h"
#include "AvatarQuestGSSettings.h"
#include "AvatarQuest/Fonts.h"
#ifdef AVATARQUEST_ENABLE_AUDIO
#include "Sound.h"
#endif

#include <vector>
#include <cstring>

namespace AvatarQuest {

AvatarQuestLayer::AvatarQuestLayer() {}
AvatarQuestLayer::~AvatarQuestLayer() { shutDown(); }

void AvatarQuestLayer::setState(AQStateId id)
{
    if (_state) { _state->onExit(); }
    _stateId = id;
    switch (id) {
    case AQStateId::Title:        _state = std::make_shared<GSTitleScreen>(); break;
    case AQStateId::MainMenu:     _state = std::make_shared<GSMainMenu>(); break;
    case AQStateId::CharCreation: _state = std::make_shared<GSCharCreation>(); break;
            case AQStateId::Settings:     _state = std::make_shared<GSSettings>(); break;
    case AQStateId::World:        _state = std::make_shared<GSWorld>(); break;
    case AQStateId::Combat:       _state = std::make_shared<GSCombat>(); break;
    default: _state.reset(); break;
    }
    if (_state) { _state->onEnter(); }

#ifdef AVATARQUEST_ENABLE_AUDIO
    // Decide if this state is part of the menu system
    auto isMenuState = [](AQStateId s) {
        switch (s) {
            case AQStateId::Title:
            case AQStateId::MainMenu:
            case AQStateId::CharCreation:
            case AQStateId::Settings:
                return true;
            default:
                return false;
        }
    };

    if (isMenuState(id)) {
        // Lazy-load theme if needed
        if (!_menuMusic) {
            _menuMusic = Sound::loadMusic("assets/mp3/AvatarQuestTheme.mp3");
        }
        if (_menuMusic && !_menuMusicPlaying) {
            // Loop indefinitely at current music volume with a short fade-in
            if (Sound::playMusicFadeIn(_menuMusic, -1, 96, 400)) {
                _menuMusicPlaying = true;
            }
        }
    } else {
        if (_menuMusicPlaying) {
            // Fade out quickly when leaving menus
            Sound::stopMusic(400);
            _menuMusicPlaying = false;
        }
    }
#endif
}



// State-local helpers and constants removed from layer after refactor

void AvatarQuestLayer::init()
{
	Renderer::setClearColor({ 0, 0, 0, 255 }); // Black clear color

    // Load shared background image (used on all screens except World)
    Renderer::loadImageFromFile("assets/backgrounds/titleScreen.png", _bgImage);

    // Start at Title
    setState(AQStateId::Title);
}

void AvatarQuestLayer::render(float deltaTime)
{
    // Draw background for all non-World states
    if (_stateId != AQStateId::World && _bgImage && _bgImage->texture) {
        SDL_Rect ws = Window::getWindowSize();
        const float cx = (float)ws.x + (float)ws.w * 0.5f;
        const float cy = (float)ws.y + (float)ws.h * 0.5f;
        const float iw = (_bgImage->imageRect.w > 0.0f ? _bgImage->imageRect.w : 1.0f);
        const float ih = (_bgImage->imageRect.h > 0.0f ? _bgImage->imageRect.h : 1.0f);
        // Stretch to exactly fill the window (no letterboxing)
        const float sx = (float)ws.w / iw;
        const float sy = (float)ws.h / ih;
        Renderer::drawImage(Vector2{ cx, cy }, Vector2{ sx, sy }, 0.0f, _bgImage, Renderer::Color{255,255,255,255});
    }
    if (_state) {
        _state->render(deltaTime);
    }
}

void AvatarQuestLayer::shutDown()
{
    // Shutdown shared resources
    if (_bgImage) { Renderer::releaseImage(_bgImage); }
    Fonts::shutdown();
#ifdef AVATARQUEST_ENABLE_AUDIO
    if (_menuMusicPlaying) { Sound::stopMusic(200); _menuMusicPlaying = false; }
    if (_menuMusic) { Sound::unloadMusic(_menuMusic); _menuMusic = nullptr; }
#endif
}

void AvatarQuestLayer::handleEvents(float /*delta*/, Game::GameEvents& events) {
    using Game::GameEvents;
    // Note: Escape no longer exits the app; use Title menu Exit instead.
    // Let the active state consume events and request transitions
    if (_state) {
        AQStateId next = _state->handleEvents(0.0f, events);
        if (next == AQStateId::Quit) {
            Game::endGameLoop();
            return;
        } else if (next != AQStateId::None) {
            setState(next);
        }
    }
}

void AvatarQuestLayer::update(float deltaTime) {
    Game::GameEvents& events = Game::getGameEvents();
    handleEvents(deltaTime, events);
    if (_state) {
        _state->update(deltaTime);
    }
}

Ref<Game::UILayer> CreateAvatarQuestLayer() {
    return CreateRef<AvatarQuestLayer>();
}

} // namespace AvatarQuest
