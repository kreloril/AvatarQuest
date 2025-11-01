#include "UIControls.h"
#include "TileBank.h"

namespace UI {

#ifdef AVATARQUEST_ENABLE_AUDIO
#include "Sound.h"
static Sound::Sfx* AQ_GetUiBeep() {
    static Sound::Sfx* s_beep = nullptr;
    if (!s_beep) {
        s_beep = Sound::loadSfx("assets/wav/menu-beep.wav");
        if (!s_beep) s_beep = Sound::loadSfx("asssets/wav/menu-beep.wav");
    }
    return s_beep;
}
static inline void AQ_PlayBeep(int vol = 96) {
    if (auto* s = AQ_GetUiBeep()) { Sound::playSfx(s, 0, -1, vol); }
}
#endif

void UIWindow::render() const {
    // Background and border
    Renderer::drawFilledRect(_x, _y, _w, _h, _bg);
    Renderer::drawRect(_x, _y, _w, _h, _border);
    // Title
    if (_titleFont && !_title.empty()) {
        SDL_FRect m = Text::measure(_titleFont, _title.c_str());
        const float tx = _x + 8.0f; // small left padding for title
        const float ty = _y + 4.0f;
        Text::draw(_titleFont, _title.c_str(), tx, ty, SDL_Color{255,255,255,255});
    }
}

static inline bool isAlphaNumKey(int keyCode) {
    return (keyCode >= SDLK_0 && keyCode <= SDLK_9) || (keyCode >= SDLK_A && keyCode <= SDLK_Z);
}

char UITextInput::keyToChar(int keyCode) {
    if (keyCode == SDLK_SPACE) return ' ';
    if (keyCode >= SDLK_0 && keyCode <= SDLK_9) {
        return char('0' + (keyCode - SDLK_0));
    }
    if (keyCode >= SDLK_A && keyCode <= SDLK_Z) {
        // Respect Shift and CapsLock for letter case
        SDL_Keymod mods = SDL_GetModState();
        bool shift = (mods & SDL_KMOD_SHIFT) != 0;
        bool caps = (mods & SDL_KMOD_CAPS) != 0;
        bool upper = shift ^ caps; // uppercase if exactly one of Shift/Caps is active
        return upper ? char('A' + (keyCode - SDLK_A)) : char('a' + (keyCode - SDLK_A));
    }
    return 0;
}

void UITextInput::handleEvents(Game::GameEvents& events) {
    using Game::GameEvents;
    if (events.isEventType(GameEvents::EventType::MouseButtonPress) && events.isMouseButton(GameEvents::MouseButton::Left)) {
        const float mx = (float)events.mouseButtonEvent.x;
        const float my = (float)events.mouseButtonEvent.y;
        const bool inside = pointInRect(mx, my, rect());
        _focused = inside;
        if (inside) {
            events.reset();
            return;
        }
    }

    if (!_focused || !_font) return;

    if (events.isEventType(GameEvents::EventType::KeyPress)) {
        const int key = events.keyEvent.keyCode;
        switch (key) {
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            if (_onSubmit) _onSubmit(_text);
            events.reset();
            return;
        case SDLK_ESCAPE:
            _focused = false; events.reset(); return;
        case SDLK_BACKSPACE:
            if (_caret > 0 && !_text.empty()) {
                _text.erase(_text.begin() + (_caret - 1));
                --_caret;
            }
            events.reset();
            return;
        case SDLK_LEFT:
            if (_caret > 0) --_caret;
            events.reset();
            return;
        case SDLK_RIGHT:
            if (_caret < (int)_text.size()) ++_caret;
            events.reset();
            return;
        default:
            break;
        }

        if ((int)_text.size() < _maxChars) {
            char ch = keyToChar(key);
            if (ch != 0) {
                _text.insert(_text.begin() + _caret, ch);
                ++_caret;
                events.reset();
                return;
            }
        }
    }
}

void UITextInput::update(float delta) {
    if (_focused) {
        _blink += delta;
        if (_blink >= 0.5f) { _blink = 0.0f; _caretVisible = !_caretVisible; }
    } else {
        _caretVisible = false;
    }
}

void UITextInput::render() const {
    // Box
    Renderer::drawFilledRect(_x, _y, _w, _h, _bg);
    Renderer::drawRect(_x, _y, _w, _h, _border);

    const float padX = 6.0f;
    const float padY = 4.0f;
    const float tx = _x + padX;
    const float ty = _y + padY;
    if (_font) {
        if (_text.empty() && !_focused && !_placeholder.empty()) {
            Text::draw(_font, _placeholder.c_str(), tx, ty, _placeholderColor);
        } else {
            Text::draw(_font, _text.c_str(), tx, ty, _textColor);
        }
        // Caret
        if (_focused && _caretVisible) {
            // Measure substring up to caret
            String sub = _text.substr(0, std::min(_caret, (int)_text.size()));
            SDL_FRect m = Text::measure(_font, sub.c_str());
            const float cx = tx + m.w;
            const float ch = Text::measure(_font, "M").h; // approx height
            Renderer::drawFilledRect(cx, ty, 1.0f, ch, Renderer::Color{255,255,255,255});
        }
    }
}

void UIButton::handleEvents(Game::GameEvents& events) {
    using Game::GameEvents;
    if (!_enabled) {
        // Swallow focus/press if disabled but still allow hover state to clear
        if (events.isEventType(GameEvents::EventType::MouseMove)) {
            _hovered = false; // don't show hover when disabled
        }
        return;
    }
    if (events.isEventType(GameEvents::EventType::MouseMove)) {
        const float mx = (float)events.mouseMoveEvent.x;
        const float my = (float)events.mouseMoveEvent.y;
        bool was = _hovered;
        _hovered = pointInRect(mx, my, rect());
#ifdef AVATARQUEST_ENABLE_AUDIO
        if (_hovered && !was) { AQ_PlayBeep(72); }
#endif
        return;
    }
    if (events.isEventType(GameEvents::EventType::MouseButtonPress) && events.isMouseButton(GameEvents::MouseButton::Left)) {
        const float mx = (float)events.mouseButtonEvent.x;
        const float my = (float)events.mouseButtonEvent.y;
        if (pointInRect(mx, my, rect())) {
            _focused = true;
            _pressedState = true;
            events.reset();
            return;
        }
    }
    if (events.isEventType(GameEvents::EventType::MouseButtonRelease) && events.isMouseButton(GameEvents::MouseButton::Left)) {
        const float mx = (float)events.mouseButtonEvent.x;
        const float my = (float)events.mouseButtonEvent.y;
        bool inside = pointInRect(mx, my, rect());
        if (_pressedState) {
            _pressedState = false;
            if (inside && _onClick) {
#ifdef AVATARQUEST_ENABLE_AUDIO
                AQ_PlayBeep(96);
#endif
                _onClick();
            }
            events.reset();
            return;
        }
    }
    if (events.isEventType(GameEvents::EventType::KeyPress) && _focused) {
        if (events.keyEvent.keyCode == SDLK_RETURN || events.keyEvent.keyCode == SDLK_KP_ENTER || events.keyEvent.keyCode == SDLK_SPACE) {
            if (_onClick) {
#ifdef AVATARQUEST_ENABLE_AUDIO
                AQ_PlayBeep(96);
#endif
                _onClick();
            }
            events.reset();
            return;
        }
        if (events.keyEvent.keyCode == SDLK_ESCAPE) { _focused = false; events.reset(); return; }
    }
}

void UIButton::render() const {
    Renderer::Color bg = _normal;
    if (!_enabled) {
        // Dim when disabled
        bg.a = (Uint8)std::min<int>(bg.a, 120);
    } else if (_pressedState) {
        bg = _pressed;
    } else if (_hovered) {
        bg = _hover;
    }
    Renderer::drawFilledRect(_x, _y, _w, _h, bg);
    Renderer::drawRect(_x, _y, _w, _h, _border);
    if (_font && !_text.empty()) {
        SDL_FRect m = Text::measure(_font, _text.c_str());
        float tx = _x + (_w - m.w) * 0.5f;
        float ty = _y + (_h - m.h) * 0.5f;
        SDL_Color tc = _textColor;
        if (!_enabled) { tc.a = (Uint8)std::min<int>(tc.a, 140); }
        Text::draw(_font, _text.c_str(), tx, ty, tc);
    }
}

// ---- UICheckbox ----

void UICheckbox::toggle() {
    _checked = !_checked;
    if (_onChanged) _onChanged(_checked);
}

void UICheckbox::handleEvents(Game::GameEvents& events) {
    using Game::GameEvents;
    if (events.isEventType(GameEvents::EventType::MouseMove)) {
        const float mx = (float)events.mouseMoveEvent.x;
        const float my = (float)events.mouseMoveEvent.y;
        _hovered = pointInRect(mx, my, rect());
        return;
    }
    if (events.isEventType(GameEvents::EventType::MouseButtonPress) && events.isMouseButton(GameEvents::MouseButton::Left)) {
        const float mx = (float)events.mouseButtonEvent.x;
        const float my = (float)events.mouseButtonEvent.y;
        if (pointInRect(mx, my, rect())) {
            _focused = true;
            _pressed = true;
            events.reset();
            return;
        }
    }
    if (events.isEventType(GameEvents::EventType::MouseButtonRelease) && events.isMouseButton(GameEvents::MouseButton::Left)) {
        const float mx = (float)events.mouseButtonEvent.x;
        const float my = (float)events.mouseButtonEvent.y;
        bool inside = pointInRect(mx, my, rect());
        if (_pressed) {
            _pressed = false;
            if (inside) { toggle(); }
            events.reset();
            return;
        }
    }
    if (events.isEventType(GameEvents::EventType::KeyPress) && _focused) {
        const int key = events.keyEvent.keyCode;
        if (key == SDLK_SPACE || key == SDLK_RETURN || key == SDLK_KP_ENTER) {
            toggle();
            events.reset();
            return;
        }
        if (key == SDLK_ESCAPE) { _focused = false; events.reset(); return; }
    }
}

void UICheckbox::render() const {
    // Box at left, label to the right
    const float boxSize = std::min(_h, 18.0f);
    const float by = _y + (_h - boxSize) * 0.5f;
    Renderer::drawFilledRect(_x, by, boxSize, boxSize, _boxBg);
    Renderer::drawRect(_x, by, boxSize, boxSize, _boxBorder);
    if (_checked) {
        // Draw a simple check mark
        const float pad = 3.0f;
        float x1 = _x + pad;
        float y1 = by + boxSize * 0.55f;
        float x2 = _x + boxSize * 0.45f;
        float y2 = by + boxSize - pad;
        float x3 = _x + boxSize - pad;
        float y3 = by + pad;
        Renderer::drawThickLine(x1, y1, x2, y2, 2.0f, _check);
        Renderer::drawThickLine(x2, y2, x3, y3, 2.0f, _check);
    }
    if (_font && !_text.empty()) {
        SDL_FRect m = Text::measure(_font, _text.c_str());
        float tx = _x + boxSize + 8.0f;
        float ty = _y + (_h - m.h) * 0.5f;
        Text::draw(_font, _text.c_str(), tx, ty, _textColor);
    }
}

// ---- UISlider ----

float UISlider::knobCenterX() const {
    const float pad = 6.0f;
    float tmin = _x + pad;
    float tmax = _x + _w - pad;
    float t = (_max > _min) ? (_value - _min) / (_max - _min) : 0.0f;
    t = clampf(t, 0.0f, 1.0f);
    return tmin + t * (tmax - tmin);
}

void UISlider::setValueFromMouse(float mx) {
    const float pad = 6.0f;
    float tmin = _x + pad;
    float tmax = _x + _w - pad;
    float t = (_w > 2*pad) ? (mx - tmin) / (tmax - tmin) : 0.0f;
    t = clampf(t, 0.0f, 1.0f);
    float v = _min + t * (_max - _min);
    if (_step > 0.0f) {
        float steps = std::round((v - _min) / _step);
        v = _min + steps * _step;
    }
    v = clampf(v, _min, _max);
    if (v != _value) {
        _value = v;
        if (_onChanged) _onChanged(_value);
    }
}

void UISlider::handleEvents(Game::GameEvents& events) {
    using Game::GameEvents;
    const float pad = 6.0f;
    SDL_FRect track{ _x + pad, _y + _h * 0.5f - 2.0f, _w - 2*pad, 4.0f };
    if (events.isEventType(GameEvents::EventType::MouseButtonPress) && events.isMouseButton(GameEvents::MouseButton::Left)) {
        const float mx = (float)events.mouseButtonEvent.x;
        const float my = (float)events.mouseButtonEvent.y;
        if (pointInRect(mx, my, rect()) || pointInRect(mx, my, track)) {
            _dragging = true;
            setValueFromMouse(mx);
            events.reset();
            return;
        }
    }
    if (events.isEventType(GameEvents::EventType::MouseMove) && _dragging) {
        const float mx = (float)events.mouseMoveEvent.x;
        setValueFromMouse(mx);
        return;
    }
    if (events.isEventType(GameEvents::EventType::MouseButtonRelease) && events.isMouseButton(GameEvents::MouseButton::Left)) {
        if (_dragging) {
            _dragging = false;
            if (_onCommit) _onCommit(_value);
            events.reset();
            return;
        }
    }
    if (events.isEventType(GameEvents::EventType::KeyPress)) {
        const int key = events.keyEvent.keyCode;
        float delta = (_step > 0.0f) ? _step : (_max - _min) * 0.01f;
        if (key == SDLK_LEFT) { setValue(_value - delta); if (_onChanged) _onChanged(_value); events.reset(); return; }
        if (key == SDLK_RIGHT) { setValue(_value + delta); if (_onChanged) _onChanged(_value); events.reset(); return; }
        if (key == SDLK_HOME) { setValue(_min); if (_onChanged) _onChanged(_value); events.reset(); return; }
        if (key == SDLK_END) { setValue(_max); if (_onChanged) _onChanged(_value); events.reset(); return; }
    }
}

void UISlider::render() const {
    const float pad = 6.0f;
    // Track
    Renderer::drawFilledRect(_x, _y, _w, _h, Renderer::Color{0,0,0,0}); // transparent hit area
    Renderer::drawFilledRect(_x + pad, _y + _h * 0.5f - 2.0f, _w - 2*pad, 4.0f, _trackBg);
    // Fill (from left to knob)
    float cx = knobCenterX();
    float tmin = _x + pad;
    float fillw = std::max(0.0f, cx - tmin);
    Renderer::drawFilledRect(tmin, _y + _h * 0.5f - 2.0f, fillw, 4.0f, _trackFill);
    // Knob
    const float kw = 10.0f;
    const float kh = _h - 4.0f;
    float kx = cx - kw * 0.5f;
    float ky = _y + (_h - kh) * 0.5f;
    Renderer::drawFilledRect(kx, ky, kw, kh, _knob);
    Renderer::drawRect(kx, ky, kw, kh, _border);
}

// ---- UIProgressBar ----

void UIProgressBar::render() const {
    Renderer::drawFilledRect(_x, _y, _w, _h, _bg);
    Renderer::drawRect(_x, _y, _w, _h, _border);
    float pw = std::max(0.0f, std::min(_w, _w * _progress));
    Renderer::drawFilledRect(_x, _y, pw, _h, _fill);
}

} // namespace UI
 
