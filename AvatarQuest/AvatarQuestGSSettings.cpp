#include "AvatarQuestGSSettings.h"
#include "AvatarQuest/Fonts.h"

namespace AvatarQuest {

static inline int clampi(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }
static inline int pctFrom128(int v) { return clampi(int(std::round((v / 128.0f) * 100.0f)), 0, 100); }

// Static back target default: return to Main Menu unless overridden by caller
AvatarQuest::AQStateId GSSettings::s_backTarget = AvatarQuest::AQStateId::MainMenu;

void GSSettings::SetBackTarget(AQStateId target) { s_backTarget = target; }
AQStateId GSSettings::GetBackTarget() { return s_backTarget; }

void GSSettings::onEnter() {
    _font = Fonts::ui();
    SDL_Rect ws = Window::getWindowSize();
    const float winW = 520.0f;
    const float winH = 380.0f; // taller to prevent Back button clipping
    const float wx = (float)ws.x + (float)ws.w * 0.5f - winW * 0.5f;
    const float wy = (float)ws.y + (float)ws.h * 0.5f - winH * 0.5f;
    _window.setPosition(wx, wy);
    _window.setSize(winW, winH);
    _window.setTitle(_font, "Settings");
    _window.setColors(Renderer::Color{30,30,40,220}, Renderer::Color{200,200,220,255});

    _title.setFont(_font);
    _title.setText("Audio");

    // Setup sliders in 0..128 (SDL mixer typical), step 1
    _lblMaster.setFont(_font);
    _lblMaster.setText("Master Volume: 100%");
    _slMaster.setRange(0.0f, 128.0f);
    _slMaster.setValue(128.0f);
    _slMaster.setStep(1.0f);
    _slMaster.setColors(Renderer::Color{40,40,40,200}, Renderer::Color{140,140,220,220}, Renderer::Color{230,230,230,255}, Renderer::Color{200,200,200,255});
    _slMaster.setOnChanged([this](float v){
        int iv = clampi((int)std::round(v), 0, 128);
        _lblMaster.setText("Master Volume: " + std::to_string(pctFrom128(iv)) + "%");
        #ifdef AVATARQUEST_ENABLE_AUDIO
        Sound::setMasterVolume(iv);
        #endif
    });

    _lblMusic.setFont(_font);
    _lblMusic.setText("Music Volume: 100%");
    _slMusic.setRange(0.0f, 128.0f);
    _slMusic.setValue(128.0f);
    _slMusic.setStep(1.0f);
    _slMusic.setColors(Renderer::Color{40,40,40,200}, Renderer::Color{90,140,220,220}, Renderer::Color{230,230,230,255}, Renderer::Color{200,200,200,255});
    _slMusic.setOnChanged([this](float v){
        int iv = clampi((int)std::round(v), 0, 128);
        _lblMusic.setText("Music Volume: " + std::to_string(pctFrom128(iv)) + "%");
        #ifdef AVATARQUEST_ENABLE_AUDIO
        Sound::setMusicVolume(iv);
        #endif
    });

    _lblSfx.setFont(_font);
    _lblSfx.setText("SFX Volume: 100%");
    _slSfx.setRange(0.0f, 128.0f);
    _slSfx.setValue(128.0f);
    _slSfx.setStep(1.0f);
    _slSfx.setColors(Renderer::Color{40,40,40,200}, Renderer::Color{120,180,120,220}, Renderer::Color{230,230,230,255}, Renderer::Color{200,200,200,255});
    _slSfx.setOnChanged([this](float v){
        int iv = clampi((int)std::round(v), 0, 128);
        _lblSfx.setText("SFX Volume: " + std::to_string(pctFrom128(iv)) + "%");
        #ifdef AVATARQUEST_ENABLE_AUDIO
        Sound::setSfxVolume(iv);
        #endif
    });

    _btnBack.setFont(_font);
    _btnBack.setText("Back");
    _btnBack.setColors(Renderer::Color{60,60,60,200}, Renderer::Color{80,80,80,220}, Renderer::Color{40,40,40,220}, Renderer::Color{200,200,200,255}, SDL_Color{255,255,255,255});
    _btnBack.setOnClick([this](){ _next = s_backTarget; });
}

AQStateId GSSettings::handleEvents(float /*delta*/, Game::GameEvents& events) {
    // Layout before hit-testing
    SDL_FRect cr = _window.contentRect();
    SDL_FRect lm = _font ? Text::measure(_font, "Mg") : SDL_FRect{0,0,0,20};

    float y = cr.y;
    _title.setPosition(cr.x, y);
    y += lm.h + 8.0f;

    _lblMaster.setPosition(cr.x, y);
    y += lm.h + 6.0f;
    _slMaster.setPosition(cr.x, y);
    _slMaster.setSize(cr.w, 18.0f);
    y += 26.0f;

    _lblMusic.setPosition(cr.x, y);
    y += lm.h + 6.0f;
    _slMusic.setPosition(cr.x, y);
    _slMusic.setSize(cr.w, 18.0f);
    y += 26.0f;

    _lblSfx.setPosition(cr.x, y);
    y += lm.h + 6.0f;
    _slSfx.setPosition(cr.x, y);
    _slSfx.setSize(cr.w, 18.0f);
    y += 30.0f;

    const float btnW = 120.0f;
    const float btnH = lm.h + 10.0f;
    y = std::min(y, cr.y + cr.h - btnH);
    _btnBack.setPosition(cr.x + cr.w - btnW, y);
    _btnBack.setSize(btnW, btnH);

    // Events to controls
    _slMaster.handleEvents(events);
    _slMusic.handleEvents(events);
    _slSfx.handleEvents(events);
    _btnBack.handleEvents(events);

    if (_next != AQStateId::None) { AQStateId out = _next; _next = AQStateId::None; events.reset(); return out; }

    if (events.type == Game::GameEvents::EventType::KeyPress) {
        if (events.keyEvent.keyCode == SDLK_ESCAPE) { events.reset(); return s_backTarget; }
    }
    // Back button click
    // UIButton invokes its callback; we can check focus+enter, but it's already handled inside the control
    return AQStateId::None;
}

void GSSettings::update(float /*delta*/) {}

void GSSettings::render(float /*delta*/) {
    _window.render();
    SDL_FRect cr = _window.contentRect();
    SDL_FRect lm = _font ? Text::measure(_font, "Mg") : SDL_FRect{0,0,0,20};

    float y = cr.y;
    _title.setPosition(cr.x, y);
    _title.render();
    y += lm.h + 8.0f;

    _lblMaster.setPosition(cr.x, y);
    _lblMaster.render();
    y += lm.h + 6.0f;
    _slMaster.setPosition(cr.x, y);
    _slMaster.setSize(cr.w, 18.0f);
    _slMaster.render();
    y += 26.0f;

    _lblMusic.setPosition(cr.x, y);
    _lblMusic.render();
    y += lm.h + 6.0f;
    _slMusic.setPosition(cr.x, y);
    _slMusic.setSize(cr.w, 18.0f);
    _slMusic.render();
    y += 26.0f;

    _lblSfx.setPosition(cr.x, y);
    _lblSfx.render();
    y += lm.h + 6.0f;
    _slSfx.setPosition(cr.x, y);
    _slSfx.setSize(cr.w, 18.0f);
    _slSfx.render();
    y += 30.0f;

    const float btnW = 120.0f;
    const float btnH = lm.h + 10.0f;
    y = std::min(y, cr.y + cr.h - btnH);
    _btnBack.setPosition(cr.x + cr.w - btnW, y);
    _btnBack.setSize(btnW, btnH);
    _btnBack.render();
}

}
