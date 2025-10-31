#pragma once

#include "Common.h"

namespace AvatarQuest {

	enum struct PlayerMovement : uint16_t {
		Move_None = 0,
		Move_Up,
		Move_Down,
		Move_Left,
		Move_Right
	};

	enum struct PlayerCameraType : uint16_t {
		Canera_WorldMap = 0,
		Camera_Battlemap
	};

	struct PlayerCamera {
		PlayerCameraType cameraType = PlayerCameraType::Canera_WorldMap;
		Vector2    playerWorldPosition { 0.0f, 0.0f };
		SDL_FRect  viewPortRatio       { 0.0f, 0.0f, 1.0f, 1.0f };
		SDL_FRect  playerViewport      { 0.0f, 0.0f, 1280.0f, 1024.0f };
		Vector2    playerCenterScreen  { 640.0f, 512.0f };
		TileVector playerTilePosition  { 64, 64 };
		PlayerMovement currentMovement = PlayerMovement::Move_None;
	};

	// Initialize player camera from window size and starting tile position.
	void createPlayer(SDL_Rect& windowSize, TileVector& startingPosition, Ref<PlayerCamera>& cameraOut);

	// Get the global player camera instance.
	void GetPlayerCamera(Ref<PlayerCamera>& outCamera);

	// Move the camera by one tile in the given direction.
	bool moveCamera(PlayerMovement moveDir, float deltaTime);

	// For future expansion; currently a no-op.
	void updatePlayerCamera(float deltaTime);

	// Update movement state from input events.
	void onEventPlayerInput(Game::GameEvents& events);

	// Optional helper to draw player/camera debug gizmos.
	void drawPlayer();

}