// --------------- UIImage implementation ---------------
namespace UI {

void UIImage::render() const {
    const float cx = _x + _w * 0.5f;
    const float cy = _y + _h * 0.5f;

    // Preferred TileMap-backed path when a Tile is provided
    if (_useTileMap && _tmTile) {
        if (_tmTile->tileRects.empty()) return;
        // Ensure active frame is within bounds; use _tileIndex to select frame if available
        int frameCount = (int)_tmTile->tileRects.size();
        int idx = std::clamp(_tileIndex, 0, frameCount - 1);
        const SDL_FRect& src = _tmTile->tileRects[idx];
        if (src.w <= 0.0f || src.h <= 0.0f || _w <= 0.0f || _h <= 0.0f) return;
        float sx = 1.0f, sy = 1.0f;
        switch (_mode) {
        case ScaleMode::None:
            sx = 1.0f; sy = 1.0f; break;
        case ScaleMode::Fit: {
            float s = std::min(_w / src.w, _h / src.h);
            sx = s; sy = s; break;
        }
        case ScaleMode::Stretch:
            sx = (_w / src.w); sy = (_h / src.h); break;
        }
        // Temporarily set activeFrame and tint to render via TileMap
        int prevFrame = _tmTile->activeFrame;
        _tmTile->activeFrame = idx;
        if (_tmTile->tint.size() < _tmTile->tileRects.size()) {
            _tmTile->tint.resize(_tmTile->tileRects.size(), Renderer::Color{255,255,255,255});
        }
        _tmTile->tint[_tmTile->activeFrame] = _tint;
        TileMap::TileTransform tf{};
        tf.position = Vector2{ cx, cy };
        tf.scale = Vector2{ sx, sy };
        tf.rotation = _rotation;
        TileMap::renderTile(tf, _tmTile);
        // Restore frame if needed
        _tmTile->activeFrame = prevFrame;
        return;
    }

    // From here down, we require a regular image
    if (!_img || !_img->texture) return;

    if (_useTiles && _tileW > 0 && _tileH > 0) {
        // Draw a specific tile from the atlas
        const float atlasW = _img->imageRect.w;
        const float atlasH = _img->imageRect.h;
        if (atlasW <= 0.0f || atlasH <= 0.0f || _w <= 0.0f || _h <= 0.0f) return;
        const int cols = (int)std::max(1.0f, std::floor(atlasW / (float)_tileW));
        const int rows = (int)std::max(1.0f, std::floor(atlasH / (float)_tileH));
        if (cols <= 0 || rows <= 0) return;
        int idx = std::max(0, _tileIndex % (cols * rows));
        int tx = idx % cols;
        int ty = idx / cols;
        SDL_FRect tileRect{ _img->imageRect.x + tx * (float)_tileW,
                            _img->imageRect.y + ty * (float)_tileH,
                            (float)_tileW,
                            (float)_tileH };
        float sx = 1.0f, sy = 1.0f;
        switch (_mode) {
        case ScaleMode::None:
            sx = 1.0f; sy = 1.0f; break;
        case ScaleMode::Fit: {
            float s = std::min(_w / tileRect.w, _h / tileRect.h);
            sx = s; sy = s; break;
        }
        case ScaleMode::Stretch:
            sx = (_w / tileRect.w); sy = (_h / tileRect.h); break;
        }
        Renderer::drawImageFromRect(Vector2{ cx, cy }, Vector2{ sx, sy }, _rotation, _img, tileRect, _tint);
        return;
    }

    // Default: draw full image region
    const SDL_FRect src = _img->imageRect;
    if (src.w <= 0.0f || src.h <= 0.0f || _w <= 0.0f || _h <= 0.0f) return;
    float sx = 1.0f, sy = 1.0f;
    switch (_mode) {
    case ScaleMode::None:
        sx = 1.0f; sy = 1.0f; break;
    case ScaleMode::Fit: {
        float s = std::min(_w / src.w, _h / src.h);
        sx = s; sy = s; break;
    }
    case ScaleMode::Stretch:
        sx = (_w / src.w); sy = (_h / src.h); break;
    }
    Renderer::drawImage(Vector2{ cx, cy }, Vector2{ sx, sy }, _rotation, _img, _tint);
}

} // namespace UI

