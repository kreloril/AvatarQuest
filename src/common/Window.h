#pragma once
namespace Window
{
	using WindowCallBack = void(*)(float deltaTime);
	bool createWindow(int width, int height, const char* title);
	SDL_Window* getWindowHandle();
	bool destroyWindow();
	bool runMainLoop();
	void setWindowCallBackUpdate(WindowCallBack callBack);
	void setWindowCallBackRender(WindowCallBack callBack);
	SDL_Rect getWindowSize();
};

