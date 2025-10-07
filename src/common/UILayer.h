#pragma once


namespace Game
{
	struct GameEvents;

	class UILayer
	{
	public:
		UILayer() = default;
		virtual ~UILayer() = default;
		virtual void init() = 0;
		virtual void update(float deltaTime) = 0;
		virtual void render(float deltaTime) = 0;
		virtual void shutDown() = 0;
		virtual void handleEvents(float delta, GameEvents& events) = 0;
	};

}