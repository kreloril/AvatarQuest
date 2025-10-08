#pragma once

namespace Util {
    struct PackedColor { uint8_t r, g, b, a; };
    void WriteFrame(Util::BinStream& w, const Renderer::AnimationFrame& fr);
    bool ReadFrame(Util::BinStream& r, Renderer::AnimationFrame& out,
    const std::function<Renderer::AnimPrimitiveRef(const Renderer::AnimPrimitive&)>& makeRef);

    void WriteBank(Util::BinStream& w, const Renderer::AnimationBank& bank);

    // Reads into an existing bank object (replaces its fields & frames).
    // You must provide a primitive factory for frames; a frame factory is optional.
    // If makeFrameRef is null, CreateRef<AnimationFrame>(temp) is used.
    bool ReadBank(
        Util::BinStream& r,
        Renderer::AnimationBank& out,
        const std::function<Renderer::AnimPrimitiveRef(const Renderer::AnimPrimitive&)>& makePrimRef,
        const std::function<Renderer::AnimationFrameRef(const Renderer::AnimationFrame&)>& makeFrameRef = {}
    );

    // Convenience: read and build a new bank via your factory createAnimationBank(...).
    // makePrimRef is required; frames are deep-cloned through makeFrameRef if provided,
    // otherwise they are created with CreateRef<AnimationFrame>(temp).
    bool ReadBank(
        Util::BinStream& r,
        Renderer::AnimationBankRef& outRef,
        const std::function<Renderer::AnimPrimitiveRef(const Renderer::AnimPrimitive&)>& makePrimRef,
        const std::function<Renderer::AnimationFrameRef(const Renderer::AnimationFrame&)>& makeFrameRef = {}
    );

}