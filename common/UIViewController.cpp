#include "UIViewController.h"

namespace UI {

UIControl* UIViewController::topMostAt(float mx, float my) {
    UIControl* best = nullptr;
    int bestZ = std::numeric_limits<int>::min();
    for (const auto& it : _items) {
        if (!it.control) continue;
        SDL_FRect r = it.control->hitRect();
        if (pointInRect(mx, my, r)) {
            if (it.z >= bestZ) { bestZ = it.z; best = it.control; }
        }
    }
    return best;
}

void UIViewController::handleEvents(Game::GameEvents& events) {
    using Game::GameEvents;
    if (events.type == GameEvents::EventType::None) return;

    // Mouse move: broadcast so hover states update everywhere
    if (events.isEventType(GameEvents::EventType::MouseMove)) {
        for (auto& it : _items) {
            if (it.control) it.control->handleEvents(events);
            if (events.type == GameEvents::EventType::None) return; // consumed
        }
        return;
    }

    // Mouse button press/release
    if (events.isEventType(GameEvents::EventType::MouseButtonPress) || events.isEventType(GameEvents::EventType::MouseButtonRelease)) {
        const float mx = (float)(events.isEventType(GameEvents::EventType::MouseButtonPress) ? events.mouseButtonEvent.x : events.mouseButtonEvent.x);
        const float my = (float)(events.isEventType(GameEvents::EventType::MouseButtonPress) ? events.mouseButtonEvent.y : events.mouseButtonEvent.y);
        if (events.isMouseButton(GameEvents::MouseButton::Left)) {
            if (events.isEventType(GameEvents::EventType::MouseButtonPress)) {
                UIControl* target = topMostAt(mx, my);
                if (target) {
                    _captured = target;
                    // Focus the pressed control if focusable
                    for (const auto& it : _items) { if (it.control == target && it.focusable) { _focused = target; break; } }
                    target->handleEvents(events);
                    return;
                }
            } else {
                // Release goes to captured if any, otherwise top-most under cursor
                UIControl* target = _captured ? _captured : topMostAt(mx, my);
                if (target) {
                    target->handleEvents(events);
                    _captured = nullptr;
                    return;
                }
                _captured = nullptr;
            }
        }
        // If not left button, just pass to focused
        if (_focused) { _focused->handleEvents(events); }
        return;
    }

    // Keyboard: route to focused control if any
    if (events.isEventType(GameEvents::EventType::KeyPress) || events.isEventType(GameEvents::EventType::KeyRelease)) {
        if (_focused) {
            _focused->handleEvents(events);
            return;
        }
    }

    // Fallback: pass to all until consumed
    for (auto& it : _items) {
        if (it.control) it.control->handleEvents(events);
        if (events.type == GameEvents::EventType::None) return;
    }
}

}
