#include "AvatarQuest/game.h"

#include <SDL3/SDL.h>
#include <stdlib.h>

struct AvatarQuestGame {
    bool running;
    SDL_Renderer *renderer;
};

AvatarQuestGame *avatarquest_game_create(void) {
    AvatarQuestGame *game = (AvatarQuestGame *)SDL_calloc(1, sizeof(AvatarQuestGame));
    if (game == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to allocate game state: %s", SDL_GetError());
        return NULL;
    }

    game->running = true;
    return game;
}

void avatarquest_game_destroy(AvatarQuestGame *game) {
    if (game == NULL) {
        return;
    }

    SDL_free(game);
}

void avatarquest_game_update(AvatarQuestGame *game, float delta_time) {
    (void)delta_time;

    if (game == NULL) {
        return;
    }

    // Placeholder for future game logic.
}

void avatarquest_game_handle_event(AvatarQuestGame *game, const SDL_Event *event) {
    if (game == NULL || event == NULL) {
        return;
    }

    if (event->type == SDL_EVENT_QUIT) {
        game->running = false;
    } else if (event->type == SDL_EVENT_KEY_DOWN && event->key.key == SDLK_ESCAPE) {
        game->running = false;
    }
}

void avatarquest_game_render(AvatarQuestGame *game) {
    if (game == NULL || game->renderer == NULL) {
        return;
    }

    SDL_SetRenderDrawColor(game->renderer, 12, 24, 36, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(game->renderer);

    SDL_SetRenderDrawColor(game->renderer, 255, 215, 0, SDL_ALPHA_OPAQUE);
    SDL_FRect hero = {360.0f, 260.0f, 80.0f, 80.0f};
    SDL_RenderFillRect(game->renderer, &hero);

    SDL_RenderPresent(game->renderer);
}

void avatarquest_game_attach_renderer(AvatarQuestGame *game, SDL_Renderer *renderer) {
    if (game == NULL) {
        return;
    }

    game->renderer = renderer;
}

bool avatarquest_game_is_running(const AvatarQuestGame *game) {
    return game != NULL && game->running;
}
