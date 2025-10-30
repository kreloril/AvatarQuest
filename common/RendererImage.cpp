#include "common.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace Renderer
{
	bool loadImageFromFile(const char* filename, Ref<Renderer::Image>& newImage)
	{
        if (!filename || !*filename)
            return false;

        SDL_Renderer* renderer = Renderer::getRenderer();
        if (!renderer)
            return false;

        // Optionally flip vertically if your art expects OpenGL-like origin.
        // stbi_set_flip_vertically_on_load(1);

        int width = 0, height = 0, channelsInFile = 0;
        // Force 4 channels (RGBA) so we can use SDL_PIXELFORMAT_RGBA32
        stbi_uc* pixels = stbi_load(filename, &width, &height, &channelsInFile, STBI_rgb_alpha);
        if (!pixels) {
            SDL_Log("stbi_load failed for '%s': %s", filename, stbi_failure_reason());
            return false;
        }

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
        SDL_PixelFormat sdlFmt = SDL_PIXELFORMAT_ABGR8888; // bytes: R,G,B,A in memory
#else
        SDL_PixelFormat sdlFmt = SDL_PIXELFORMAT_RGBA8888;
#endif

        SDL_Texture* tex = SDL_CreateTexture(
            renderer,
            sdlFmt,
            SDL_TEXTUREACCESS_STREAMING,  // streaming is safer for manual uploads
            width, height
        );


        if (!tex) {
            SDL_Log("SDL_CreateTexture failed: %s", SDL_GetError());
            stbi_image_free(pixels);
            return false;
        }

        // Blend/scaling settings are optional but typical for sprites/UI
        SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_LINEAR);


        void* dst = nullptr;
        int dstPitch = 0;
        if (SDL_LockTexture(tex, nullptr, &dst, &dstPitch) == false) {
            SDL_Log("SDL_LockTexture failed: %s", SDL_GetError());
            SDL_DestroyTexture(tex);
            stbi_image_free(pixels);
            return false;
        }

        const int srcPitch = width * 4; // RGBA8
        if (dstPitch == srcPitch) {
            // Fast path: single memcpy
            std::memcpy(dst, pixels, size_t(srcPitch) * size_t(height));
        }
        else {
            // Safe path: copy row-by-row
            const uint8_t* srcRow = pixels;
            uint8_t* dstRow = static_cast<uint8_t*>(dst);
            for (int y = 0; y < height; ++y) {
                std::memcpy(dstRow, srcRow, srcPitch);
                srcRow += srcPitch;
                dstRow += dstPitch;
            }
        }

        SDL_UnlockTexture(tex);
        // We’re done with CPU-side pixels
        stbi_image_free(pixels);

        // Build the Image object
        Ref<Renderer::Image> img = CreateRef<Renderer::Image>();
        img->texture = tex;
        img->imageSize = SDL_FRect{ 0, 0, (float)width, (float)height };
		img->imageRect = SDL_FRect{ 0, 0, (float)width, (float)height };
        img->format = SDL_PIXELFORMAT_RGBA32;

        newImage = img;
        return true;
	}
	bool releaseImage(Ref<Renderer::Image>& img)
	{
        if (!img)
            return false;

        if (img->texture) {
            SDL_DestroyTexture(img->texture);
            img->texture = nullptr;
        }
        img->imageSize = SDL_FRect{ 0, 0, 0, 0 };
        img->format = SDL_PIXELFORMAT_UNKNOWN;

        img.reset(); // release Ref (shared_ptr/unique_ptr as you defined)
        return true;
	}
}