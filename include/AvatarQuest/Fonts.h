#pragma once

#include "Common.h"
#include "Text.h"

namespace AvatarQuest {
namespace Fonts {

// Initialize font cache (optional; lazy loads by default)
void init();

// Get a cached font at the requested point size. Loads once on first use.
// Returns nullptr on failure.
Text::Font* get(int pointSize);

// Convenience wrappers for common sizes
inline Text::Font* title() { return get(31); }
inline Text::Font* ui()    { return get(27); }
// 2x UI for menus
inline Text::Font* menu()  { return get(54); }

// Release all cached fonts; call on shutdown.
void shutdown();

}
}
