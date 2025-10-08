#include "Common.h"
#include "Common.h"


inline bool HasFrameFlag(Renderer::AnimFrameFlags f, Renderer::AnimFrameFlags bit) {
    return (static_cast<uint32_t>(f) & static_cast<uint32_t>(bit)) != 0u;
}



inline bool HasBankFlag(Renderer::AnimBankFlags f, Renderer::AnimBankFlags bit) {
	return (static_cast<uint32_t>(f) & static_cast<uint32_t>(bit)) != 0u;
}

inline void SetBankFlag(Renderer::AnimBankFlags& m, Renderer::AnimBankFlags f) {
	using U = std::underlying_type_t<Renderer::AnimBankFlags>;
	m = static_cast<Renderer::AnimBankFlags>(static_cast<U>(m) | static_cast<U>(f));
}

inline void ClearBankFlag(Renderer::AnimBankFlags& m, Renderer::AnimBankFlags f) {
	using U = std::underlying_type_t<Renderer::AnimBankFlags>;
	m = static_cast<Renderer::AnimBankFlags>(static_cast<U>(m) & ~static_cast<U>(f));
}

static inline float clamp01(float v) { return v < 0.f ? 0.f : (v > 1.f ? 1.f : v); }
static inline float lerpf(float a, float b, float t) { return a + (b - a) * t; }
static inline Vector2 vlerp(const Vector2& a, const Vector2& b, float t) {
	return { lerpf(a.x,b.x,t), lerpf(a.y,b.y,t) };
}
static inline Vector2 rotateAround(const Vector2& p, const Vector2& pivot, float deg) {
	const float r = deg * 0.01745329251994329577f;
	const float s = std::sin(r), c = std::cos(r);
	const float x = p.x - pivot.x, y = p.y - pivot.y;
	return { pivot.x + x * c - y * s, pivot.y + x * s + y * c };
}
static inline Vector2 scaleAround(const Vector2& p, const Vector2& pivot, const Vector2& s) {
	return { pivot.x + (p.x - pivot.x) * s.x,
			 pivot.y + (p.y - pivot.y) * s.y };
}

inline void SetFlag(Renderer::AnimBankFlags& m, Renderer::AnimBankFlags f) { m |= f; }
inline void ClearFlag(Renderer::AnimBankFlags& m, Renderer::AnimBankFlags f) { m &= ~f; }


Ref<Renderer::AnimPrimitive> Renderer::createAnimPrimitive(AnimPrimitiveType type, const Renderer::AABB& aabb, const Renderer::Color& color, float thickness,int layer)
{
	
	Ref<AnimPrimitive> primitive = CreateRef<AnimPrimitive>();
	primitive->type = type;
	primitive->aabb = aabb;
	primitive->color = color;
	primitive->center = Vector2(aabb.getPosition().x + aabb.getSize().x / 2, aabb.getPosition().y + aabb.getSize().y / 2);
	primitive->thickness = thickness;
	primitive->layer = layer;
	return primitive;
}

Vector<Renderer::AnimPrimitiveRef> Renderer::createAnimPrimitiveFromPrimitiveModel(const Renderer::PrimitiveModel& model, const Renderer::Color& color, float thickness, int layer)
{

	Vector<AnimPrimitiveRef> primitives;

    for (int i = 0; i < model.indices.size(); i += 2) {
        if (i + 1 < model.indices.size()) {
            int idx1 = model.indices[i];
            int idx2 = model.indices[i + 1];
            if (idx1 >= 0 && idx1 < model.points.size() && idx2 >= 0 && idx2 < model.points.size()) {
                const Vector2& p1 = model.points[idx1];
                const Vector2& p2 = model.points[idx2];
                Renderer::AABB aabb;
                aabb.setPosition({ std::min(p1.x, p2.x), std::min(p1.y, p2.y) });
                aabb.setSize({ std::fabs(p2.x - p1.x), std::fabs(p2.y - p1.y) });
                AnimPrimitiveRef prim = createAnimPrimitive(AnimPrimitiveType::Line, aabb, color, thickness, layer);
                primitives.push_back(prim);
            }
        }
	}

    return primitives;
}