// --------------- UIAnimatedTextBox implementation ---------------
namespace UI {

int UIAnimatedTextBox::totalWords() const {
    int n = 0;
    for (const auto& t : _tokens) if (!t.newline) ++n;
    return n;
}

void UIAnimatedTextBox::tokenize() {
    _tokens.clear();
    String cur;
    for (size_t i = 0; i < _text.size(); ++i) {
        char c = _text[i];
        if (c == '\n') {
            if (!cur.empty()) { _tokens.push_back(Token{cur, false}); cur.clear(); }
            _tokens.push_back(Token{"", true});
        } else if (c == ' ' || c == '\t') {
            if (!cur.empty()) { _tokens.push_back(Token{cur, false}); cur.clear(); }
            // collapse multiple spaces; actual spacing is reinserted during layout
        } else {
            cur.push_back(c);
        }
    }
    if (!cur.empty()) { _tokens.push_back(Token{cur, false}); }
}

void UIAnimatedTextBox::update(float delta) {
    if (!_font) return;
    int total = totalWords();
    if (_visibleWords >= total) return;
    float local = delta;
    if (_delayLeft > 0.0f) {
        float d = std::min(_delayLeft, local);
        _delayLeft -= d;
        local -= d;
        if (_delayLeft > 0.0f) return; // still waiting
    }
    _accum += local;
    while (_accum >= _wordInterval && _visibleWords < total) {
        _accum -= _wordInterval;
        ++_visibleWords;
    }
}

void UIAnimatedTextBox::render() const {
    // Box
    Renderer::drawFilledRect(_x, _y, _w, _h, _bg);
    Renderer::drawRect(_x, _y, _w, _h, _border);
    if (!_font || _tokens.empty()) return;

    const float availW = std::max(0.0f, _w - 2.0f * _padX);
    const float startX = _x + _padX;
    const float startY = _y + _padY;
    const float lineH = Text::measure(_font, "Mg").h;

    // Build wrapped lines from the first _visibleWords words, respecting explicit newlines
    Vector<String> lines; lines.reserve(8);
    String current;
    int wordsShown = 0;
    for (const auto& t : _tokens) {
        if (t.newline) {
            if (!current.empty()) { lines.push_back(current); current.clear(); }
            else { lines.push_back(""); }
            continue;
        }
        if (wordsShown >= _visibleWords) break;
        const String& w = t.word;
        if (current.empty()) {
            // Start of a new line
            current = w;
        } else {
            String candidate = current + " " + w;
            SDL_FRect m = Text::measure(_font, candidate.c_str());
            if (m.w <= availW) current = std::move(candidate);
            else { lines.push_back(current); current = w; }
        }
        ++wordsShown;
    }
    if (!current.empty()) lines.push_back(current);

    // Draw lines
    float y = startY;
    for (const auto& ln : lines) {
        if (!ln.empty()) {
            Text::draw(_font, ln.c_str(), startX, y, _textColor);
        }
        y += lineH + _lineGap;
        if (y > _y + _h - _padY) break; // stop if overflowing the box vertically
    }
}

} // namespace UI

