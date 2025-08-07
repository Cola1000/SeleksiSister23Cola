#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "stb_image_write.h"

void save_bmp(uint32_t* pixels, int w, int h) {
    SDL_Surface* surf = SDL_CreateSurfaceFrom(
        w, h, SDL_PIXELFORMAT_ARGB8888, pixels, w * 4
    );
    if (surf) {
        SDL_SaveBMP(surf, "mandelbrot.bmp");
        SDL_DestroySurface(surf);
        std::cout << "Saved mandelbrot.bmp\n";
    } else {
        std::cerr << "SDL_CreateSurfaceFrom failed: " << SDL_GetError() << "\n";
    }
}

void save_png(uint32_t* pixels, int w, int h) {
    std::vector<uint8_t> png_data(w * h * 4);
    for (int y = 0; y < h; ++y) {
        memcpy(&png_data[(h - 1 - y) * w * 4], &pixels[y * w], w * 4);
    }
    if (stbi_write_png("mandelbrot.png", w, h, 4, png_data.data(), w * 4)) {
        std::cout << "Saved mandelbrot.png\n";
    } else {
        std::cerr << "Failed writing mandelbrot.png\n";
    }
}