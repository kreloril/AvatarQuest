#pragma once


namespace Game
{
	
	struct GameEvents {
		enum class EventType {
			None = 0,
			WindowResize,
			WindowClose,
			KeyPress,
			KeyRelease,
			MouseButtonPress,
			MouseButtonRelease,
			MouseMove,
			MouseScroll,
			MouseButtonDoubleClick
		};

		enum class MouseButton : int {
			Unknown = 0,
			Left = 1, // SDL_BUTTON_LEFT
			Middle = 2, // SDL_BUTTON_MIDDLE
			Right = 3, // SDL_BUTTON_RIGHT
			X1 = 4, // SDL_BUTTON_X1
			X2 = 5  // SDL_BUTTON_X2
		};

		EventType type;
		union {
			struct { int width, height; } windowResize;
			struct { int keyCode; } keyEvent;
			struct { MouseButton button = MouseButton::Unknown; int x, y; int numClicks; } mouseButtonEvent;
			struct { int x, y; } mouseMoveEvent;
			struct { int deltaX, deltaY; } mouseScrollEvent;

		};

		template <typename... ETs>
		[[nodiscard]] inline bool isEventType(ETs... ets) const noexcept {
			static_assert(sizeof...(ETs) >= 1, "isEventType requires at least one EventType");
			static_assert((std::is_same_v<ETs, EventType> && ...),
				"isEventType only accepts GameEvents::EventType");
			const EventType cur = type;
			return ((cur == ets) || ...);
		}

		bool isMouseButton(Game::GameEvents::MouseButton btn) const {
			return (type == EventType::MouseButtonPress || type == EventType::MouseButtonRelease || type == EventType::MouseButtonDoubleClick) && mouseButtonEvent.button == btn;
		}

		void reset() {
			type = EventType::None;
			std::memset(&mouseButtonEvent, 0, sizeof(mouseButtonEvent));
		}
	};

	bool initGame(const char* settings);
	bool shutDownGame();
	bool runGameLoop();
	void update(float deltaTime);
	void render(float deltaTime);
	void addlayer(Ref<UILayer>&);
	void handleEvent(SDL_Event* event);
	GameEvents& getGameEvents();
	void endGameLoop();
};

