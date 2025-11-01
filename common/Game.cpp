#include "Common.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>

struct GameState {
	bool appInitialized = false;
	bool gameRunning = true;
	VectorRef<Game::UILayer> _layers;
	Game::GameEvents _gameEvents;
};

static GameState g_GameState{};
bool Game::initGame(const char* settings)
{
	(void)settings;
	if (!Renderer::initRenderer(1024,768)) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Renderer initialization failed.");
		return false;
	}
	#ifdef AVATARQUEST_ENABLE_AUDIO
	if (!Sound::init()) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Sound initialization failed.");
		// Not fatal to the whole app; continue running without audio.
	}
	#endif
	g_GameState.appInitialized = true;
	Window::setWindowCallBackRender(Game::render);
	Window::setWindowCallBackUpdate(Game::update);

	return true;
}

bool Game::shutDownGame()
{
	// First, let UI layers release their resources (fonts, textures, etc.)
	for (auto& layer : g_GameState._layers) {
		if (layer) {
			layer->shutDown();
		}
	}
	g_GameState._layers.clear();

	#ifdef AVATARQUEST_ENABLE_AUDIO
	Sound::shutdown();
	#endif
	if (!Renderer::shutDownRenderer()) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Renderer shutdown failed.");
		return false;
	}
	if (!Window::destroyWindow()) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window shutdown failed.");
		return false;
	}
	SDL_Quit();
	return false;
}

bool Game::runGameLoop()
{
	if (!g_GameState.appInitialized) {
		return true;
	}

	bool loopResult = Window::runMainLoop();
	if (!loopResult) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Game loop failed.");
		return false;
	}
	return g_GameState.gameRunning;
}

void Game::update(float deltaTime)
{
	#ifdef AVATARQUEST_ENABLE_AUDIO
	Sound::update();
	#endif
	for (auto& layer : g_GameState._layers) {
		layer->update(deltaTime);
	}
}

void Game::render(float deltaTime)
{
	for (auto& layer : g_GameState._layers) {
		layer->render(deltaTime);
	}
}

void Game::addlayer(Ref<UILayer>& layer) {
	g_GameState._layers.push_back(layer);
}


static inline Game::GameEvents::MouseButton ToMouseButton(uint8_t sdlButton) noexcept {
	switch (sdlButton) {
	case SDL_BUTTON_LEFT:   return Game::GameEvents::MouseButton::Left;
	case SDL_BUTTON_MIDDLE: return Game::GameEvents::MouseButton::Middle;
	case SDL_BUTTON_RIGHT:  return Game::GameEvents::MouseButton::Right;
	case SDL_BUTTON_X1:     return Game::GameEvents::MouseButton::X1;
	case SDL_BUTTON_X2:     return Game::GameEvents::MouseButton::X2;
	default:                return Game::GameEvents::MouseButton::Unknown;
	}
}


void Game::handleEvent(SDL_Event* event)
{
	if (!event) return;
	switch (event->type) {
	case SDL_EventType::SDL_EVENT_WINDOW_RESIZED:
		g_GameState._gameEvents.type = GameEvents::EventType::WindowResize;
		g_GameState._gameEvents.windowResize.width = static_cast<int>(event->display.data1);
		g_GameState._gameEvents.windowResize.height = static_cast<int>(event->display.data2);
		break;
	case SDL_EventType::SDL_EVENT_WINDOW_CLOSE_REQUESTED:
		g_GameState._gameEvents.type = GameEvents::EventType::WindowClose;
		g_GameState.gameRunning = false; // Set game running to false to end the loop
		break; 
	case SDL_EventType::SDL_EVENT_KEY_DOWN:
		g_GameState._gameEvents.type = GameEvents::EventType::KeyPress;
		g_GameState._gameEvents.keyEvent.keyCode = event->key.key;
		break;
	case SDL_EventType::SDL_EVENT_KEY_UP:
		g_GameState._gameEvents.type = GameEvents::EventType::KeyRelease;
		g_GameState._gameEvents.keyEvent.keyCode = event->key.key;
		break;
	case SDL_EventType::SDL_EVENT_MOUSE_BUTTON_UP:
		g_GameState._gameEvents.type = GameEvents::EventType::MouseButtonRelease;
		g_GameState._gameEvents.mouseButtonEvent.numClicks = event->button.clicks;
		g_GameState._gameEvents.mouseButtonEvent.button = ToMouseButton(event->button.button);
		g_GameState._gameEvents.mouseButtonEvent.x = static_cast<int>(event->button.x);
		g_GameState._gameEvents.mouseButtonEvent.y = static_cast<int>(event->button.y);
		break;
	case SDL_EventType::SDL_EVENT_MOUSE_BUTTON_DOWN:
		g_GameState._gameEvents.mouseButtonEvent.numClicks = event->button.clicks;
		if (g_GameState._gameEvents.mouseButtonEvent.numClicks >= 2) {
			g_GameState._gameEvents.type = GameEvents::EventType::MouseButtonDoubleClick;
		}
		else {
			g_GameState._gameEvents.type = GameEvents::EventType::MouseButtonPress;
		}
		g_GameState._gameEvents.mouseButtonEvent.button = ToMouseButton(event->button.button);
		g_GameState._gameEvents.mouseButtonEvent.x = static_cast<int>(event->button.x);
		g_GameState._gameEvents.mouseButtonEvent.y = static_cast<int>(event->button.y);
		break;
	case SDL_EventType::SDL_EVENT_MOUSE_MOTION:
		g_GameState._gameEvents.mouseButtonEvent.numClicks = event->button.clicks;
		g_GameState._gameEvents.type = GameEvents::EventType::MouseMove;
		g_GameState._gameEvents.mouseMoveEvent.x = static_cast<int>(event->motion.x);
		g_GameState._gameEvents.mouseMoveEvent.y = static_cast<int>(event->motion.y);
		break;
	case SDL_EventType::SDL_EVENT_MOUSE_WHEEL:
		g_GameState._gameEvents.type = GameEvents::EventType::MouseScroll;
		g_GameState._gameEvents.mouseScrollEvent.deltaX = static_cast<int>(event->wheel.x);
		g_GameState._gameEvents.mouseScrollEvent.deltaY = static_cast<int>(event->wheel.y);
		break;
	default:
		return; // Unhandled event type
	}

}

Game::GameEvents& Game::getGameEvents() 
{
	return g_GameState._gameEvents;
}

void Game::endGameLoop()
{
	g_GameState.gameRunning = false;
}
