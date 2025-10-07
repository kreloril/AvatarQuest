#pragma once


namespace Renderer {

	enum struct AnimFrameFlags : uint32_t {
		None = 0,
		Move = 1u << 0,
		Rotate = 1u << 1,
		Scale = 1u << 2
	};
	inline AnimFrameFlags operator|(AnimFrameFlags a, AnimFrameFlags b) {
		return static_cast<AnimFrameFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
	}
	inline AnimFrameFlags operator&(AnimFrameFlags a, AnimFrameFlags b) {
		return static_cast<AnimFrameFlags>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
	}
	inline bool HasFrameFlag(AnimFrameFlags m, AnimFrameFlags f) {
		return (static_cast<uint32_t>(m) & static_cast<uint32_t>(f)) != 0u;
	}


	enum struct AnimBankFlags : uint32_t {
		None = 0,
			Repeat = 1u << 0,   // loop time inside THIS frame	
			Ended = 1u << 1,   // end of animation
			Started = 1u << 2 // start of animation

	};

	inline AnimBankFlags operator|(AnimBankFlags lhs, AnimBankFlags rhs) {
		return static_cast<AnimBankFlags>(
			static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs)
			);
	}
	inline AnimBankFlags& operator|=(AnimBankFlags& lhs, AnimBankFlags rhs) {
		lhs = lhs | rhs;
		return lhs;
	}

	inline AnimBankFlags operator&(AnimBankFlags lhs, AnimBankFlags rhs) {
		return static_cast<AnimBankFlags>(
			static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs)
			);
	}
	inline AnimBankFlags& operator&=(AnimBankFlags& lhs, AnimBankFlags rhs) {
		lhs = lhs & rhs;
		return lhs;
	}

	inline AnimBankFlags operator~(AnimBankFlags f) {
		return static_cast<AnimBankFlags>(
			~static_cast<uint32_t>(f)
			);
	}


	enum struct AnimPrimitiveType {
		Line = 0,
		Rect = 1,
		RectFilled = 2
	};

	struct AnimPrimitive {
		AnimPrimitiveType type = AnimPrimitiveType::Line;
		AABB aabb;
		Vector2 center;
		Renderer::Color color = { 255,255,255,255 };
		float thickness = 1.0f; // For lines
		int layer = 0; // Layer for rendering, if needed
	};

	struct AnimationFrame;
	using AnimPrimitiveRef = Ref<Renderer::AnimPrimitive>;
	
	struct AnimationFrame {
		String name; // Name of the frame
		float duration = 0.1f; // Duration of the frame in seconds
		
		Vector2 positionStart; // Position of the frame
		Vector2 positionEnd; // End position of the frame (if moves is true)
		Vector2 rotationStartEnd;
		Vector2 scaleStart;
		Vector2 scaleEnd;
		AnimFrameFlags flags = AnimFrameFlags::None;   // <-- per-frame controls
		Vector<AnimPrimitiveRef> primitives; // List of primitives for this frame
	
	};

	struct AnimationBank;
	using AnimationFrameRef = Ref<Renderer::AnimationFrame>;
	using AnimationBackCallback = std::function<void(Ref<AnimationBank>&)>;
	
	struct AnimationBank {
		String name;
		AnimBankFlags flags = AnimBankFlags::None; // Flags for the animation bank
		Uint32 currentFrame = 0; // Current frame index
		float currentTime = 0.0f; // Current time in the animation
		VectorRef<AnimationFrame> frames; // List of frames in the animation
		AnimationBackCallback onBankStart = nullptr; // Callback for when the bank starts
		AnimationBackCallback onBankEnd = nullptr; // Callback for when the bank ends

	};

	enum struct AnimationUpdateState {
		None = 0,
		Playing = 1,
		Paused = 2,
		Stopped = 3,
		Finished = 4
	};

	using AnimationBankRef = Ref<Renderer::AnimationBank>;

	// Function declarations
	// Creates an animation primitive of the specified type with the given AABB and color

	AnimPrimitiveRef createAnimPrimitive(AnimPrimitiveType type, const Renderer::AABB& aabb, const Renderer::Color& color, float thickness = 1.0f,int layer = 0.0f);

	Vector<Renderer::AnimPrimitiveRef>  createAnimPrimitiveFromPrimitiveModel(const Renderer::PrimitiveModel& model, const Renderer::Color& color, float thickness = 1.0f, int layer = 0.0f);
	// Creates an animation frame with the specified name, list of primitives, flags, duration, and transformation parameters
	
	// AnimFrameFlags flags
	AnimationFrameRef createAnimationFrame(const String& name, AnimFrameFlags flags, Vector<AnimPrimitiveRef>& anims, float duration = 1.0f, Vector2 posStart = Vector2(0,0), Vector2 posEnd = Vector2(0, 0), Vector2 rotationStartEnd = Vector2(0, 0), Vector2 scaleStart = Vector2(1, 1), Vector2 scaleEnd = Vector2(1, 1));

	// Creates an animation bank with the specified name, list of frames, current frame index, and current time
	AnimationBankRef createAnimationBank(const String& name, AnimBankFlags flags, Vector<AnimationFrameRef>& frames, Uint32 currentFrame = 0, float currentTime = 0.0f);

	// Updates the animation bank based on the delta time and returns the update state

	AnimationUpdateState updateAnimationBank(AnimationBankRef& bank, float deltaTime);

	// Renders the animation bank at the specified position, scale, and rotation
	void RenderAnimationBank(AnimationBankRef& bank, const Vector2& position, const Vector2& scale, float rotation);

	void ResetAnimationBank(AnimationBankRef& bank,
		Uint32 toFrame = 0,
		float startTime = 0.0f);
}