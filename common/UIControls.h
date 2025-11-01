#pragma once

#include "Common.h"
#include "Game.h"
#include "Text.h"
#include "TileBank.h"
#include <functional>

namespace UI {

// Using TileMap types directly for tile-backed UI rendering

// Basic UI control base class
class UIControl {
public:
    virtual ~UIControl() = default;
    virtual void setPosition(float x, float y) { _x = x; _y = y; }
    virtual void setSize(float w, float h) { _w = w; _h = h; }
    virtual void handleEvents(Game::GameEvents& /*events*/) {}
    virtual void update(float /*delta*/) {}
    virtual void render() const = 0;
    SDL_FRect rect() const { return SDL_FRect{_x, _y, _w, _h}; }
    // Extended hit-test area (defaults to rect). Controls like UIComboBox override this when open.
    virtual SDL_FRect hitRect() const { return rect(); }
protected:
    float _x = 0, _y = 0, _w = 0, _h = 0;
};

// Simple label control using Text subsystem
class UILabel : public UIControl {
public:
    UILabel() = default;
    UILabel(Text::Font* font, String text, SDL_Color color = SDL_Color{255,255,255,255})
        : _font(font), _text(std::move(text)), _color(color) {}

    void setText(String text) { _text = std::move(text); }
    void setColor(SDL_Color color) { _color = color; }
    void setFont(Text::Font* font) { _font = font; }

    void render() const override {
        if (!_font || _text.empty()) return;
        Text::draw(_font, _text.c_str(), _x, _y, _color);
    }

    SDL_FRect measure() const {
        if (!_font || _text.empty()) return SDL_FRect{0,0,0,0};
        return Text::measure(_font, _text.c_str());
    }
private:
    Text::Font* _font = nullptr;
    String _text;
    SDL_Color _color{255,255,255,255};
};

// Simple window/panel with background and border, optional title
class UIWindow : public UIControl {
public:
    void setTitle(Text::Font* font, String title) { _titleFont = font; _title = std::move(title); }
    void setColors(Renderer::Color bg, Renderer::Color border) { _bg = bg; _border = border; }
    void setPadding(float px, float py) { _padX = px; _padY = py; }

    SDL_FRect contentRect() const {
        float titleH = 0.0f;
        if (_titleFont && !_title.empty()) {
            SDL_FRect m = Text::measure(_titleFont, _title.c_str());
            titleH = m.h + _padY;
        }
        return SDL_FRect{ _x + _padX, _y + _padY + titleH, _w - 2*_padX, _h - 2*_padY - titleH };
    }

    void render() const override;

private:
    Text::Font* _titleFont = nullptr;
    String _title;
    Renderer::Color _bg{20,20,20,200};
    Renderer::Color _border{200,200,200,255};
    float _padX = 8.0f, _padY = 6.0f;
};

// Basic single-line text input
class UITextInput : public UIControl {
public:
    void setFont(Text::Font* font) { _font = font; }
    void setText(String t) { _text = std::move(t); _caret = (int)_text.size(); }
    const String& text() const { return _text; }
    void setPlaceholder(String p) { _placeholder = std::move(p); }
    void setMaxChars(int n) { _maxChars = n; }
    void setColors(Renderer::Color bg, Renderer::Color border, SDL_Color textCol, SDL_Color placeholderCol) {
        _bg = bg; _border = border; _textColor = textCol; _placeholderColor = placeholderCol;
    }
    void setOnSubmit(std::function<void(const String&)> cb) { _onSubmit = std::move(cb); }

    void handleEvents(Game::GameEvents& events) override;
    void update(float delta) override;
    void render() const override;
    bool focused() const { return _focused; }
    void focus(bool f) { _focused = f; _blink = 0.0f; _caretVisible = true; }

private:
    static bool pointInRect(float px, float py, const SDL_FRect& r) {
        return px >= r.x && py >= r.y && px <= r.x + r.w && py <= r.y + r.h;
    }
    static char keyToChar(int keyCode);

