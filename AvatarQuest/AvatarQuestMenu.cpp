#include "AvatarQuestMenu.h"

namespace AvatarQuest {

#ifdef AVATARQUEST_ENABLE_AUDIO
#include "Sound.h"
static Sound::Sfx* GetUiBeep() {
    static Sound::Sfx* s_beep = nullptr;
    if (!s_beep) {
        // Try canonical path first, then a common typo fallback
        s_beep = Sound::loadSfx("assets/wav/menu-beep.wav");
        if (!s_beep) s_beep = Sound::loadSfx("asssets/wav/menu-beep.wav");
    }
    return s_beep;
}
static inline void PlayBeep() {
    if (auto* s = GetUiBeep()) { Sound::playSfx(s, 0, -1, 96); }
}
#endif

AQStateId Menu::handleEvents(Game::GameEvents& events) {
    using Game::GameEvents;
    if (!_font || _items.empty()) return AQStateId::None;
    if (events.isEventType(GameEvents::EventType::KeyPress)) {
        switch (events.keyEvent.keyCode) {
    case SDLK_UP:
    case SDLK_W:
            _selected = (_selected + (int)_items.size() - 1) % (int)_items.size();
#ifdef AVATARQUEST_ENABLE_AUDIO
            PlayBeep();
#endif
            events.reset();
            break;
    case SDLK_DOWN:
    case SDLK_S:
            _selected = (_selected + 1) % (int)_items.size();
#ifdef AVATARQUEST_ENABLE_AUDIO
            PlayBeep();
#endif
            events.reset();
            break;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            if (_selected >= 0 && _selected < (int)_items.size()) {
                #ifdef AVATARQUEST_ENABLE_AUDIO
                PlayBeep();
                #endif
                _lastActivatedIndex = _selected;
                AQStateId next = _items[_selected].next;
                events.reset();
                return next;
            }
            break;
        default: break;
        }
    }
    return AQStateId::None;
}

AQStateId Menu::handleEventsAt(Game::GameEvents& events, float cx, float cy) {
    using Game::GameEvents;
    if (!_font || _items.empty()) return AQStateId::None;

    // Keyboard support (reuse existing)
    AQStateId kb = handleEvents(events);
    if (kb != AQStateId::None) return kb;

    // Mouse handling
    // Recompute layout like render()
    float maxW = 0.0f; float lineH = 0.0f; Vector<float> widths; widths.reserve(_items.size());
    for (const auto& it : _items) {
        SDL_FRect m = Text::measure(_font, it.label.c_str());
        widths.push_back(m.w);
        maxW = std::max(maxW, m.w);
        lineH = std::max(lineH, m.h);
    }
    const float padX = 12.0f; const float padY = 6.0f; const float gap = 6.0f;
    float x0 = cx - (maxW + 2 * padX) * 0.5f;
    float y = cy - ((float)_items.size() * lineH + ((float)_items.size() - 1.0f) * gap + 2 * padY) * 0.5f + padY;

    auto indexAt = [&](float mx, float my) -> int {
        float iy = y;
        for (int i = 0; i < (int)_items.size(); ++i) {
            SDL_FRect r{ x0, iy - 2.0f, maxW + 2 * padX, lineH + 4.0f };
            if (mx >= r.x && my >= r.y && mx <= r.x + r.w && my <= r.y + r.h) return i;
            iy += lineH + gap;
        }
        return -1;
    };

    if (events.isEventType(GameEvents::EventType::MouseMove)) {
        const float mx = (float)events.mouseMoveEvent.x;
        const float my = (float)events.mouseMoveEvent.y;
        int idx = indexAt(mx, my);
        if (idx >= 0 && idx != _selected) {
            _selected = idx;
            #ifdef AVATARQUEST_ENABLE_AUDIO
            PlayBeep();
            #endif
        }
        return AQStateId::None;
    }
    if (events.isEventType(GameEvents::EventType::MouseButtonPress) && events.isMouseButton(GameEvents::MouseButton::Left)) {
        const float mx = (float)events.mouseButtonEvent.x;
        const float my = (float)events.mouseButtonEvent.y;
        int idx = indexAt(mx, my);
        if (idx >= 0) {
            _selected = idx;
            _pressedIndex = idx;
            events.reset();
            return AQStateId::None;
        }
    }
    if (events.isEventType(GameEvents::EventType::MouseButtonRelease) && events.isMouseButton(GameEvents::MouseButton::Left)) {
        const float mx = (float)events.mouseButtonEvent.x;
        const float my = (float)events.mouseButtonEvent.y;
        int idx = indexAt(mx, my);
        if (idx >= 0 && idx == _pressedIndex) {
            _selected = idx;
            _pressedIndex = -1;
            #ifdef AVATARQUEST_ENABLE_AUDIO
            PlayBeep();
            #endif
            _lastActivatedIndex = idx;
            events.reset();
            return _items[_selected].next;
        }
        _pressedIndex = -1;
    }
    return AQStateId::None;
}

void Menu::render(float cx, float cy) const {
    if (!_font || _items.empty()) return;
    // Measure max width and item height
    float maxW = 0.0f; float totalH = 0.0f; float lineH = 0.0f; Vector<float> widths; widths.reserve(_items.size());
    for (const auto& it : _items) {
        SDL_FRect m = Text::measure(_font, it.label.c_str());
        widths.push_back(m.w);
        maxW = std::max(maxW, m.w);
        lineH = std::max(lineH, m.h);
    }
    const float padX = 12.0f; const float padY = 6.0f; const float gap = 6.0f;
    totalH = _items.size() * lineH + (_items.size() - 1) * gap + 2 * padY;
    // Top-left start position
    float x0 = cx - (maxW + 2 * padX) * 0.5f;
    float y0 = cy - totalH * 0.5f;
    // Draw items
    float y = y0 + padY;
    for (int i = 0; i < (int)_items.size(); ++i) {
        const bool sel = (i == _selected);
        float iw = widths[i];
        float ix = cx - iw * 0.5f;
        if (sel) {
            Renderer::drawFilledRect(x0, y - 2.0f, maxW + 2 * padX, lineH + 4.0f, { 60,60,60,160 });
        }
        Text::draw(_font, _items[i].label.c_str(), ix, y, sel ? SDL_Color{255,255,160,255} : SDL_Color{220,220,220,255});
        y += lineH + gap;
    }
}

}
