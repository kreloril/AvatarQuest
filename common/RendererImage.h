#pragma once

namespace Renderer
{

	struct Image {
		SDL_FRect imageSize = { 0,0,0,0 };
		SDL_FRect imageRect = { 0,0,0,0 };
		SDL_Texture* texture = nullptr;
		SDL_PixelFormat format = SDL_PIXELFORMAT_UNKNOWN;
	};


	bool loadImageFromFile(const char* filename, Ref<Renderer::Image>&);
	bool releaseImage(Ref<Renderer::Image>& img);

} // namespace Renderer