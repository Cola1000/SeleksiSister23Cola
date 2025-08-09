#ifndef IMAGE_SAVE_H
#define IMAGE_SAVE_H

#include <SDL3/SDL.h>
#include <cstdint>
#include <string>

// void save_bmp_from_texture(SDL_Renderer* renderer, SDL_Texture* tex, int w, int h);
// void save_png_from_texture(SDL_Renderer* renderer, SDL_Texture* tex, int w, int h);

void save_bmp_from_buffer(const uint32_t* pixels, int w, int h, const std::string& path);
void save_png_from_buffer(const uint32_t* pixels, int w, int h, const std::string& path);

#endif