#pragma once

#include "Common.h"
#include "AvatarQuestGameState.h"
#include "AvatarQuestMenu.h"

namespace AvatarQuest {

class GSCombat : public IGameState {
public:
    GSCombat() = default;
    ~GSCombat() override;

    void onEnter() override;
    AQStateId handleEvents(float delta, Game::GameEvents& events) override;
    void update(float delta) override;
    void render(float delta) override;
    const char* name() const override { return "Combat"; }

private:
    Text::Font* _font = nullptr;
    Menu _menu;
};

}
