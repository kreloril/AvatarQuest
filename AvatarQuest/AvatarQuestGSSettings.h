#pragma once

#include "Common.h"
#include "AvatarQuestGameState.h"
#include "UIControls.h"

namespace AvatarQuest {

class GSSettings : public IGameState {
public:
    GSSettings() = default;
    ~GSSettings() override = default;

    void onEnter() override;
    AQStateId handleEvents(float delta, Game::GameEvents& events) override;
    void update(float delta) override;
    void render(float delta) override;
    const char* name() const override { return "Settings"; }

    // Configure which state the Back/Esc action should return to
    static void SetBackTarget(AQStateId target);
    static AQStateId GetBackTarget();

private:
    Text::Font* _font = nullptr;
    UI::UIWindow _window;
    UI::UILabel _title;

    UI::UILabel _lblMaster;
    UI::UISlider _slMaster;

    UI::UILabel _lblMusic;
    UI::UISlider _slMusic;

    UI::UILabel _lblSfx;
    UI::UISlider _slSfx;

    UI::UIButton _btnBack;
    AvatarQuest::AQStateId _next = AvatarQuest::AQStateId::None;

    static AvatarQuest::AQStateId s_backTarget;
};

}
