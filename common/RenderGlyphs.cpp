#include "Common.h"


static inline float GLYPH_WIDTH() { return 0.60f; }
struct Seg { float x1, y1, x2, y2; };
using Strokes = std::vector<Seg>;

struct TextMetrics {
    float width;   // total horizontal extent
    float height;  // vertical extent
    float ascent;  // above baseline
    float descent; // below baseline
};

static inline const std::unordered_map<char, Strokes>& StrokeAlphabet()
{
    static std::unordered_map<char, Strokes> map;
    if (!map.empty()) return map;

    const float w = GLYPH_WIDTH();
    const float x0 = (1.0f - w) * 0.5f;
    const float xL = x0;
    const float xR = x0 + w;
    const float xM = x0 + w * 0.5f;
    const float top = 0.05f;
    const float mid = 0.50f;
    const float bot = 0.95f;

    auto Hbar = [&](float y) { return Seg{ xL, y, xR, y }; };

    // Letters (subset, extend as needed)
    map['A'] = { {xL, bot, xM, top}, {xR, bot, xM, top}, Hbar(mid) };
    map['B'] = { {xL, top, xL, bot}, {xL, top, xR, top}, {xR, top, xR, mid}, {xR, mid, xL, mid}, {xL, mid, xR, mid}, {xR, mid, xR, bot}, {xR, bot, xL, bot} };
    map['C'] = { {xR, top, xL, top}, {xL, top, xL, bot}, {xL, bot, xR, bot} };
    map['D'] = { {xL, top, xL, bot}, {xL, top, xR, mid}, {xR, mid, xL, bot} };
    map['E'] = { {xR, top, xL, top}, {xL, top, xL, bot}, {xL, mid, xR, mid}, {xL, bot, xR, bot} };
    map['F'] = { {xL, top, xL, bot}, {xL, top, xR, top}, {xL, mid, xR * 0.9f, mid} };
    map['H'] = { {xL, top, xL, bot}, {xR, top, xR, bot}, Hbar(mid) };
    map['J'] = { {xR, top, xR, bot * 0.8f}, {xR, bot, xL, bot}, {xL, bot, xL, bot * 0.8f} };
    map['L'] = { {xL, top, xL, bot}, {xL, bot, xR, bot} };
    map['M'] = { {xL, bot, xL, top}, {xL, top, xM, mid}, {xM, mid, xR, top}, {xR, top, xR, bot} };
    map['N'] = { {xL, bot, xL, top}, {xL, top, xR, bot}, {xR, bot, xR, top} };
    map['O'] = { {xL, top, xR, top}, {xR, top, xR, bot}, {xR, bot, xL, bot}, {xL, bot, xL, top} };
    map['P'] = { {xL, bot, xL, top}, {xL, top, xR, top}, {xR, top, xR, mid}, {xR, mid, xL, mid} };
    map['Q'] = { {xL, top, xR, top}, {xR, top, xR, bot}, {xR, bot, xL, bot}, {xL, bot, xL, top}, {xM, mid, xR, bot} };
    map['R'] = { {xL, bot, xL, top}, {xL, top, xR, top}, {xR, top, xR, mid}, {xR, mid, xL, mid}, {xL, mid, xR, bot} };
    map['S'] = { {xR, top, xL, top}, {xL, top, xL, mid}, {xL, mid, xR, mid}, {xR, mid, xR, bot}, {xR, bot, xL, bot} };
    map['T'] = { {xL, top, xR, top}, {xM, top, xM, bot} };
    map['U'] = { {xL, top, xL, bot}, {xL, bot, xR, bot}, {xR, bot, xR, top} };
    map['V'] = { {xL, top, xM, bot}, {xM, bot, xR, top} };
    map['W'] = { {xL, top, xL, bot}, {xL, bot, xM, top}, {xM, top, xR, bot}, {xR, bot, xR, top} };
    map['X'] = { {xL, top, xR, bot}, {xR, top, xL, bot} };
    map['Y'] = { {xL, top, xM, mid}, {xR, top, xM, mid}, {xM, mid, xM, bot} };
    map['Z'] = { {xL, top, xR, top}, {xR, top, xL, bot}, {xL, bot, xR, bot} };
    map['G'] = { {xR, top, xL, top}, {xL, top, xL, bot}, {xL, bot, xR, bot}, {xR, bot, xR, mid}, {xR, mid, xM, mid} };
    map['I'] = { {xM, top, xM, bot}, {xL, top, xR, top}, {xL, bot, xR, bot} };
    map['K'] = { {xL, top, xL, bot}, {xL, mid, xR, top}, {xL, mid, xR, bot} };
    map[' '] = {};

    // Digits
    map['0'] = { {xL, top, xR, top}, {xR, top, xR, bot}, {xR, bot, xL, bot}, {xL, bot, xL, top} };
    map['1'] = { {xM, top, xM, bot}, {xL, bot, xR, bot} };
    map['2'] = { {xL, mid * 0.2f, xR, top}, {xR, top, xR, mid}, {xR, mid, xL, mid}, {xL, mid, xL, bot}, {xL, bot, xR, bot} };
    map['3'] = { {xL, top, xR, top}, {xR, top, xR, mid}, {xR, mid, xL * 0.9f, mid}, {xR, mid, xR, bot}, {xR, bot, xL, bot} };
    map['4'] = { {xL, top, xL, mid}, {xL, mid, xR, mid}, {xR, top, xR, bot} };
    map['5'] = { {xR, top, xL, top}, {xL, top, xL, mid}, {xL, mid, xR, mid}, {xR, mid, xR, bot}, {xR, bot, xL, bot} };
    map['6'] = { {xR, top, xL, top}, {xL, top, xL, bot}, {xL, bot, xR, bot}, {xR, bot, xR, mid}, {xR, mid, xL, mid} };
    map['7'] = { {xL, top, xR, top}, {xR, top, xL, bot} };
    map['8'] = { {xL, top, xR, top}, {xR, top, xR, bot}, {xR, bot, xL, bot}, {xL, bot, xL, top}, {xL, mid, xR, mid} };
    map['9'] = { {xR, bot, xR, top}, {xR, top, xL, top}, {xL, top, xL, mid}, {xL, mid, xR, mid} };

    // Punctuation
    // Comma: small vertical stroke near bottom-left + small diagonal tail
    map[','] = { {xM, bot - 0.1f, xM, bot}, {xM, bot, xM - 0.1f, bot + 0.1f} };
    // Period: tiny point at bottom-center
  
    map['.'] = { {xM, bot - 0.05f, xM, bot} };
    map[';'] = { {xM, mid, xM, mid + 0.15f}, {xM, bot - 0.05f, xM, bot} };
    map['-'] = { {xL, mid, xR, mid} };
    map['='] = { {xL, mid - 0.1f, xR, mid - 0.1f}, {xL, mid + 0.1f, xR, mid + 0.1f} };
    map['!'] = { {xM, top, xM, mid}, {xM, bot - 0.05f, xM, bot} };
    map['@'] = { {xL, mid, xR, mid}, {xR, mid, xR, bot}, {xR, bot, xL, bot}, {xL, bot, xL, mid}, {xM, mid, xM, bot * 0.9f} };
    map['#'] = { {xL, mid - 0.2f, xR, mid - 0.2f}, {xL, mid + 0.2f, xR, mid + 0.2f}, {xM - 0.1f, top, xM - 0.1f, bot}, {xM + 0.1f, top, xM + 0.1f, bot} };
    map['$'] = { {xM, top, xM, bot}, {xR, top, xL, top}, {xL, mid, xR, mid}, {xR, bot, xL, bot} };
    map['%'] = { {xL, bot, xR, top}, {xL, top, xL, top + 0.05f}, {xR, bot, xR, bot - 0.05f} };
    map['^'] = { {xL, mid, xM, top}, {xM, top, xR, mid} };
    map['&'] = { {xR, top, xL, mid}, {xL, mid, xR, bot}, {xR, bot, xM, mid}, {xM, mid, xL, bot} };
    map['*'] = { {xM, top, xM, bot}, {xL, mid, xR, mid}, {xL, top, xR, bot}, {xR, top, xL, bot} };
    map['('] = { {xR, top, xL, mid}, {xL, mid, xR, bot} };
    map[')'] = { {xL, top, xR, mid}, {xR, mid, xL, bot} };

    return map;
}

 void DrawStrokeGlyph(char ch,
    float ox, float oy, float size,
    float thickness, Renderer::Color color)
{
    const auto& dict = StrokeAlphabet();
    char key = (char)SDL_toupper((unsigned char)ch);
    auto it = dict.find(key);
    if (it == dict.end()) return;
    for (const auto& s : it->second) {
        Renderer::drawThickLine(ox + s.x1 * size, oy + s.y1 * size,
            ox + s.x2 * size, oy + s.y2 * size,
            thickness, color);
    }
}

