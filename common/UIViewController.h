#pragma once

#include "Common.h"
#include "Game.h"
#include "UIControls.h"

namespace UI {

// Lightweight controller to manage focus and input routing for a set of UIControls
// - Maintains a list of active controls per frame (non-owning)
// - Routes mouse button events to the top-most control under the cursor (captures until release)
// - Broadcasts mouse move to all controls so hover states update correctly
// - Routes key events to the currently focused control
// States can rebuild the active list each frame based on visibility/step.
class UIViewController {
public:
    UIViewController() = default;
    ~UIViewController() = default;

    void setWindow(UIWindow* w) { _window = w; }

    struct Item { UIControl* control = nullptr; bool focusable = true; int z = 0; };
    void clear() { _items.clear(); _captured = nullptr; /* keep focus across frames */ }
    void add(UIControl* c, bool focusable = true, int z = 0) { if (c) _items.push_back(Item{c, focusable, z}); }

    void setFocus(UIControl* c) {
        if (c == _focused) return;
        _focused = c;
    }

    UIControl* focused() const { return _focused; }

    // Event routing
    void handleEvents(Game::GameEvents& events);

    // Optional pass-through
    void update(float delta) {
        for (auto& it : _items) { if (it.control) it.control->update(delta); }
    }

private:
    // Helpers
    static bool pointInRect(float px, float py, const SDL_FRect& r) { return px >= r.x && py >= r.y && px <= r.x + r.w && py <= r.y + r.h; }
    UIControl* topMostAt(float mx, float my);

    Vector<Item> _items;
    UIWindow* _window = nullptr; // non-owning
    UIControl* _focused = nullptr; // non-owning
    UIControl* _captured = nullptr; // mouse capture target
};

}
