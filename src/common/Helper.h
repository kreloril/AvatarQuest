#pragma once

namespace Util {
    constexpr uint32_t kType_Frame = 0xA0010001;
    // primitive
    void WritePrimitive(Util::BinStream& w, const Renderer::AnimPrimitive & p);
    bool ReadPrimitive(Util::BinStream& r, Renderer::AnimPrimitive& out);

    // full frame
   

    void writeVec2(Util::BinStream& w, const Vector2& v);
    bool readVec2(Util::BinStream& r, Vector2& v);
}