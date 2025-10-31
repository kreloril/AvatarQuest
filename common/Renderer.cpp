#include "Common.h"
#include <algorithm>
#include <array>


#define MAX_RENDER_VERTICES 32768*8
struct RenderState {
    int width;
    int height;
    Renderer::Color clearColor = { 0,0,0,255 };
    SDL_Renderer* sdlRenderer = nullptr;
};


static RenderState g_RenderState;

void RenderThickLineAlt(SDL_Renderer* renderer,
    float x1, float y1,
    float x2, float y2,
    float thickness,
    SDL_Color color)
{
    // Direction vector of the line
    float dx = x2 - x1;
    float dy = y2 - y1;
    float len = sqrtf(dx * dx + dy * dy);
    if (len <= 0.0001f) return;

    // Normalize direction
    dx /= len;
    dy /= len;

    // Perpendicular vector (scaled by half thickness)
    float px = -dy * (thickness * 0.5f);
    float py = dx * (thickness * 0.5f);

    SDL_FColor lineColor = { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f };

    SDL_Vertex verts[4] = {
        { {x1 + px, y1 + py}, lineColor, {0,0} },
        { {x2 + px, y2 + py}, lineColor, {1,0} },
        { {x2 - px, y2 - py}, lineColor, {1,1} },
        { {x1 - px, y1 - py}, lineColor, {0,1} },
    };
    int indices[6] = { 0,1,2, 2,3,0 };

    SDL_RenderGeometry(renderer, NULL, verts, 4, indices, 6);
}

// Immediate rendering, no command batching.

bool Renderer::initRenderer(int width, int height)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != true)
    {
        std::cerr << "SDL could not initialize! SDL_Error: "
            << SDL_GetError() << std::endl;
        return 1;
    }

    if (!Window::createWindow(width, height, "Farm Stead Harmony")) {
        std::cerr << "Window could not be created! SDL_Error: "
            << SDL_GetError() << std::endl;
		return false;
    }

	SDL_Window* window = (SDL_Window*)Window::getWindowHandle();
    if (!window) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window not created.");
        return false;
    }

    g_RenderState.sdlRenderer = SDL_CreateRenderer(window, nullptr);
    if (g_RenderState.sdlRenderer == nullptr)
    {
        std::cerr << "Renderer could not be created! SDL_Error: "
            << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_GetRenderOutputSize(g_RenderState.sdlRenderer, &g_RenderState.width, &g_RenderState.height);

    // Enable standard alpha blending for immediate draw calls
    SDL_SetRenderDrawBlendMode(g_RenderState.sdlRenderer, SDL_BLENDMODE_BLEND);

    // Initialize SDL_ttf text subsystem
    (void)Text::init();

    return true;
}

bool Renderer::shutDownRenderer()
{
    // Shutdown text subsystem before renderer destruction
    Text::shutdown();
    if (g_RenderState.sdlRenderer) {
        SDL_DestroyRenderer(g_RenderState.sdlRenderer);
        g_RenderState.sdlRenderer = nullptr;
        return false;
	}
     return true;
}

bool Renderer::drawFilledRect(float x, float y, float width, float height, Color color)
{
    if (!g_RenderState.sdlRenderer) return false;
    SDL_SetRenderDrawColor(g_RenderState.sdlRenderer, color.r, color.g, color.b, color.a);
    SDL_FRect r{ x, y, width, height };
    SDL_RenderFillRect(g_RenderState.sdlRenderer, &r);
    return true;
}

void Renderer::drawThickLine(float x1, float y1, float x2, float y2, float thickness, Color color)
{
    // Immediate thick line draw
    SDL_Color c{ color.r, color.g, color.b, color.a };
    RenderThickLineAlt(g_RenderState.sdlRenderer, x1, y1, x2, y2, thickness, c);
    return;
}

