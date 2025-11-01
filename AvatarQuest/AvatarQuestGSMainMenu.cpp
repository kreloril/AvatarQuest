#include "AvatarQuestGSMainMenu.h"
#include "AvatarQuest/Fonts.h"
#include "AvatarQuestGSSettings.h"

namespace AvatarQuest {

void GSMainMenu::onEnter() {
    _menu.setFont(Fonts::menu());
    Vector<MenuItem> items{
        { "Continue Adventure", AQStateId::World },
        { "New Game",           AQStateId::CharCreation },
    { "Settings",           AQStateId::Settings },
        { "Credits",            AQStateId::None },
        { "Exit",               AQStateId::Quit }
    };
    _menu.setItems(items);
}

AQStateId GSMainMenu::handleEvents(float /*delta*/, Game::GameEvents& events) {
    SDL_Rect ws = Window::getWindowSize();
    const float cx = (float)ws.x + (float)ws.w * 0.5f;
    const float menuCenterY = (float)ws.y + (float)ws.h * 0.82f;
    AQStateId next = _menu.handleEventsAt(events, cx, menuCenterY);
    if (next == AQStateId::Settings) {
        // Ensure Settings returns to Main Menu when launched from here
        GSSettings::SetBackTarget(AQStateId::MainMenu);
    }
    return next;
}

void GSMainMenu::render(float /*delta*/) {
    SDL_Rect ws = Window::getWindowSize();
    const float cx = (float)ws.x + (float)ws.w * 0.5f;
    const float menuCenterY = (float)ws.y + (float)ws.h * 0.82f;
    _menu.render(cx, menuCenterY);
}

} // namespace AvatarQuest
