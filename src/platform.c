#include "AvatarQuest/engine.h"

#include <SDL3/SDL.h>

static SDL_Window *g_window = NULL;
static SDL_Renderer *g_renderer = NULL;
static const int WINDOW_WIDTH = 800;
static const int WINDOW_HEIGHT = 600;
static const char *WINDOW_TITLE = "AvatarQuest";

bool avatarquest_platform_initialize(void) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL could not initialize: %s", SDL_GetError());
        return false;
    }

    g_window = SDL_CreateWindow(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (g_window == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window could not be created: %s", SDL_GetError());
        SDL_Quit();
        return false;
    }

    g_renderer = SDL_CreateRenderer(g_window, NULL);
    if (g_renderer == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Renderer could not be created: %s", SDL_GetError());
        SDL_DestroyWindow(g_window);
        g_window = NULL;
        SDL_Quit();
        return false;
    }

    if (SDL_SetRenderVSync(g_renderer, true) < 0) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Failed to enable VSync: %s", SDL_GetError());
    }

    return true;
}

void avatarquest_platform_shutdown(void) {
    if (g_renderer != NULL) {
        SDL_DestroyRenderer(g_renderer);
        g_renderer = NULL;
    }

    if (g_window != NULL) {
        SDL_DestroyWindow(g_window);
        g_window = NULL;
    }

    SDL_Quit();
}

void avatarquest_platform_mainloop(struct AvatarQuestGame *game) {
    if (game == NULL) {
        return;
    }

    if (g_renderer == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Renderer is not initialized.");
        return;
    }

    avatarquest_game_attach_renderer(game, g_renderer);

    Uint64 previous_ticks = SDL_GetTicks();
    while (avatarquest_game_is_running(game)) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            avatarquest_game_handle_event(game, &event);
        }

        Uint64 current_ticks = SDL_GetTicks();
        float delta_time = (float)(current_ticks - previous_ticks) / 1000.0f;
        previous_ticks = current_ticks;

        avatarquest_game_update(game, delta_time);
        avatarquest_game_render(game);

        SDL_Delay(1);
    }
}
