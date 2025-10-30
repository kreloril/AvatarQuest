#include <catch2/catch_test_macros.hpp>
#include "Common.h"
#include "AvatarQuestPlayer.h"

using namespace AvatarQuest;

TEST_CASE("Player camera: create and basic fields", "[player][camera]") {
    SDL_Rect window{ 0, 0, 1280, 720 };
    TileVector start{ 10, 20 };

    Ref<PlayerCamera> cam;
    createPlayer(window, start, cam);

    REQUIRE(cam != nullptr);

    // Viewport should reflect window rect
    REQUIRE(cam->playerViewport.x == static_cast<float>(window.x));
    REQUIRE(cam->playerViewport.y == static_cast<float>(window.y));
    REQUIRE(cam->playerViewport.w == static_cast<float>(window.w));
    REQUIRE(cam->playerViewport.h == static_cast<float>(window.h));

    // Screen center should be the center of the window
    REQUIRE(cam->playerCenterScreen.x == (window.x + window.w * 0.5f));
    REQUIRE(cam->playerCenterScreen.y == (window.y + window.h * 0.5f));

    // Starting tile set
    REQUIRE(cam->playerTilePosition.x == start.x);
    REQUIRE(cam->playerTilePosition.y == start.y);

    // Default movement is none
    REQUIRE(cam->currentMovement == PlayerMovement::Move_None);
}

TEST_CASE("Player camera: moveCamera steps tiles", "[player][camera][movement]") {
    SDL_Rect window{ 0, 0, 800, 600 };
    TileVector start{ 0, 0 };
    Ref<PlayerCamera> cam;
    createPlayer(window, start, cam);

    REQUIRE(moveCamera(PlayerMovement::Move_None, 0.016f) == false);

    REQUIRE(moveCamera(PlayerMovement::Move_Right, 0.016f) == true);
    REQUIRE(cam->playerTilePosition.x == 1);
    REQUIRE(cam->playerTilePosition.y == 0);

    REQUIRE(moveCamera(PlayerMovement::Move_Up, 0.016f) == true);
    REQUIRE(cam->playerTilePosition.x == 1);
    REQUIRE(cam->playerTilePosition.y == -1);

    REQUIRE(moveCamera(PlayerMovement::Move_Left, 0.016f) == true);
    REQUIRE(cam->playerTilePosition.x == 0);
    REQUIRE(cam->playerTilePosition.y == -1);

    REQUIRE(moveCamera(PlayerMovement::Move_Down, 0.016f) == true);
    REQUIRE(cam->playerTilePosition.x == 0);
    REQUIRE(cam->playerTilePosition.y == 0);
}

TEST_CASE("Player camera: input mapping via onEventPlayerInput", "[player][camera][input]") {
    SDL_Rect window{ 0, 0, 320, 240 };
    TileVector start{ 5, 5 };
    Ref<PlayerCamera> cam;
    createPlayer(window, start, cam);

    Game::GameEvents ev{};

    // Press W -> Move_Up
    ev.type = Game::GameEvents::EventType::KeyPress;
    ev.keyEvent.keyCode = SDLK_W;
    onEventPlayerInput(ev);
    REQUIRE(cam->currentMovement == PlayerMovement::Move_Up);
    REQUIRE(moveCamera(cam->currentMovement, 0.016f) == true);
    REQUIRE(cam->playerTilePosition.x == 5);
    REQUIRE(cam->playerTilePosition.y == 4);

    // Key release -> reset movement
    ev.type = Game::GameEvents::EventType::KeyRelease;
    onEventPlayerInput(ev);
    REQUIRE(cam->currentMovement == PlayerMovement::Move_None);
}
