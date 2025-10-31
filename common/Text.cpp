#include "Common.h"
#include "Text.h"
#include <SDL3_ttf/SDL_ttf.h>

namespace {
    bool g_ttfInited = false;
    Vector<Ref<Text::Font>> g_fonts;
}

bool Text::init()
{
    if (g_ttfInited) return true;
    if (!TTF_Init()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TTF_Init failed: %s", SDL_GetError());
        return false;
    }
    g_ttfInited = true;
    return true;
}

void Text::shutdown()
{
    // Unload any remaining fonts
    for (auto& f : g_fonts) {
        if (f && f->handle) {
            TTF_CloseFont(f->handle);
            f->handle = nullptr;
        }
    }
    g_fonts.clear();

    if (g_ttfInited) {
        TTF_Quit();
        g_ttfInited = false;
    }
}

Text::Font* Text::loadFont(const std::string& path, int ptSize)
{
    if (!g_ttfInited) {
        // Attempt to init on demand if not already
        if (!Text::init()) return nullptr;
    }

    TTF_Font* f = TTF_OpenFont(path.c_str(), static_cast<float>(ptSize));
    if (!f) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TTF_OpenFont failed for '%s': %s", path.c_str(), SDL_GetError());
        return nullptr;
    }
    auto fontRef = CreateRef<Text::Font>();
    fontRef->handle = f;
    fontRef->size = ptSize;
    g_fonts.push_back(fontRef);
    return fontRef.get();
}

void Text::unloadFont(Text::Font* font)
{
    if (!font) return;
    // Find and release from our list; caller's pointer becomes invalid after this
    for (auto it = g_fonts.begin(); it != g_fonts.end(); ++it) {
        if (it->get() == font) {
            if ((*it)->handle) {
                TTF_CloseFont((*it)->handle);
                (*it)->handle = nullptr;
            }
            g_fonts.erase(it);
            break;
        }
    }
}

SDL_FRect Text::measure(Text::Font* font, const std::string& text)
{
    SDL_FRect r{0,0,0,0};
    if (!font || !font->handle) return r;
    int w = 0, h = 0;
    if (TTF_GetStringSize(font->handle, text.c_str(), text.size(), &w, &h)) {
        r.w = static_cast<float>(w);
        r.h = static_cast<float>(h);
    }
    return r;
}

void Text::draw(Text::Font* font, const std::string& text, float x, float y, SDL_Color color)
{
    if (!font || !font->handle || text.empty()) return;
    SDL_Surface* surface = TTF_RenderText_Blended(font->handle, text.c_str(), text.size(), color);
    if (!surface) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TTF_RenderText_Blended failed: %s", SDL_GetError());
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(Renderer::getRenderer(), surface);
    if (!texture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateTextureFromSurface failed: %s", SDL_GetError());
        SDL_DestroySurface(surface);
        return;
    }
    // Ensure text blends properly with background
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    SDL_FRect dst{ x, y, static_cast<float>(surface->w), static_cast<float>(surface->h) };
    // Blit without rotation or scaling for now
    SDL_RenderTexture(Renderer::getRenderer(), texture, nullptr, &dst);

    SDL_DestroyTexture(texture);
    SDL_DestroySurface(surface);
}
