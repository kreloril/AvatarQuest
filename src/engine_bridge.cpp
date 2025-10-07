#include "AvatarQuest/engine.h"

#include "common.h"

struct AvatarQuestGame {
    bool running = true;
};

bool avatarquest_platform_initialize(void) {
    // Create the window and initialize engine subsystems
    if (!Window::createWindow(800, 600, "AvatarQuest")) {
        return false;
    }
    return true;
}

void avatarquest_platform_shutdown(void) {
    Window::destroyWindow();
}

void avatarquest_platform_mainloop(struct AvatarQuestGame *game) {
    (void)game;
    Window::runMainLoop();
}

AvatarQuestGame *avatarquest_game_create(void) {
    AvatarQuestGame *g = (AvatarQuestGame *)SDL_calloc(1, sizeof(AvatarQuestGame));
    return g;
}

void avatarquest_game_destroy(AvatarQuestGame *game) {
    if (game) SDL_free(game);
}

void avatarquest_game_update(AvatarQuestGame *game, float delta_time) {
    (void)game; (void)delta_time;
    // Engine drives update internally via Window callbacks.
}

void avatarquest_game_handle_event(AvatarQuestGame *game, const void *event) {
    (void)game; (void)event;
    // Engine handles events via Game::handleEvent (wired in Main.cpp)
}

void avatarquest_game_render(AvatarQuestGame *game) {
    (void)game;
    // Engine render path is internal
}

void avatarquest_game_attach_renderer(AvatarQuestGame *game, void *renderer) {
    (void)game; (void)renderer;
}

bool avatarquest_game_is_running(const AvatarQuestGame *game) {
    return game != NULL && game->running;
}
