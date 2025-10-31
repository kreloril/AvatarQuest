#pragma once
namespace  Renderer
{
	struct AABB {
		Vector2 position;
		Vector2 size;
	public:
		AABB() : position(0, 0), size(0, 0) {}
		AABB(const Vector2& pos, const Vector2& sz) : position(pos), size(sz) {}
		Vector2 getPosition() const { return position; }
		Vector2 getSize() const { return size; }
		void setPosition(Vector2 pos) { position = pos; }
		void setSize(Vector2 sz) { size = sz; }
		bool intersects(const AABB& other) const {
			return !(position.x + size.x < other.position.x ||
				position.x > other.position.x + other.size.x ||
				position.y + size.y < other.position.y ||
				position.y > other.position.y + other.size.y);
		}
		float distanceTo(const AABB& other) const {
			float dx = std::max(position.x, other.position.x) - std::min(position.x + size.x, other.position.x + other.size.x);
			float dy = std::max(position.y, other.position.y) - std::min(position.y + size.y, other.position.y + other.size.y);
			return std::sqrt(dx * dx + dy * dy);
		}
	};

	struct Color {
		uint8_t r = 0, g = 0, b = 0, a = 255;

	};

	struct PrimitiveModel {
		Vector<Vector2> points;
		Vector<int> indices;
	};

	bool initRenderer(int width, int height);
	bool shutDownRenderer();
	bool drawFilledRect(float x, float y, float width, float height, Color color);
	bool drawRect(float x, float y, float width, float height, Color color);
	void drawThickLine(float x1, float y1, float x2, float y2, float thickness, Color color);
	void drawImage(Vector2 position, Vector2 scale, float rotation, Ref<Image> img, Color tint = {255,255,255,255});
	void drawImageFromRect(Vector2 position, Vector2 scale, float rotation, Ref<Image> img, const SDL_FRect& tileRect, Color tint = { 255,255,255,255 });
	void beginRender();
	void endRender();
	void setClearColor(Color color);
	// Set the renderer's draw blend mode (e.g., SDL_BLENDMODE_NONE, _BLEND, _ADD, _MOD, _MUL)
	void setBlendMode(SDL_BlendMode mode);
	void drawPrimitiveList(const Vector<Vector2>& points, const Vector<int>& indices, Color color, float thickness = 1.0f);
	SDL_Renderer* getRenderer();
};

