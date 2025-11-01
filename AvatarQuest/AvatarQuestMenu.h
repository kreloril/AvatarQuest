#pragma once

#include "Common.h"
#include "Game.h"
#include "AvatarQuestGameState.h"

namespace AvatarQuest {

struct MenuItem {
    String label;
    AQStateId next = AQStateId::None;
};

class Menu {
public:
    void setFont(Text::Font* font) { _font = font; }
    void setItems(const Vector<MenuItem>& items) { _items = items; _selected = 0; }

    // Returns next state if an item is activated; otherwise AQStateId::None.
    AQStateId handleEvents(Game::GameEvents& events);

    // Mouse-aware handling when you know the render center. Pass the same (cx, cy)
    // you'll use for render() so hit-testing matches visuals.
    AQStateId handleEventsAt(Game::GameEvents& events, float cx, float cy);

    // Renders the menu centered at (cx, cy). Spacing is derived from font metrics.
    void render(float cx, float cy) const;

    // Accessors for selection and items (useful for custom activation handling)
    int selected() const { return _selected; }
    int itemCount() const { return (int)_items.size(); }
    const MenuItem* itemAt(int index) const {
        if (index < 0 || index >= (int)_items.size()) return nullptr;
        return &_items[index];
    }

    // Activation info: index of the last item activated via Enter or mouse click
    int lastActivatedIndex() const { return _lastActivatedIndex; }
    void clearActivation() { _lastActivatedIndex = -1; }

private:
    Text::Font* _font = nullptr;
    Vector<MenuItem> _items;
    int _selected = 0;
    int _pressedIndex = -1;
    int _lastActivatedIndex = -1;
};

}