void Renderer::drawImage(Vector2 position, Vector2 scale, float rotation, Ref<Image> img, Color tint)
{
    if (!img || !img->texture) return;
    SDL_FRect destRect;
    destRect.w = img->imageRect.w * scale.x;
    destRect.h = img->imageRect.h * scale.y;
    destRect.x = position.x - destRect.w * 0.5f; // Centered
    destRect.y = position.y - destRect.h * 0.5f; // Centered
    SDL_SetTextureColorMod(img->texture, tint.r, tint.g, tint.b);
    SDL_SetTextureAlphaMod(img->texture, tint.a);
	SDL_FPoint center = { (img->imageRect.w * 0.5f), (img->imageRect.h * 0.5f) };
	SDL_RenderTextureRotated(g_RenderState.sdlRenderer, img->texture, &img->imageRect, &destRect, rotation, &center, SDL_FLIP_NONE);
    //SDL_RenderCopyExF(g_RenderState.sdlRenderer, img->texture, &img->imageRect, &destRect, rotation, nullptr, SDL_FLIP_NONE);
}

void Renderer::drawImageFromRect(Vector2 position, Vector2 scale, float rotation, Ref<Image> img, const SDL_FRect& tileRect, Color tint)
{
    if (!img || !img->texture) return;
    SDL_FRect destRect;
    destRect.w = tileRect.w * scale.x;
    destRect.h = tileRect.h * scale.y;
    destRect.x = position.x - destRect.w * 0.5f; // Centered
    destRect.y = position.y - destRect.h * 0.5f; // Centered
    SDL_SetTextureColorMod(img->texture, tint.r, tint.g, tint.b);
    SDL_SetTextureAlphaMod(img->texture, tint.a);
    SDL_FPoint center = { (tileRect.w * 0.5f), (tileRect.h * 0.5f) };
    SDL_RenderTextureRotated(g_RenderState.sdlRenderer, img->texture, &tileRect, &destRect, rotation, &center, SDL_FLIP_NONE);
}

bool Renderer::drawRect(float x, float y, float width, float height, Color color)
{
    if (!g_RenderState.sdlRenderer) return false;
    SDL_SetRenderDrawColor(g_RenderState.sdlRenderer, color.r, color.g, color.b, color.a);
    SDL_FRect r{ x, y, width, height };
    SDL_RenderRect(g_RenderState.sdlRenderer, &r);
    return true;
}
void Renderer::beginRender()
{
	// clear the renderer with the clear color
    SDL_SetRenderDrawColor(g_RenderState.sdlRenderer, g_RenderState.clearColor.r, g_RenderState.clearColor.g, g_RenderState.clearColor.b, g_RenderState.clearColor.a);
    SDL_RenderClear(g_RenderState.sdlRenderer);
}

void Renderer::endRender()
{
    SDL_RenderPresent(g_RenderState.sdlRenderer);
}

void Renderer::setClearColor(Color color)
{
    g_RenderState.clearColor = color;
}

void Renderer::setBlendMode(SDL_BlendMode mode)
{
    if (!g_RenderState.sdlRenderer) return;
    SDL_SetRenderDrawBlendMode(g_RenderState.sdlRenderer, mode);
}

void Renderer::drawPrimitiveList(const Vector<Vector2>& points, const Vector<int>& indices, Color color, float thickness)
{
    if (points.size() >= MAX_RENDER_VERTICES) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Too many vertices to render.");
        return;
    }
    if (indices.size() >= MAX_RENDER_VERTICES) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Too many indices to render.");
        return;
    }
    SDL_SetRenderDrawColor(g_RenderState.sdlRenderer, color.r, color.g, color.b, color.a);
    // Convert points to SDL_FPoint
  
    // Draw lines based on indices
    for (size_t i = 0; i < indices.size(); i += 2) {
        if (i + 1 < indices.size()) { 
            int idx1 = indices[i];
            int idx2 = indices[i + 1];
            if (idx1 < points.size() && idx2 < points.size()) {
                drawThickLine(
                    points[idx1].x, points[idx1].y,
                    points[idx2].x, points[idx2].y,
                    thickness,
                    color);
            }
        }
	}
}



SDL_Renderer* Renderer::getRenderer()
{
    return g_RenderState.sdlRenderer;
}