// --------------- UIListBox implementation ---------------
namespace UI {

int UIListBox::visibleRows() const {
    if (!_font || _h <= 0.0f) return 0;
    const float lineH = Text::measure(_font, "Mg").h + 2.0f * _rowPadY;
    return (int)std::max(1.0f, std::floor(_h / lineH));
}

void UIListBox::clampScroll() {
    int vis = visibleRows();
    int maxStart = (int)std::max(0, (int)_items.size() - vis);
    if (_scroll < 0) _scroll = 0;
    if (_scroll > maxStart) _scroll = maxStart;
    if (_selected >= (int)_items.size()) _selected = (int)_items.size() - 1;
}

int UIListBox::rowAt(float mx, float my) const {
    if (!_font) return -1;
    if (mx < _x || my < _y || mx > _x + _w || my > _y + _h) return -1;
    const float lineH = Text::measure(_font, "Mg").h + 2.0f * _rowPadY;
    int row = (int)std::floor((my - _y) / lineH);
    int idx = _scroll + row;
    if (idx < 0 || idx >= (int)_items.size()) return -1;
    return idx;
}

void UIListBox::handleEvents(Game::GameEvents& events) {
    using Game::GameEvents;
    if (events.isEventType(GameEvents::EventType::MouseMove)) {
        _lastMx = (float)events.mouseMoveEvent.x;
        _lastMy = (float)events.mouseMoveEvent.y;
        // don't consume
    }
    if (events.isEventType(GameEvents::EventType::MouseScroll)) {
        // Scroll only when the cursor is over the listbox
        if (_lastMx >= _x && _lastMy >= _y && _lastMx <= _x + _w && _lastMy <= _y + _h) {
            int dy = events.mouseScrollEvent.deltaY;
            if (dy != 0) {
                _scroll -= (dy > 0 ? 1 : -1);
                clampScroll();
                events.reset();
                return;
            }
        }
    }
    if (events.isEventType(GameEvents::EventType::MouseButtonPress) && events.isMouseButton(GameEvents::MouseButton::Left)) {
        const float mx = (float)events.mouseButtonEvent.x;
        const float my = (float)events.mouseButtonEvent.y;
        int idx = rowAt(mx, my);
        if (idx >= 0) {
            if (_selected != idx) {
                _selected = idx;
                if (_onChanged) _onChanged(_selected);
            }
            events.reset();
            return;
        }
    }
    if (events.isEventType(GameEvents::EventType::KeyPress)) {
        const int key = events.keyEvent.keyCode;
        if (key == SDLK_UP) {
            if (_selected > 0) { _selected--; if (_selected < _scroll) _scroll = _selected; if (_onChanged) _onChanged(_selected); }
            events.reset();
            return;
        } else if (key == SDLK_DOWN) {
            if (_selected < (int)_items.size() - 1) {
                _selected++;
                int vis = visibleRows();
                if (_selected >= _scroll + vis) _scroll = _selected - vis + 1;
                if (_onChanged) _onChanged(_selected);
            }
            events.reset();
            return;
        } else if (key == SDLK_PAGEUP) {
            int vis = std::max(1, visibleRows());
            _selected = std::max(0, _selected - vis);
            _scroll = std::max(0, _scroll - vis);
            if (_onChanged) _onChanged(_selected);
            events.reset();
            return;
        } else if (key == SDLK_PAGEDOWN) {
            int vis = std::max(1, visibleRows());
            _selected = std::min((int)_items.size() - 1, _selected + vis);
            _scroll = std::min(std::max(0, (int)_items.size() - vis), _scroll + vis);
            if (_onChanged) _onChanged(_selected);
            events.reset();
            return;
        }
    }
}

void UIListBox::render() const {
    Renderer::drawFilledRect(_x, _y, _w, _h, _bg);
    Renderer::drawRect(_x, _y, _w, _h, _border);
    if (!_font || _items.empty()) return;
    const float lineH = Text::measure(_font, "Mg").h + 2.0f * _rowPadY;
    int vis = std::max(1, visibleRows());
    float y = _y;
    for (int r = 0; r < vis; ++r) {
        int idx = _scroll + r;
        if (idx >= (int)_items.size()) break;
        const bool sel = (idx == _selected);
        if (sel) {
            Renderer::drawFilledRect(_x+1, y+1, _w-2, lineH-2, _selBg);
        }
        const SDL_Color col = sel ? _textSel : _text;
        Text::draw(_font, _items[idx].c_str(), _x + 6.0f, y + _rowPadY, col);
        y += lineH;
    }
}

} // namespace UI

