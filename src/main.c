#include <SDL3/SDL.h>
#include "AvatarQuest/engine.h"

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    if (!avatarquest_platform_initialize()) {
        return 1;
    }

    AvatarQuestGame *game = avatarquest_game_create();
    if (game == NULL) {
        avatarquest_platform_shutdown();
        return 1;
    }

    avatarquest_platform_mainloop(game);

    avatarquest_game_destroy(game);
    avatarquest_platform_shutdown();

    return 0;
}
