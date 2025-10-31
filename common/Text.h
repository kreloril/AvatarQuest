#pragma once

#include <string>

// Forward declaration to avoid forcing all includers to have SDL_ttf headers
struct TTF_Font;

namespace Text {

    // Initialize/Shutdown SDL_ttf. Must be called after SDL_Init and before quitting SDL.
    bool init();
    void shutdown();

    struct Font {
        TTF_Font* handle = nullptr;
        int size = 0;
    };

    // Load a font at a given point size. Caller owns and must unload via unloadFont.
    // Returns nullptr on failure.
    Font* loadFont(const std::string& path, int ptSize);
    void unloadFont(Font* font);

    // Measure text using the given font; returns width/height in pixels.
    SDL_FRect measure(Font* font, const std::string& text);

    // Draw UTF-8 text at top-left pixel position (x,y) with RGBA color.
    // This creates a transient texture per draw. For HUD text this is fine;
    // consider caching if drawing the same strings frequently.
    void draw(Font* font, const std::string& text, float x, float y, SDL_Color color);
}