Renderer::AnimationFrameRef Renderer::createAnimationFrame(const String& name, AnimFrameFlags flags, Vector<AnimPrimitiveRef>& anims, float duration, Vector2 posStart, Vector2 posEnd, Vector2 rotationStartEnd, Vector2 scaleStart, Vector2 scaleEnd)
{
	Renderer::AnimationFrameRef frame = CreateRef<Renderer::AnimationFrame>();
	frame->name = name;
	frame->flags = flags;
	frame->duration = duration;
	frame->positionStart = posStart;
	frame->positionEnd = posEnd;
	frame->rotationStartEnd = rotationStartEnd;
	frame->scaleStart = scaleStart;
	frame->scaleEnd = scaleEnd;

    std::stable_sort(anims.begin(), anims.end(), [](const AnimPrimitiveRef& a, const AnimPrimitiveRef& b) {
		if (!a) return false; // Handle null references
		if (!b) return false; // Handle null references
        return a->layer < a->layer; // Sort by layer for rendering order
		});

	frame->primitives = anims;

	return frame;
}




Renderer::AnimationBankRef Renderer::createAnimationBank(const String& name, AnimBankFlags flags, Vector<AnimationFrameRef>& frames, Uint32 currentFrame, float currentTime)
{
	
	Renderer::AnimationBankRef bank = CreateRef<Renderer::AnimationBank>();
	bank->flags = flags;
	bank->name = name;
	bank->frames = frames;
	bank->currentFrame = currentFrame;
	bank->currentTime = std::min<Uint32>(currentFrame, frames.empty() ? 0u : (Uint32)frames.size() - 1);
	return bank;
}

Renderer::AnimationUpdateState Renderer::updateAnimationBank(AnimationBankRef& bank, float deltaTime)
{
    if (!HasBankFlag(bank->flags, AnimBankFlags::Started) && bank->currentFrame == 0) {
        SetBankFlag(bank->flags, AnimBankFlags::Started);
        ClearBankFlag(bank->flags, AnimBankFlags::Ended);
        if (bank->onBankStart) bank->onBankStart(bank);
    }

    const uint32_t n = (uint32_t)bank->frames.size();
    uint32_t idx = bank->currentFrame;
    auto cur = bank->frames[idx];
    if (!cur) return AnimationUpdateState::Stopped;

    if (cur->duration <= 0.f) cur->duration = 0.0001f;
    bank->currentTime += deltaTime;

    // Step across frames if time overflows
    while (bank->currentTime >= cur->duration) {
        bank->currentTime -= cur->duration;

        if (idx + 1 < n) {
            ++idx;
            cur = bank->frames[idx];
            if (!cur) return AnimationUpdateState::Stopped;
            if (cur->duration <= 0.f) cur->duration = 0.0001f;
        }
        else {
            // reached last frame end
            if (HasBankFlag(bank->flags, AnimBankFlags::Repeat)) {
                // end-of-cycle
                if (!HasBankFlag(bank->flags, AnimBankFlags::Ended)) {
                    SetFlag(bank->flags, AnimBankFlags::Ended);
                    if (bank->onBankEnd) bank->onBankEnd(bank);
                }
                // wrap
                idx = 0;
                cur = bank->frames[idx];
                if (!cur) return AnimationUpdateState::Stopped;
                if (cur->duration <= 0.f) cur->duration = 0.0001f;

                // re-arm start for next cycle
                ClearFlag(bank->flags, AnimBankFlags::Started);
                // continue loop to consume any leftover time
            }
            else {
                // clamp at end
                bank->currentFrame = n - 1;
                bank->currentTime = cur->duration;
                if (!HasBankFlag(bank->flags, AnimBankFlags::Ended)) {
                    SetFlag(bank->flags, AnimBankFlags::Ended);
                    if (bank->onBankEnd) bank->onBankEnd(bank);
                }
                return AnimationUpdateState::Finished;
            }
        }
    }

    bank->currentFrame = idx;
    return AnimationUpdateState::Playing;
}