void RendererGlyphs::DrawStrokeText(const std::string& text,
    float x, float y, float size,
    float thickness, float spacing,
    Renderer::Color color)
{
    float advance = size * GLYPH_WIDTH() + spacing;
    float penX = x;
    for (char c : text) {
        DrawStrokeGlyph(c, penX, y, size, thickness, color);
        penX += advance;
    }
}

static inline TextMetrics MeasureStrokeText(const std::string& text,
    float size,
    float thickness,
    float spacing)
{
    // Horizontal: we advance by (glyphW + spacing) per char,
    // but there is no trailing spacing after the final glyph.
    const size_t n = text.size();
    const float glyphW = size * GLYPH_WIDTH();
    const float advance = glyphW + spacing;
    const float width = n ? (advance * (float)n - spacing) : 0.0f; // n*advance - spacing

    // Vertical: normalized glyph box is [top=0.05 .. bot=0.95].
    // Lines are centered on stroke, so add half thickness to top and bottom.
    const float topPad = 0.05f;
    const float botPad = 0.95f;
    const float ascent = botPad * size + 0.5f * thickness;          // above baseline (y origin)
    const float descent = (1.0f - botPad) * size + 0.5f * thickness;  // below baseline
    const float height = ascent + descent; // equals size + thickness

    return { width, height, ascent, descent };
}

SDL_FRect RendererGlyphs::MeasureStrokeTextRect(float x, float y,
    const std::string& text,
    float size,
    float thickness,
    float spacing)
{
    TextMetrics m = MeasureStrokeText(text, size, thickness, spacing);
    SDL_FRect r{ x, y - m.ascent, m.width, m.height }; // y is treated as baseline
    return r;

}