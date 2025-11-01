#pragma once

#include "Common.h"
#include "AvatarQuestGameState.h"
#include "AvatarQuestMap.h"
#include "AvatarQuestPlayer.h"
#include "AvatarQuestMenu.h"
#include "UIControls.h"

namespace AvatarQuest {

class GSWorld : public IGameState {
public:
    GSWorld() = default;
    ~GSWorld() override = default;

    void onEnter() override;
    AQStateId handleEvents(float delta, Game::GameEvents& events) override;
    void update(float delta) override;
    void render(float delta) override;
    const char* name() const override { return "World"; }

private:
    bool movePlayer(PlayerMovement moveDir, float deltaTime);

    WorldMap _map;
    Ref<PlayerCamera> _playerCamera;
    SDL_Rect _window{};

    // Pause menu state
    bool _paused = false;
    Menu _pauseMenu;
    Text::Font* _pauseFont = nullptr;
    UI::UIWindow _pauseWindow;

    // Quest text panel (animated word-by-word with wrapping)
    UI::UIWindow _questWindow;
    UI::UIAnimatedTextBox _questText;
    Text::Font* _questFont = nullptr;

    // Simple persistence for player position
    bool saveGame();
    bool loadGame();
};

}
