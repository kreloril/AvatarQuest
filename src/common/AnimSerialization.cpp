#include "Common.h"

using namespace Renderer;

namespace Util {

    void WritePrimitive(Util::BinStream& w, const Renderer::AnimPrimitive& p)
    {
        w.writeU32(static_cast<uint32_t>(p.type));
        writeVec2(w, p.aabb.getPosition());
        writeVec2(w, p.aabb.getSize());
        writeVec2(w, p.center);

        PackedColor c{ (uint8_t)p.color.r,(uint8_t)p.color.g,(uint8_t)p.color.b,(uint8_t)p.color.a };
        w.writeBytes(&c, sizeof(c));

        w.writeF32(p.thickness);
        w.writeI32((int32_t)p.layer);
    }

    bool ReadPrimitive(Util::BinStream& r, Renderer::AnimPrimitive& out)
    {
        uint32_t typeU = 0;
        Vector2 pos{}, size{}, center{};
        PackedColor c{};
        float thickness = 1.f; int32_t layer = 0;

        if (!r.readU32(typeU))        return false;
        if (!readVec2(r, pos))         return false;
        if (!readVec2(r, size))        return false;
        if (!readVec2(r, center))      return false;
        if (!r.readBytes(&c, sizeof(c))) return false;
        if (!r.readF32(thickness))    return false;
        if (!r.readI32(layer))        return false;

        out.type = (Renderer::AnimPrimitiveType)typeU;
        out.aabb.setPosition(pos);
        out.aabb.setSize(size);
        out.center = center;
        out.color = Renderer::Color{ c.r,c.g,c.b,c.a };
        out.thickness = thickness;
        out.layer = layer;
        return true;
    }

    void WriteFrame(Util::BinStream& w, const Renderer::AnimationFrame& fr)
    {
        w.writeStr(fr.name.c_str());
        w.writeF32(fr.duration);
        writeVec2(w, fr.positionStart);
        writeVec2(w, fr.positionEnd);
        writeVec2(w, fr.rotationStartEnd);
        writeVec2(w, fr.scaleStart);
        writeVec2(w, fr.scaleEnd);
        w.writeU32((uint32_t)fr.flags);

        const uint32_t n = (uint32_t)fr.primitives.size();
        w.writeU32(n);
        for (uint32_t i = 0; i < n; ++i) {
            const auto& ref = fr.primitives[i];
            const Renderer::AnimPrimitive& p = *ref;
            WritePrimitive(w, p);
        }
    }

    bool ReadFrame(Util::BinStream& r, Renderer::AnimationFrame& out, const std::function<Renderer::AnimPrimitiveRef(const Renderer::AnimPrimitive&)>& makeRef)
    {
        std::string name; float duration = 0.f;
        Vector2 p0{}, p1{}, rot{}, s0{}, s1{}; uint32_t flagsU = 0, n = 0;

        if (!r.readStr(name)) return false;
        if (!r.readF32(duration)) return false;
        if (!readVec2(r, p0) || !readVec2(r, p1) || !readVec2(r, rot) || !readVec2(r, s0) || !readVec2(r, s1)) return false;
        if (!r.readU32(flagsU)) return false;
        if (!r.readU32(n)) return false;

        out.name = name.c_str();
        out.duration = duration;
        out.positionStart = p0;
        out.positionEnd = p1;
        out.rotationStartEnd = rot;
        out.scaleStart = s0;
        out.scaleEnd = s1;
        out.flags = (Renderer::AnimFrameFlags)flagsU;

        out.primitives.clear();
        out.primitives.reserve(n);
        for (uint32_t i = 0; i < n; ++i) {
            Renderer::AnimPrimitive p{};
            if (!ReadPrimitive(r, p)) return false;
            out.primitives.push_back(makeRef(p));
        }
        return true;
    }

    void WriteBank(Util::BinStream& w, const AnimationBank& bank)
    {
        w.writeStr(bank.name.c_str());
        w.writeU32(static_cast<uint32_t>(bank.flags));
        w.writeU32(bank.currentFrame);
        w.writeF32(bank.currentTime);

        const uint32_t n = static_cast<uint32_t>(bank.frames.size());
        w.writeU32(n);
        for (uint32_t i = 0; i < n; ++i) {
            const AnimationFrame& fr = *bank.frames[i];
            Util::WriteFrame(w, fr);
        }
    }

    static inline uint32_t ClampIndex(uint32_t v, uint32_t count) {
        if (count == 0) return 0;
        return (v < count) ? v : (count - 1);
    }

    bool ReadBank(Util::BinStream& r,
        AnimationBank& out,
        const std::function<AnimPrimitiveRef(const AnimPrimitive&)>& makePrimRef,
        const std::function<AnimationFrameRef(const AnimationFrame&)>& makeFrameRef)
    {
        if (!makePrimRef) return false;

        std::string name;
        uint32_t flagsU = 0, curIdx = 0, count = 0;
        float curTime = 0.0f;

        if (!r.readStr(name))    return false;
        if (!r.readU32(flagsU))  return false;
        if (!r.readU32(curIdx))  return false;
        if (!r.readF32(curTime)) return false;
        if (!r.readU32(count))   return false;

        VectorRef<AnimationFrame> frames;
        frames.reserve(count);

        for (uint32_t i = 0; i < count; ++i) {
            AnimationFrame temp{};
            if (!Util::ReadFrame(r, temp, makePrimRef)) return false;

            AnimationFrameRef ref =
                makeFrameRef ? makeFrameRef(temp) : CreateRef<AnimationFrame>(temp);
            frames.push_back(ref);
        }

        out.name = name.c_str();
        out.flags = static_cast<AnimBankFlags>(flagsU);
        out.currentFrame = ClampIndex(curIdx, count);
        out.currentTime = curTime;
        out.frames = std::move(frames);
        // callbacks intentionally untouched (file doesn’t carry them)
        return true;
    }

    bool ReadBank(Util::BinStream& r,
        AnimationBankRef& outRef,
        const std::function<AnimPrimitiveRef(const AnimPrimitive&)>& makePrimRef,
        const std::function<AnimationFrameRef(const AnimationFrame&)>& makeFrameRef)
    {
        AnimationBank temp{};
        if (!ReadBank(r, temp, makePrimRef, makeFrameRef)) return false;

        // Rebuild via your factory
        Vector<AnimationFrameRef> frames = temp.frames; // move/copy refs into the factory call
        outRef = createAnimationBank(temp.name, temp.flags, frames, temp.currentFrame, temp.currentTime);
        return (outRef != nullptr);
    }

}