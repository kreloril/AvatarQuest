#include "Common.h"
#include <iostream>

static SDL_Window* g_WindowState = nullptr;
Window::WindowCallBack g_WindowCallBackUpdate = nullptr;
Window::WindowCallBack g_WindowCallBackRender = nullptr;

bool Window::createWindow(int width, int height, const char* title)
{
    if (g_WindowState) {
        destroyWindow();
    }

    (void)width; (void)height;
    SDL_Rect desktop = { 0 };
    SDL_GetDisplayBounds(SDL_GetPrimaryDisplay(), &desktop);

    g_WindowState = SDL_CreateWindow(title, desktop.w, desktop.h, SDL_WINDOW_BORDERLESS);
    if (g_WindowState == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not create window: %s", SDL_GetError());

        return false;
    }

 //   SDL_SetWindowAlwaysOnTop(g_WindowState, true);

    return true;
}

SDL_Rect Window::getWindowSize()
{
    SDL_Rect rect = { 0 };
    if (g_WindowState) {
        SDL_GetWindowSize(g_WindowState, &rect.w, &rect.h);
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window not created.");
    }
    return rect;
}

SDL_Window* Window::getWindowHandle()
{
    return g_WindowState;
}

bool Window::destroyWindow()
{
    if (g_WindowState) {
        SDL_DestroyWindow(g_WindowState);
        g_WindowState = nullptr;
        return true;
	}
    return false;
}

bool Window::runMainLoop()
{
    if (!g_WindowState) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window not created.");
        return false;
    }


    // Single frame update for callback environment
    static uint64_t last = SDL_GetTicks();
    uint64_t now = SDL_GetTicks();
    double deltaTime = (double)(now - last) * (60.0 / 1000.0);
    last = now;
    if (deltaTime > 1.0f)
        deltaTime = 1.0f;

    Renderer::beginRender();
    if (g_WindowCallBackUpdate) {
        g_WindowCallBackUpdate((float)deltaTime);
    }
    if (g_WindowCallBackRender) {
        g_WindowCallBackRender((float)deltaTime);
    }
    Renderer::endRender();
    return true;
}

void Window::setWindowCallBackUpdate(WindowCallBack callBack)
{
    g_WindowCallBackUpdate = callBack;
}

void Window::setWindowCallBackRender(WindowCallBack callBack)
{
    g_WindowCallBackRender = callBack;
}