// --------------- UIComboBox implementation ---------------
namespace UI {

void UIComboBox::handleEvents(Game::GameEvents& events) {
    using Game::GameEvents;
    if (events.isEventType(GameEvents::EventType::MouseButtonPress) && events.isMouseButton(GameEvents::MouseButton::Left)) {
        const float mx = (float)events.mouseButtonEvent.x;
        const float my = (float)events.mouseButtonEvent.y;
        // Toggle dropdown if header clicked
        if (pointInRect(mx, my, rect())) {
            _open = !_open;
            events.reset();
            return;
        }
        // If open, check dropdown area clicks
        if (_open && _font) {
            const float lineH = Text::measure(_font, "Mg").h + 4.0f;
            int rows = std::min(_visibleRows, (int)_items.size());
            SDL_FRect drop{ _x, _y + _h, _w, lineH * rows };
            if (pointInRect(mx, my, drop)) {
                int row = (int)((my - drop.y) / lineH);
                int idx = row; // no scroll for now; list is short
                if (idx >= 0 && idx < (int)_items.size()) {
                    if (_selected != idx) { _selected = idx; if (_onChanged) _onChanged(_selected); }
                    _open = false;
                    events.reset();
                    return;
                }
            } else {
                // Clicking outside closes
                _open = false;
            }
        }
    }
    if (events.isEventType(GameEvents::EventType::MouseMove)) {
        if (_open && _font) {
            const float mx = (float)events.mouseMoveEvent.x;
            const float my = (float)events.mouseMoveEvent.y;
            const float lineH = Text::measure(_font, "Mg").h + 4.0f;
            int rows = std::min(_visibleRows, (int)_items.size());
            SDL_FRect drop{ _x, _y + _h, _w, lineH * rows };
            if (pointInRect(mx, my, drop)) {
                int row = (int)((my - drop.y) / lineH);
                _hoverRow = (row >= 0 && row < rows) ? row : -1;
            } else {
                _hoverRow = -1;
            }
        }
    }
    if (events.isEventType(GameEvents::EventType::KeyPress)) {
        const int key = events.keyEvent.keyCode;
        if (!_open) {
            if (key == SDLK_SPACE || key == SDLK_RETURN || key == SDLK_KP_ENTER) { _open = true; events.reset(); return; }
        } else {
            if (key == SDLK_ESCAPE) { _open = false; events.reset(); return; }
            if (key == SDLK_UP) { if (_selected > 0) { _selected--; if (_onChanged) _onChanged(_selected); } events.reset(); return; }
            if (key == SDLK_DOWN) { if (_selected < (int)_items.size()-1) { _selected++; if (_onChanged) _onChanged(_selected); } events.reset(); return; }
            if (key == SDLK_RETURN || key == SDLK_KP_ENTER) { _open = false; events.reset(); return; }
        }
    }
}

void UIComboBox::render() const {
    // Render header (current selection)
    Renderer::drawFilledRect(_x, _y, _w, _h, _bg);
    Renderer::drawRect(_x, _y, _w, _h, _border);
    if (_font && _selected >= 0 && _selected < (int)_items.size()) {
        Text::draw(_font, _items[_selected].c_str(), _x + 6.0f, _y + 4.0f, _text);
    }
    // Draw a simple down arrow indicator on the right
    float ax = _x + _w - 14.0f; float ay = _y + _h * 0.5f;
    Renderer::drawThickLine(ax-4, ay-2, ax+4, ay-2, 2.0f, _border);
    Renderer::drawThickLine(ax-4, ay-2, ax, ay+2, 2.0f, _border);
    Renderer::drawThickLine(ax+4, ay-2, ax, ay+2, 2.0f, _border);

    if (!_open || !_font || _items.empty()) return;
    const float lineH = Text::measure(_font, "Mg").h + 4.0f;
    int rows = std::min(_visibleRows, (int)_items.size());
    SDL_FRect drop{ _x, _y + _h, _w, lineH * rows };
    Renderer::drawFilledRect(drop.x, drop.y, drop.w, drop.h, _dropBg);
    Renderer::drawRect(drop.x, drop.y, drop.w, drop.h, _dropBorder);
    for (int i = 0; i < rows; ++i) {
        float ry = drop.y + i * lineH;
        bool hover = (_hoverRow == i);
        if (hover) Renderer::drawFilledRect(drop.x+1, ry+1, drop.w-2, lineH-2, _hoverBg);
        const String& label = _items[i];
        Text::draw(_font, label.c_str(), drop.x + 6.0f, ry + 2.0f, _text);
    }
}

SDL_FRect UIComboBox::hitRect() const {
    if (!_open || !_font || _items.empty()) return rect();
    const float lineH = Text::measure(_font, "Mg").h + 4.0f;
    int rows = std::min(_visibleRows, (int)_items.size());
    SDL_FRect r = rect();
    r.h += lineH * rows; // union header + dropdown region
    return r;
}

} // namespace UI

