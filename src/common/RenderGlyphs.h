#pragma once

namespace RendererGlyphs {

    void DrawStrokeText(const std::string& text,
        float x, float y, float size,
        float thickness, float spacing,
        Renderer::Color color);

    SDL_FRect MeasureStrokeTextRect(float x, float y,
        const std::string& text,
        float size,
        float thickness,
        float spacing);
}
