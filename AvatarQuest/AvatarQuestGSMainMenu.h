#pragma once

#include "Common.h"
#include "AvatarQuestGameState.h"
#include "AvatarQuestMenu.h"

namespace AvatarQuest {

class GSMainMenu : public IGameState {
public:
    GSMainMenu() = default;
    ~GSMainMenu() override = default;

    void onEnter() override;
    AQStateId handleEvents(float delta, Game::GameEvents& events) override;
    void render(float delta) override;
    const char* name() const override { return "MainMenu"; }

private:
    Text::Font* _titleFont = nullptr;
    Menu _menu;
};

}