// --------------- UISegmentedControl implementation ---------------
namespace UI {

int UISegmentedControl::segmentAt(float mx, float my) const {
    if (_items.empty()) return -1;
    if (!pointInRect(mx, my, rect())) return -1;
    float segW = (_w > 0.0f) ? (_w / (float)_items.size()) : 0.0f;
    if (segW <= 0.0f) return -1;
    int idx = (int)std::floor((mx - _x) / segW);
    if (idx < 0 || idx >= (int)_items.size()) return -1;
    return idx;
}

void UISegmentedControl::handleEvents(Game::GameEvents& events) {
    using Game::GameEvents;
    if (events.isEventType(GameEvents::EventType::MouseMove)) {
        _hover = segmentAt((float)events.mouseMoveEvent.x, (float)events.mouseMoveEvent.y);
        return;
    }
    if (events.isEventType(GameEvents::EventType::MouseButtonPress) && events.isMouseButton(GameEvents::MouseButton::Left)) {
        int idx = segmentAt((float)events.mouseButtonEvent.x, (float)events.mouseButtonEvent.y);
        if (idx >= 0) {
            _pressed = idx;
            events.reset();
            return;
        }
    }
    if (events.isEventType(GameEvents::EventType::MouseButtonRelease) && events.isMouseButton(GameEvents::MouseButton::Left)) {
        int idx = segmentAt((float)events.mouseButtonEvent.x, (float)events.mouseButtonEvent.y);
        if (_pressed >= 0) {
            if (idx == _pressed && idx >= 0) {
                if (_selected != idx) {
                    _selected = idx;
                    if (_onChanged) _onChanged(_selected);
                }
            }
            _pressed = -1;
            events.reset();
            return;
        }
    }
    if (events.isEventType(GameEvents::EventType::KeyPress)) {
        const int key = events.keyEvent.keyCode;
        if (key == SDLK_LEFT) {
            if (_selected > 0) { _selected--; if (_onChanged) _onChanged(_selected); }
            events.reset();
            return;
        }
        if (key == SDLK_RIGHT) {
            if (_selected < (int)_items.size() - 1) { _selected++; if (_onChanged) _onChanged(_selected); }
            events.reset();
            return;
        }
        if (key == SDLK_HOME) { if (!_items.empty()) { _selected = 0; if (_onChanged) _onChanged(_selected); } events.reset(); return; }
        if (key == SDLK_END) { if (!_items.empty()) { _selected = (int)_items.size()-1; if (_onChanged) _onChanged(_selected); } events.reset(); return; }
    }
}

void UISegmentedControl::render() const {
    // Outer bg/border
    Renderer::drawFilledRect(_x, _y, _w, _h, _bg);
    Renderer::drawRect(_x, _y, _w, _h, _border);
    if (_items.empty()) return;
    float segW = (_w > 0.0f) ? (_w / (float)_items.size()) : 0.0f;
    const float segH = _h;
    for (int i = 0; i < (int)_items.size(); ++i) {
        float sx = _x + segW * i;
        Renderer::Color bg = _segBg;
        if (i == _selected) bg = _segBgSel;
        else if (i == _hover) bg = _segBgHover;
        Renderer::drawFilledRect(sx, _y, segW, segH, bg);
        // Segment divider (except first)
        if (i > 0) {
            Renderer::drawRect(sx, _y, 1.0f, segH, _border);
        }
        // Label
        if (_font) {
            SDL_Color col = (i == _selected) ? _textSel : _text;
            SDL_FRect m = Text::measure(_font, _items[i].c_str());
            float tx = sx + (segW - m.w) * 0.5f;
            float ty = _y + (segH - m.h) * 0.5f;
            Text::draw(_font, _items[i].c_str(), tx, ty, col);
        }
    }
}

} // namespace UI
