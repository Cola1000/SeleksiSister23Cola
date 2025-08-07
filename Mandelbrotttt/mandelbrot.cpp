#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <numeric>
#include <complex>
#include <iostream>
#include <string>
#include <vector>

#include "image_save.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int mandelbrot(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    int width = 720;
    int height = 480;
    if (argc >= 3) {
        try { width = std::stoi(argv[1]); height = std::stoi(argv[2]); }
        catch (...) { std::cerr << "Invalid arguments. Using default size." << std::endl; }
    }

    SDL_Window* window = SDL_CreateWindow(
        "Mandelbrot Renderer",
        width, height,
        SDL_WINDOW_RESIZABLE
    );
    if (!window) { std::cerr << "Window creation error! " << SDL_GetError() << std::endl; SDL_Quit(); return 1; }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) { std::cerr << "Renderer creation error! " << SDL_GetError() << std::endl; SDL_DestroyWindow(window); SDL_Quit(); return 1; }

    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                             SDL_TEXTUREACCESS_STREAMING, width, height);

    std::vector<uint32_t> pixels(width * height);
    double minRe = -2.0, maxRe = 1.0;
    double minIm, maxIm, reFactor, imFactor;
    auto update_bounds = [&]() {
        minIm   = -(maxRe - minRe) * height / width / 2.0;
        maxIm   = -minIm;
        reFactor = (maxRe - minRe) / (width - 1);
        imFactor = (maxIm - minIm) / (height - 1);
    };
    auto render_fractal = [&]() {
        const int maxIter = 500;
        for (int y = 0; y < height; ++y) {
            double c_im = maxIm - y * imFactor;
            for (int x = 0; x < width; ++x) {
                double c_re = minRe + x * reFactor;
                double Z_re = c_re, Z_im = c_im;
                int n = 0;
                for (; n < maxIter; ++n) {
                    double Z_re2 = Z_re * Z_re;
                    double Z_im2 = Z_im * Z_im;
                    if (Z_re2 + Z_im2 > 4.0) break;
                    Z_im = 2 * Z_re * Z_im + c_im;
                    Z_re = Z_re2 - Z_im2 + c_re;
                }
                uint8_t shade = static_cast<uint8_t>(255 - (255.0 * n / maxIter));
                pixels[y * width + x] = (255u << 24) | (shade << 16) | (shade << 8) | shade;
            }
        }
    };

    // Initial draw
    update_bounds(); render_fractal();
    SDL_UpdateTexture(texture, nullptr, pixels.data(), width * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
    SDL_SetWindowTitle(window, "Mandelbrot - B=BMP, P=PNG, Scroll=Zoom");

    bool quit = false;
    SDL_Event event;
    while (!quit) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    quit = true;
                    break;
                case SDL_EVENT_KEY_DOWN:
                    if (event.key.scancode == SDL_SCANCODE_B) save_bmp(pixels.data(), width, height);
                    else if (event.key.scancode == SDL_SCANCODE_P) save_png(pixels.data(), width, height);
                    break;
                case SDL_EVENT_MOUSE_WHEEL: {
                    // Zoom at mouse position
                    double scroll = event.wheel.y; // positive = up, negative = down
                    double zoomFactor = (scroll > 0 ? 0.8 : 1.25);
                    double mx = event.wheel.mouse_x;
                    double my = event.wheel.mouse_y;
                    double clickRe = minRe + mx * reFactor;
                    double clickIm = maxIm - my * imFactor;
                    double wSpan = (maxRe - minRe) * zoomFactor;
                    double hSpan = (maxIm - minIm) * zoomFactor;
                    minRe = clickRe - wSpan / 2;
                    maxRe = clickRe + wSpan / 2;
                    minIm = clickIm - hSpan / 2;
                    maxIm = clickIm + hSpan / 2;
                    update_bounds(); render_fractal();
                    SDL_UpdateTexture(texture, nullptr, pixels.data(), width * sizeof(uint32_t));
                    SDL_RenderClear(renderer);
                    SDL_RenderTexture(renderer, texture, nullptr, nullptr);
                    SDL_RenderPresent(renderer);
                    break;
                }
                default:
                    break;
            }
        }
        SDL_Delay(10);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

int main(int argc, char* argv[]) {
    return SDL_RunApp(argc, argv, mandelbrot, nullptr);
}