    Text::Font* _font = nullptr;
    String _text;
    String _placeholder;
    int _maxChars = 128;
    int _caret = 0; // insertion point
    bool _focused = false;
    float _blink = 0.0f;
    bool _caretVisible = true;
    Renderer::Color _bg{30,30,30,200};
    Renderer::Color _border{200,200,200,255};
    SDL_Color _textColor{255,255,255,255};
    SDL_Color _placeholderColor{160,160,160,255};
    std::function<void(const String&)> _onSubmit;
};

// Clickable button control
class UIButton : public UIControl {
public:
    void setFont(Text::Font* font) { _font = font; }
    void setText(String t) { _text = std::move(t); }
    void setOnClick(std::function<void()> cb) { _onClick = std::move(cb); }
    void setColors(Renderer::Color normal, Renderer::Color hover, Renderer::Color pressed, Renderer::Color border, SDL_Color text) {
        _normal = normal; _hover = hover; _pressed = pressed; _border = border; _textColor = text;
    }
    void setEnabled(bool en) { _enabled = en; }
    bool enabled() const { return _enabled; }
    void focus(bool f) { _focused = f; }
    bool focused() const { return _focused; }
    // Expose pressed state for press-and-hold behaviors
    bool pressed() const { return _pressedState; }

    void handleEvents(Game::GameEvents& events) override;
    void render() const override;

private:
    static bool pointInRect(float px, float py, const SDL_FRect& r) {
        return px >= r.x && py >= r.y && px <= r.x + r.w && py <= r.y + r.h;
    }
    Text::Font* _font = nullptr;
    String _text;
    std::function<void()> _onClick;
    bool _hovered = false;
    bool _pressedState = false;
    bool _focused = false;
    bool _enabled = true;
    Renderer::Color _normal{60,60,60,200};
    Renderer::Color _hover{80,80,80,220};
    Renderer::Color _pressed{40,40,40,220};
    Renderer::Color _border{200,200,200,255};
    SDL_Color _textColor{255,255,255,255};
};

// Checkbox control with label
class UICheckbox : public UIControl {
public:
    void setFont(Text::Font* font) { _font = font; }
    void setText(String t) { _text = std::move(t); }
    void setChecked(bool c) { _checked = c; }
    bool checked() const { return _checked; }
    void setOnChanged(std::function<void(bool)> cb) { _onChanged = std::move(cb); }
    void setColors(Renderer::Color boxBg, Renderer::Color boxBorder, Renderer::Color checkCol, SDL_Color textCol) {
        _boxBg = boxBg; _boxBorder = boxBorder; _check = checkCol; _textColor = textCol;
    }
    void focus(bool f) { _focused = f; }
    bool focused() const { return _focused; }

    void handleEvents(Game::GameEvents& events) override;
    void render() const override;

private:
    static bool pointInRect(float px, float py, const SDL_FRect& r) {
        return px >= r.x && py >= r.y && px <= r.x + r.w && py <= r.y + r.h;
    }
    void toggle();

    Text::Font* _font = nullptr;
    String _text;
    std::function<void(bool)> _onChanged;
    bool _checked = false;
    bool _hovered = false;
    bool _pressed = false;
    bool _focused = false;
    Renderer::Color _boxBg{40,40,40,200};
    Renderer::Color _boxBorder{200,200,200,255};
    Renderer::Color _check{220,220,220,255};
    SDL_Color _textColor{255,255,255,255};
};

// Horizontal slider control
class UISlider : public UIControl {
public:
    void setRange(float minv, float maxv) { _min = minv; _max = maxv; _value = std::clamp(_value, _min, _max); }
    void setValue(float v) { _value = std::clamp(v, _min, _max); }
    float value() const { return _value; }
    void setStep(float s) { _step = s; }
    void setColors(Renderer::Color trackBg, Renderer::Color trackFill, Renderer::Color knobCol, Renderer::Color border) {
        _trackBg = trackBg; _trackFill = trackFill; _knob = knobCol; _border = border;
    }
    void setOnChanged(std::function<void(float)> cb) { _onChanged = std::move(cb); }
    void setOnCommit(std::function<void(float)> cb) { _onCommit = std::move(cb); }

    void handleEvents(Game::GameEvents& events) override;
    void render() const override;

private:
    static float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
    static bool pointInRect(float px, float py, const SDL_FRect& r) {
        return px >= r.x && py >= r.y && px <= r.x + r.w && py <= r.y + r.h;
    }
    float knobCenterX() const; // knob center based on value
    void setValueFromMouse(float mx);

