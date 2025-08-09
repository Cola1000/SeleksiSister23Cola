#include "image_save.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <vector>
#include <cstring>
#include <iostream>
#include <filesystem>

static void ensure_parent(const std::string& path) {
    try {
        std::filesystem::path p(path);
        auto dir = p.parent_path();
        if (!dir.empty() && !std::filesystem::exists(dir)) {
            std::filesystem::create_directories(dir);
        }
    } catch (...) {}
}

void save_bmp_from_buffer(const uint32_t* pixels, int w, int h, const std::string& path) {
    ensure_parent(path);
    std::vector<uint8_t> buf(w * h * 4);
    // Flip vertically for conventional top-left origin
    for (int y = 0; y < h; ++y) {
        std::memcpy(&buf[(h-1-y) * w * 4], &pixels[y * w], w * 4);
    }
    if (stbi_write_bmp(path.c_str(), w, h, 4, buf.data())) {
        std::cout << "Saved " << path << "\n";
    } else {
        std::cerr << "BMP save failed: " << path << "\n";
    }
}

void save_png_from_buffer(const uint32_t* pixels, int w, int h, const std::string& path) {
    ensure_parent(path);
    std::vector<uint8_t> buf(w * h * 4);
    for (int y = 0; y < h; ++y) {
        std::memcpy(&buf[(h-1-y) * w * 4], &pixels[y * w], w * 4);
    }
    if (stbi_write_png(path.c_str(), w, h, 4, buf.data(), w * 4)) {
        std::cout << "Saved " << path << "\n";
    } else {
        std::cerr << "PNG save failed: " << path << "\n";
    }
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
