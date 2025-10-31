// Player camera implementation
#include "AvatarQuestPlayer.h"

static Ref<AvatarQuest::PlayerCamera> g_PlayerCamera = nullptr;

namespace AvatarQuest {

	void createPlayer(SDL_Rect& windowSize, TileVector& startingPosition, Ref<AvatarQuest::PlayerCamera>& cameraOut)
	{
		g_PlayerCamera = std::make_shared<PlayerCamera>();
		// Initialize viewport and center based on window
		g_PlayerCamera->playerViewport = { (float)windowSize.x, (float)windowSize.y, (float)windowSize.w, (float)windowSize.h };
		g_PlayerCamera->playerCenterScreen = {
			windowSize.x + (windowSize.w * 0.5f),
			windowSize.y + (windowSize.h * 0.5f)
		};
		g_PlayerCamera->playerTilePosition = startingPosition;
		g_PlayerCamera->playerWorldPosition = { 0.0f, 0.0f }; // Layer will compute actual world pos from tile
		g_PlayerCamera->currentMovement = PlayerMovement::Move_None;

		cameraOut = g_PlayerCamera;
	}

	void GetPlayerCamera(Ref<PlayerCamera>& outCamera)
	{
		outCamera = g_PlayerCamera;
	}

	bool moveCamera(PlayerMovement moveDir, float /*deltaTime*/)
	{
		if (!g_PlayerCamera) return false;
		if (moveDir == PlayerMovement::Move_None) return false;

		TileVector pos = g_PlayerCamera->playerTilePosition;
		switch (moveDir) {
		case PlayerMovement::Move_Up:    pos.y -= 1; break;
		case PlayerMovement::Move_Down:  pos.y += 1; break;
		case PlayerMovement::Move_Left:  pos.x -= 1; break;
		case PlayerMovement::Move_Right: pos.x += 1; break;
		default: return false;
		}
		g_PlayerCamera->playerTilePosition = pos;
		g_PlayerCamera->currentMovement = PlayerMovement::Move_None;
		return true;
	}

	void updatePlayerCamera(float /*deltaTime*/)
	{
		// Currently handled by the layer (needs tile size to compute world pos).
		// Hook for smoothing, lerp, etc.
	}

	void onEventPlayerInput(Game::GameEvents& events)
	{
		if (!g_PlayerCamera) return;

		using Game::GameEvents;
		if (events.isEventType(GameEvents::EventType::KeyRelease)) {
			g_PlayerCamera->currentMovement = PlayerMovement::Move_None;
			return;
		}
		if (events.isEventType(GameEvents::EventType::KeyPress)) {
			switch (events.keyEvent.keyCode) {
			case SDLK_W: g_PlayerCamera->currentMovement = PlayerMovement::Move_Up;    break;
			case SDLK_S: g_PlayerCamera->currentMovement = PlayerMovement::Move_Down;  break;
			case SDLK_A: g_PlayerCamera->currentMovement = PlayerMovement::Move_Left;  break;
			case SDLK_D: g_PlayerCamera->currentMovement = PlayerMovement::Move_Right; break;
			default: break;
			}
		}
	}

	void drawPlayer()
	{
#ifdef AQ_BUILD_TESTS
		// In test builds, avoid linking against Renderer. No-op.
		(void)g_PlayerCamera;
		return;
#else
		if (!g_PlayerCamera) return;
		// Simple debug: draw a green rect at the player's screen center, sized by a tile.
		// The layer decides actual tile size; we use 32x32 as a placeholder if needed.
		Renderer::drawRect(
			g_PlayerCamera->playerCenterScreen.x,
			g_PlayerCamera->playerCenterScreen.y,
			32.0f, 32.0f,
			{ 0,255,0,255 }
		);
#endif
	}
}


