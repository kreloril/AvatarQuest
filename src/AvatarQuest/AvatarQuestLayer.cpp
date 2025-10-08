#include "AvatarQuestLayer.h"

#include <SDL3_image/SDL_image.h>
#include <vector>
#include <cstring>

namespace AvatarQuest {

AvatarQuestLayer::AvatarQuestLayer() : m_texture(nullptr), m_texW(0), m_texH(0) {}
AvatarQuestLayer::~AvatarQuestLayer() { shutDown(); }

// A tiny 2x2 PNG (red, green, blue, white) — encoded PNG bytes
static const unsigned char test_png[] = {
    0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A,0x00,0x00,0x00,0x0D,0x49,0x48,0x44,0x52,
    0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x02,0x08,0x06,0x00,0x00,0x00,0xF4,0x78,0xD4,
    0xFA,0x00,0x00,0x00,0x0A,0x49,0x44,0x41,0x54,0x78,0x9C,0x63,0x60,0x60,0x60,0x00,0x00,
    0x00,0x05,0x00,0x01,0x0D,0x0A,0x2D,0xB4,0x00,0x00,0x00,0x00,0x49,0x45,0x4E,0x44,0xAE,
    0x42,0x60,0x82
};

void AvatarQuestLayer::init()
{
    // Load PNG from memory using SDL_RWops and SDL_image
        SDL_IOStream* io = SDL_IOFromConstMem(test_png, sizeof(test_png));
        if (!io) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create IOStream for test PNG: %s", SDL_GetError());
            return;
        }

        // Ask SDL_image to load the image from the IO stream; tell it the type is PNG
        SDL_Surface* surf = IMG_LoadTyped_IO(io, true, "PNG"); // SDL_image will close/free the IO stream
        if (!surf) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "IMG_LoadTyped_IO failed: %s", SDL_GetError());
            return;
        }

    SDL_Renderer* ren = Renderer::getRenderer();
    if (!ren) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Renderer not available when creating test texture.");
            SDL_DestroySurface(surf);
        return;
    }

    m_texture = SDL_CreateTextureFromSurface(ren, surf);
    if (!m_texture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateTextureFromSurface failed: %s", SDL_GetError());
    } else {
        m_texW = (float)surf->w;
        m_texH = (float)surf->h;
    }

        SDL_DestroySurface(surf);
}

void AvatarQuestLayer::render(float /*deltaTime*/)
{
    if (!m_texture) return;
    SDL_Renderer* ren = Renderer::getRenderer();
    if (!ren) return;

    // Get renderer output size
    int rw = 0, rh = 0;
    SDL_GetRenderOutputSize(ren, &rw, &rh);

    SDL_FRect dst = { (rw - m_texW) * 0.5f, (rh - m_texH) * 0.5f, m_texW, m_texH };
        SDL_RenderTexture(ren, m_texture, nullptr, &dst);
}

void AvatarQuestLayer::shutDown()
{
    if (m_texture) {
        SDL_DestroyTexture(m_texture);
        m_texture = nullptr;
    }
}

void AvatarQuestLayer::handleEvents(float /*delta*/, Game::GameEvents& events) {
    using Game::GameEvents;
    if (events.isEventType(GameEvents::EventType::KeyPress)) {
        // SDL keycodes are forwarded in keyEvent.keyCode
        if (events.keyEvent.keyCode == SDLK_ESCAPE) {
            Game::endGameLoop();
        }
    }
}

// Ensure we call the event handler from update so layer can react to events
void AvatarQuestLayer::update(float deltaTime) {
    Game::GameEvents& events = Game::getGameEvents();
    handleEvents(deltaTime, events);
}

Ref<Game::UILayer> CreateAvatarQuestLayer() {
    return CreateRef<AvatarQuestLayer>();
}

}
