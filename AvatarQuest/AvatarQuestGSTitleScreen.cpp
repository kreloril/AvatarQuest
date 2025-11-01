#include "AvatarQuestGSTitleScreen.h"
#include "AvatarQuest/Fonts.h"
#include "UIControls.h"

using namespace AvatarQuest;

GSTitleScreen::~GSTitleScreen() { }

void GSTitleScreen::onEnter() {
    // Title text is hidden; make the prompt large so it's readable
    _titleFont = nullptr;
    _promptFont = Fonts::menu();
}

AQStateId GSTitleScreen::handleEvents(float /*delta*/, Game::GameEvents& events) {
    using Game::GameEvents;
    if (events.isEventType(GameEvents::EventType::KeyPress) ||
        (events.isEventType(GameEvents::EventType::MouseButtonPress) && events.isMouseButton(GameEvents::MouseButton::Left))) {
        events.reset();
        return AQStateId::MainMenu; // any key proceeds to main menu
    }
    return AQStateId::None;
}

void GSTitleScreen::render(float /*delta*/) {
    SDL_Rect ws = Window::getWindowSize();
    const float cx = (float)ws.x + (float)ws.w * 0.5f;
    const float titleYAnchor = (float)ws.y + (float)ws.h * 0.40f; // centered higher
    if (_promptFont) {
        const char* hint = "Press any key";
        SDL_FRect hsize = Text::measure(_promptFont, hint);
        const float hintY = (float)ws.y + (float)ws.h * 0.75f;
        Text::draw(_promptFont, hint, cx - hsize.w * 0.5f, hintY - hsize.h * 0.5f, SDL_Color{200,200,200,255});
    }
}
