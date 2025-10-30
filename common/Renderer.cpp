#include "Common.h"
#include <algorithm>
#include <array>


#define MAX_RENDER_COMMANDS 32768*8
#define MAX_RENDER_VERTICES 32768*8
struct RenderCommands {
    enum CommandType {
        DRAW_FILLED_RECT,
        DRAW_RECT,
		DRAW_LINE,
        DRAW_VERTEXS // For future use, e.g., for drawing lines
        // Add more command types as needed
	};

	CommandType type;
	SDL_FRect rect; 
	SDL_Color color; // Color for the rectangle
	float lineWidth = 1.0f; // For line commands, if needed

};


struct RenderState {
    int width;
    int height;
    Uint32 commandCount = 0;
    Renderer::Color clearColor = { 0,0,0,255 };
    SDL_Renderer* sdlRenderer = nullptr;
    UMap<uint32_t, Vector<Uint32>> commandMap;
    Array<RenderCommands, MAX_RENDER_COMMANDS> renderCommandCache;

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

inline uint32_t colorToHash(const SDL_Color& color) {
    return (static_cast<uint32_t>(color.r) << 24) |
        (static_cast<uint32_t>(color.g) << 16) |
        (static_cast<uint32_t>(color.b) << 8) |
        (static_cast<uint32_t>(color.a));
}

inline SDL_Color hashToColor(uint32_t hash) {
    SDL_Color color;
    color.r = (hash >> 24) & 0xFF;
    color.g = (hash >> 16) & 0xFF;
    color.b = (hash >> 8) & 0xFF;
    color.a = hash & 0xFF;
    return color;
}

void insertCommandIntoMap(SDL_Color& color, Uint32 index) {
	auto hash = colorToHash(color);
    g_RenderState.commandMap[hash].push_back(index);
}

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


    return true;
}

bool Renderer::shutDownRenderer()
{
    if (g_RenderState.sdlRenderer) {
        SDL_DestroyRenderer(g_RenderState.sdlRenderer);
        g_RenderState.sdlRenderer = nullptr;
        return false;
	}

    g_RenderState.commandMap.clear();

    return true;
}

bool Renderer::drawFilledRect(float x, float y, float width, float height, Color color)
{

    if (g_RenderState.commandCount + 1  >= MAX_RENDER_COMMANDS) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "render command queue full.");
        return false;
    }


    g_RenderState.renderCommandCache[g_RenderState.commandCount].type = RenderCommands::DRAW_FILLED_RECT;
    g_RenderState.renderCommandCache[g_RenderState.commandCount].rect = { (float)x, (float)y, (float)width, (float)height };
    g_RenderState.renderCommandCache[g_RenderState.commandCount].color = { (Uint8)color.r, (Uint8)color.g, (Uint8)color.b, (Uint8)color.a };
   
	// Insert the command into the map for deduplication
	insertCommandIntoMap(g_RenderState.renderCommandCache[g_RenderState.commandCount].color, g_RenderState.commandCount);
    
    g_RenderState.commandCount++;

   

    return true;
}

void Renderer::drawThickLine(float x1, float y1, float x2, float y2, float thickness, Color color)
{
    if (g_RenderState.commandCount + 1 >= MAX_RENDER_COMMANDS) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "render command queue full.");
        return;
    }
    g_RenderState.renderCommandCache[g_RenderState.commandCount].type = RenderCommands::DRAW_LINE;
    g_RenderState.renderCommandCache[g_RenderState.commandCount].rect = { x1, y1, x2, y2};
    g_RenderState.renderCommandCache[g_RenderState.commandCount].color = { (Uint8)color.r, (Uint8)color.g, (Uint8)color.b, (Uint8)color.a };
    g_RenderState.renderCommandCache[g_RenderState.commandCount].lineWidth = thickness;
    // Insert the command into the map for deduplication
    insertCommandIntoMap(g_RenderState.renderCommandCache[g_RenderState.commandCount].color, g_RenderState.commandCount);
    g_RenderState.commandCount++;
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
    if (g_RenderState.commandCount + 1 >= MAX_RENDER_COMMANDS) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "render command queue full.");
        return false;
    }


    g_RenderState.renderCommandCache[g_RenderState.commandCount].type = RenderCommands::DRAW_RECT;
    g_RenderState.renderCommandCache[g_RenderState.commandCount].rect = { (float)x, (float)y, (float)width, (float)height };
    g_RenderState.renderCommandCache[g_RenderState.commandCount].color = { (Uint8)color.r, (Uint8)color.g, (Uint8)color.b, (Uint8)color.a };

    // Insert the command into the map for deduplication
    insertCommandIntoMap(g_RenderState.renderCommandCache[g_RenderState.commandCount].color, g_RenderState.commandCount);

    g_RenderState.commandCount++;
    return true;
}

void sortRenderCommands(ListRef<RenderCommands>& commands)
{
    // Sort commands based on type or any other criteria if needed
    // For now, we will keep the order as is
	// You can implement a custom sorting logic here if needed
 //   std::sort(commands.begin(), commands.end(), []( Ref<RenderCommands>& a, Ref<RenderCommands>& b) {
   //     return false; // Example sorting by type
	//	});
}
void Renderer::beginRender()
{
	// clear the renderer with the clear color
    SDL_SetRenderDrawColor(g_RenderState.sdlRenderer,g_RenderState.clearColor.r,g_RenderState.clearColor.g,g_RenderState.clearColor.b,g_RenderState.clearColor.a);
    SDL_RenderClear(g_RenderState.sdlRenderer);
	g_RenderState.commandCount = 0;
    for (auto& [_, indices] : g_RenderState.commandMap) {
        indices.clear();  // keeps capacity
    }

}

void Renderer::endRender()
{
	
    for (auto& [color,list] : g_RenderState.commandMap) {
        auto hashColor = hashToColor(color);
        SDL_SetRenderDrawColor(g_RenderState.sdlRenderer, hashColor.r, hashColor.g, hashColor.b, hashColor.a);
        
        Vector<SDL_FRect> rectsFilled;
		Vector<SDL_FRect> rectsOutline;

        for (auto index : list) {
            auto& command = g_RenderState.renderCommandCache[index];

            switch (command.type) {
            case RenderCommands::DRAW_FILLED_RECT:
                // Add filled rectangle to the list
                rectsFilled.push_back(command.rect);
                break;
            case RenderCommands::DRAW_RECT:
                // Add outlined rectangle to the list
                rectsOutline.push_back(command.rect);
                break;

			case RenderCommands::DRAW_LINE:
				// Draw thick line
				RenderThickLineAlt(g_RenderState.sdlRenderer, command.rect.x, command.rect.y, command.rect.w, command.rect.h, command.lineWidth, hashColor);
				break;
            }
        }
        // Draw filled rectangles
        if (!rectsFilled.empty()) {
            SDL_RenderFillRects(g_RenderState.sdlRenderer, rectsFilled.data(), rectsFilled.size());
        }
        // Draw outlined rectangles
        if (!rectsOutline.empty()) {
            SDL_RenderRects(g_RenderState.sdlRenderer, rectsOutline.data(), rectsOutline.size());
		}
	}


   
	SDL_RenderPresent(g_RenderState.sdlRenderer);

}

void Renderer::setClearColor(Color color)
{
    g_RenderState.clearColor = color;
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