    float _min = 0.0f, _max = 1.0f, _value = 0.5f;
    float _step = 0.0f; // 0 means continuous
    bool _dragging = false;
    Renderer::Color _trackBg{40,40,40,200};
    Renderer::Color _trackFill{100,140,220,220};
    Renderer::Color _knob{220,220,220,255};
    Renderer::Color _border{200,200,200,255};
    std::function<void(float)> _onChanged;
    std::function<void(float)> _onCommit;
};

// Render-only progress bar
class UIProgressBar : public UIControl {
public:
    void setProgress(float p) { _progress = std::clamp(p, 0.0f, 1.0f); }
    float progress() const { return _progress; }
    void setColors(Renderer::Color bg, Renderer::Color fill, Renderer::Color border) { _bg = bg; _fill = fill; _border = border; }
    void render() const override;
private:
    float _progress = 0.0f;
    Renderer::Color _bg{30,30,30,200};
    Renderer::Color _fill{80,160,80,220};
    Renderer::Color _border{200,200,200,255};
};

// Animated text box that reveals one word at a time with word wrapping
class UIAnimatedTextBox : public UIControl {
public:
    void setFont(Text::Font* font) { _font = font; }
    void setText(String t) { _text = std::move(t); tokenize(); reset(); }
    void setWordInterval(float seconds) { _wordInterval = seconds > 0.0f ? seconds : 0.01f; }
    void setStartDelay(float seconds) { _startDelay = seconds > 0.0f ? seconds : 0.0f; _delayLeft = _startDelay; }
    void setPadding(float px, float py) { _padX = px; _padY = py; }
    void setColors(Renderer::Color bg, Renderer::Color border, SDL_Color textCol) {
        _bg = bg; _border = border; _textColor = textCol;
    }
    void reset() { _visibleWords = 0; _accum = 0.0f; _delayLeft = _startDelay; }
    void revealAll() { _visibleWords = totalWords(); _delayLeft = 0.0f; }
    int totalWords() const;

    void update(float delta) override;
    void render() const override;

private:
    struct Token { String word; bool newline = false; };
    void tokenize();

    Text::Font* _font = nullptr;
    String _text;
    Vector<Token> _tokens;
    int _visibleWords = 0; // words currently revealed
    float _wordInterval = 0.2f; // seconds between words
    float _startDelay = 0.0f;    // delay before starting reveal
    float _delayLeft = 0.0f;     // remaining delay time
    float _accum = 0.0f;
    float _padX = 8.0f, _padY = 6.0f;
    float _lineGap = 4.0f;
    Renderer::Color _bg{20,20,20,200};
    Renderer::Color _border{200,200,200,255};
    SDL_Color _textColor{255,255,255,255};
};

// Image control for displaying textures inside windows/panels
class UIImage : public UIControl {
public:
    enum class ScaleMode { None, Fit, Stretch };

    void setImage(Ref<Renderer::Image> img) { _img = std::move(img); }
    bool loadFromFile(const char* path) { return Renderer::loadImageFromFile(path, _img); }
    void setScaleMode(ScaleMode m) { _mode = m; }
    void setTint(Renderer::Color c) { _tint = c; }
    void setRotation(float deg) { _rotation = deg; }
    // Tile atlas support (optional): set a grid and select a tile index
    void setTileGrid(int tileW, int tileH) { _tileW = tileW; _tileH = tileH; }
    void useTiles(bool enable) { _useTiles = enable; }
    void setTileIndex(int index) { _tileIndex = index; }
    int tileIndex() const { return _tileIndex; }
    // TileMap-backed rendering (preferred for game tiles): if set, render via TileMap::renderTile
    void setTileMapTile(Ref<TileMap::Tile> tile) { _tmTile = std::move(tile); _useTileMap = (_tmTile != nullptr); }
    void useTileMap(bool enable) { _useTileMap = enable; }

    void render() const override;

private:
    Ref<Renderer::Image> _img;
    ScaleMode _mode = ScaleMode::Fit;
    Renderer::Color _tint{255,255,255,255};
    float _rotation = 0.0f; // degrees
    // Tile atlas parameters
    bool _useTiles = false;
    int _tileW = 0;
    int _tileH = 0;
    int _tileIndex = 0; // 0-based index into the grid (row-major)
    // TileMap-backed
    bool _useTileMap = false;
    Ref<TileMap::Tile> _tmTile;
};

// Scrollable list box with single selection
class UIListBox : public UIControl {
public:
    void setFont(Text::Font* font) { _font = font; }
    void setItems(Vector<String> items) { _items = std::move(items); _selected = _items.empty() ? -1 : 0; clampScroll(); }
    int selectedIndex() const { return _selected; }
    const String* selectedItem() const { return (_selected >= 0 && _selected < (int)_items.size()) ? &_items[_selected] : nullptr; }
    void setOnChanged(std::function<void(int)> cb) { _onChanged = std::move(cb); }
    void setColors(Renderer::Color bg, Renderer::Color border, Renderer::Color selBg, SDL_Color textCol, SDL_Color selTextCol) {
        _bg = bg; _border = border; _selBg = selBg; _text = textCol; _textSel = selTextCol;
    }

    void handleEvents(Game::GameEvents& events) override;
    void render() const override;

private:
    void clampScroll();
    int visibleRows() const;
    int rowAt(float mx, float my) const;