void Renderer::RenderAnimationBank(AnimationBankRef& bank, const Vector2& worldPos, const Vector2& worldScale, float worldRotDeg)
{

    if (!bank || bank->frames.empty()) return;
    auto fr = bank->frames[bank->currentFrame]; if (!fr) return;

    const float dur = fr->duration > 0.f ? fr->duration : 0.0001f;
    const float t = clamp01(bank->currentTime / dur);

    const bool doMove = HasFrameFlag(fr->flags, AnimFrameFlags::Move);
    const bool doRot = HasFrameFlag(fr->flags, AnimFrameFlags::Rotate);
    const bool doScl = HasFrameFlag(fr->flags, AnimFrameFlags::Scale);

    const Vector2 tPos = doMove ? vlerp(fr->positionStart, fr->positionEnd, t)
        : fr->positionStart;
    const float   tRot = doRot ? lerpf(fr->rotationStartEnd.x, fr->rotationStartEnd.y, t)
        : fr->rotationStartEnd.x;
    const Vector2 tScl = doScl ? vlerp(fr->scaleStart, fr->scaleEnd, t)
        : fr->scaleStart;

    const float totalRot = worldRotDeg + tRot;
    const Vector2 totalScl{ worldScale.x * tScl.x, worldScale.y * tScl.y };

    auto applyAll = [&](const Vector2& p)->Vector2 {
        Vector2 s{ p.x * totalScl.x, p.y * totalScl.y };
        Vector2 r = rotateAround(s, { 0,0 }, totalRot);
        return { r.x + worldPos.x + tPos.x, r.y + worldPos.y + tPos.y };
        };

   

    for (const auto& pRef : fr->primitives) {
		if (!pRef) continue; // Skip null references
        const AnimPrimitive& p = *pRef;
        const Vector2 P = p.aabb.getPosition();
        const Vector2 S = p.aabb.getSize();

        if (p.type == AnimPrimitiveType::Line) {
            Vector2 s{ P.x,       P.y };
            Vector2 e{ P.x + S.x,   P.y + S.y };

            s = scaleAround(s, p.center, tScl);
            e = scaleAround(e, p.center, tScl);
            s = rotateAround(s, p.center, tRot);
            e = rotateAround(e, p.center, tRot);

            s = applyAll(s);
            e = applyAll(e);

            Renderer::drawThickLine(s.x, s.y, e.x, e.y, p.thickness, p.color);
        }
        else { // Rect
            Vector2 a{ P.x,       P.y };
            Vector2 b{ P.x + S.x,   P.y };
            Vector2 c{ P.x + S.x,   P.y + S.y };
            Vector2 d{ P.x,       P.y + S.y };

            if (p.type == AnimPrimitiveType::RectFilled) {
                Vector2 tl = { a.x + worldPos.x + tPos.x, a.y + worldPos.y + tPos.y };
                Renderer::drawFilledRect(tl.x, tl.y, S.x, S.y, p.color);
            }
            else {
                a = scaleAround(a, p.center, tScl); b = scaleAround(b, p.center, tScl);
                c = scaleAround(c, p.center, tScl); d = scaleAround(d, p.center, tScl);
                a = rotateAround(a, p.center, tRot); b = rotateAround(b, p.center, tRot);
                c = rotateAround(c, p.center, tRot); d = rotateAround(d, p.center, tRot);
                a = applyAll(a); b = applyAll(b); c = applyAll(c); d = applyAll(d);

                const float th = (p.thickness > 0.f) ? p.thickness : 1.0f;
                Renderer::drawThickLine(a.x, a.y, b.x, b.y, th, p.color);
                Renderer::drawThickLine(b.x, b.y, c.x, c.y, th, p.color);
                Renderer::drawThickLine(c.x, c.y, d.x, d.y, th, p.color);
                Renderer::drawThickLine(d.x, d.y, a.x, a.y, th, p.color);
            }
        }
    }

}

void Renderer::ResetAnimationBank(AnimationBankRef& bank, Uint32 toFrame, float startTime)
{
    if (!bank || bank->frames.empty()) return;
    bank->currentFrame = 0;
    bank->currentTime = 0.0f;
    ClearFlag(bank->flags, AnimBankFlags::Started);
    ClearFlag(bank->flags, AnimBankFlags::Ended);
}


