#include "AvatarQuest/Fonts.h"

namespace AvatarQuest { namespace Fonts {

static UMap<int, Text::Font*> s_fonts; // pointSize -> font*

static const char* kFontCandidates[] = {
    "assets/fonts/DroidSerifBold-aMPE.ttf",
    "../assets/fonts/DroidSerifBold-aMPE.ttf",
    "../../assets/fonts/DroidSerifBold-aMPE.ttf"
};

void init() {
    // No-op: lazy load on demand
}

Text::Font* get(int pointSize) {
    auto it = s_fonts.find(pointSize);
    if (it != s_fonts.end()) return it->second;

    Text::Font* f = nullptr;
    for (const char* p : kFontCandidates) {
        f = Text::loadFont(p, pointSize);
        if (f) {
            SDL_Log("[Fonts] Loaded font size %d from %s", pointSize, p);
            break;
        }
    }
    if (!f) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Fonts] Failed to load font size %d from candidates", pointSize);
    }
    s_fonts[pointSize] = f; // cache nullptr to avoid repeated attempts
    return f;
}

void shutdown() {
    // Let the Text subsystem unload/close all fonts safely and handle TTF_Quit ordering.
    // Our cache only stores raw pointers into Text's internal list; after Text::shutdown()
    // those become invalid, so just clear without individually unloading to avoid double free
    // or calling into SDL_ttf after it has been torn down.
    Text::shutdown();
    s_fonts.clear();
}

} } // namespace AvatarQuest::Fonts