    Text::Font* _font = nullptr;
    Vector<String> _items;
    int _selected = -1;
    int _scroll = 0; // index of first visible item
    mutable float _lastMx = -1000.0f, _lastMy = -1000.0f; // last mouse position for wheel focus
    float _rowPadY = 2.0f;
    Renderer::Color _bg{20,20,20,200};
    Renderer::Color _border{200,200,200,255};
    Renderer::Color _selBg{70,90,140,220};
    SDL_Color _text{255,255,255,255};
    SDL_Color _textSel{255,255,255,255};
    std::function<void(int)> _onChanged;
};

// Simple combo box (pulldown) for selecting one item from a small list
class UIComboBox : public UIControl {
public:
    void setFont(Text::Font* font) { _font = font; }
    void setItems(Vector<String> items) { _items = std::move(items); if (_selected < 0 || _selected >= (int)_items.size()) _selected = _items.empty() ? -1 : 0; }
    void setSelectedIndex(int idx) { if (idx >= 0 && idx < (int)_items.size()) _selected = idx; }
    int selectedIndex() const { return _selected; }
    const String* selectedItem() const { return (_selected >= 0 && _selected < (int)_items.size()) ? &_items[_selected] : nullptr; }
    void setOnChanged(std::function<void(int)> cb) { _onChanged = std::move(cb); }
    void setColors(Renderer::Color bg, Renderer::Color border, SDL_Color textCol, Renderer::Color dropBg, Renderer::Color dropBorder, Renderer::Color hoverBg) {
        _bg = bg; _border = border; _text = textCol; _dropBg = dropBg; _dropBorder = dropBorder; _hoverBg = hoverBg;
    }
    // For layout and event routing decisions in screens
    bool isOpen() const { return _open; }

    SDL_FRect hitRect() const override; // extend to include dropdown when open

    void handleEvents(Game::GameEvents& events) override;
    void render() const override;

private:
    static bool pointInRect(float px, float py, const SDL_FRect& r) { return px >= r.x && py >= r.y && px <= r.x + r.w && py <= r.y + r.h; }
    Text::Font* _font = nullptr;
    Vector<String> _items;
    int _selected = -1;
    bool _open = false;
    int _hoverRow = -1;
    int _visibleRows = 6; // dropdown rows
    SDL_Color _text{255,255,255,255};
    Renderer::Color _bg{40,40,40,200};
    Renderer::Color _border{200,200,200,255};
    Renderer::Color _dropBg{20,20,20,230};
    Renderer::Color _dropBorder{200,200,200,255};
    Renderer::Color _hoverBg{70,90,140,220};
    std::function<void(int)> _onChanged;
};

// Segmented control: multiple side-by-side segments, single selection (toggle-like)
class UISegmentedControl : public UIControl {
public:
    void setFont(Text::Font* font) { _font = font; }
    void setItems(Vector<String> items) { _items = std::move(items); if (_selected < 0 || _selected >= (int)_items.size()) _selected = _items.empty() ? -1 : 0; }
    void setSelectedIndex(int idx) { if (idx >= 0 && idx < (int)_items.size()) { _selected = idx; if (_onChanged) _onChanged(_selected); } }
    int selectedIndex() const { return _selected; }
    void setOnChanged(std::function<void(int)> cb) { _onChanged = std::move(cb); }
    void setColors(Renderer::Color bg, Renderer::Color border, Renderer::Color segBg, Renderer::Color segBgSel,
                   Renderer::Color segBgHover, SDL_Color text, SDL_Color textSel) {
        _bg = bg; _border = border; _segBg = segBg; _segBgSel = segBgSel; _segBgHover = segBgHover; _text = text; _textSel = textSel;
    }

    void handleEvents(Game::GameEvents& events) override;
    void render() const override;

private:
    static bool pointInRect(float px, float py, const SDL_FRect& r) { return px >= r.x && py >= r.y && px <= r.x + r.w && py <= r.y + r.h; }
    int segmentAt(float mx, float my) const;

    Text::Font* _font = nullptr;
    Vector<String> _items;
    int _selected = -1;
    int _hover = -1;
    int _pressed = -1;
    Renderer::Color _bg{30,30,30,200};
    Renderer::Color _border{200,200,200,255};
    Renderer::Color _segBg{50,50,60,220};
    Renderer::Color _segBgSel{70,100,140,230};
    Renderer::Color _segBgHover{60,80,110,230};
    SDL_Color _text{230,230,240,255};
    SDL_Color _textSel{255,255,255,255};
    std::function<void(int)> _onChanged;
};

}
