#include "AvatarQuestGSCombat.h"
#include "AvatarQuest/Fonts.h"

using namespace AvatarQuest;

GSCombat::~GSCombat() { }

void GSCombat::onEnter() {
    _font = Fonts::menu();
    _menu.setFont(_font);
    Vector<MenuItem> items{
        { "Attack (noop)", AQStateId::None },
        { "Defend (noop)", AQStateId::None },
        { "Run (back to World)", AQStateId::World }
    };
    _menu.setItems(items);
}

AQStateId GSCombat::handleEvents(float /*delta*/, Game::GameEvents& events) {
    // Delegate to menu; also allow 'C' to exit back to World quickly.
    if (events.isEventType(Game::GameEvents::EventType::KeyPress) && events.keyEvent.keyCode == SDLK_C) {
        events.reset();
        return AQStateId::World;
    }
    return _menu.handleEvents(events);
}

void GSCombat::update(float /*delta*/) {
}

void GSCombat::render(float /*delta*/) {
    SDL_Rect ws = Window::getWindowSize();
    const float cx = (float)ws.x + (float)ws.w * 0.5f;
    const float cy = (float)ws.y + (float)ws.h * 0.35f;
    if (_font) {
        SDL_FRect tsize = Text::measure(_font, "Combat (placeholder)");
        Text::draw(_font, "Combat (placeholder)", cx - tsize.w * 0.5f, cy - tsize.h, SDL_Color{255,200,200,255});
    }
    // Render menu slightly below the title
    _menu.render(cx, cy + 60.0f);
}
