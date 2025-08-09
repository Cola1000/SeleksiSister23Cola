#include "image_save.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <SDL3/SDL.h>
#include <vector>
#include <iostream>
#include <cstring>

void save_bmp_from_buffer(const uint32_t* pixels, int w, int h) {
    SDL_Surface* surf = SDL_CreateSurface(w, h, SDL_PIXELFORMAT_ARGB8888);
    if (!surf) {
        std::cerr << "Failed to create SDL_Surface\n";
        return;
    }
    
    // Copy pixel data to surface
    memcpy(surf->pixels, pixels, w * h * sizeof(uint32_t));
    
    if (SDL_SaveBMP(surf, "./img/output.bmp") == 0) {
        std::cout << "Saved output.bmp\n";
    } else {
        std::cerr << "BMP save failed: " << SDL_GetError() << "\n";
    }
    SDL_DestroySurface(surf);
}

void save_png_from_buffer(const uint32_t* pixels, int w, int h) {
    std::vector<uint8_t> buf(w*h*4);
    for(int y=0;y<h;++y)
        std::memcpy(&buf[(h-1-y)*w*4], &pixels[y*w], w*4);
    if (stbi_write_png("./img/output.png", w,h,4, buf.data(), w*4))
        std::cout<<"Saved output.png\n";
    else
        std::cerr<<"PNG save failed\n";
}

// Legacy functions (IDK WHY IT DOESN'T WORK)

// void save_bmp_from_texture(SDL_Renderer* rend, SDL_Texture* tex, int w, int h) {
//     SDL_Surface* surf = SDL_CreateSurface(w, h, SDL_PIXELFORMAT_ARGB8888);
//     // SDL_RenderReadPixels(rend, nullptr, SDL_PIXELFORMAT_ARGB8888,
//     //                      surf->pixels, surf->pitch);
//     SDL_RenderReadPixels(rend, nullptr);
//     SDL_SaveBMP(surf, "mandelbrot.bmp");
//     SDL_DestroySurface(surf);
//     std::cout<<"Saved mandelbrot.bmp\n";
// }

// void save_png_from_texture(SDL_Renderer* rend, SDL_Texture* tex, int w, int h) {
//     std::vector<uint8_t> buf(w*h*4);
//     // SDL_RenderReadPixels(rend, nullptr, SDL_PIXELFORMAT_ARGB8888,
//     //                      buf.data(), w*4);
//     SDL_RenderReadPixels(rend, nullptr);
//     // flip rows:
//     std::vector<uint8_t> png(w*h*4);
//     for(int y=0; y<h; ++y)
//         memcpy(&png[(h-1-y)*w*4], &buf[y*w*4], w*4);

//     if (stbi_write_png("mandelbrot.png", w, h, 4, png.data(), w*4))
//         std::cout<<"Saved mandelbrot.png\n";
//     else
//         std::cerr<<"PNG save failed\n";
// }
