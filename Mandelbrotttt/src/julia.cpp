#include "julia.h"
#include <SDL3/SDL.h>

Julia::Julia(int w, int h)
  : width(w), height(h),
    minRe(-1.5), maxRe(1.5),
    minIm(-1.2), maxIm(1.2),
    c_re(-0.8), c_im(0.156),
    pixels(w*h, 0xFF000000u)
{
    updateFactors();
}

void Julia::updateFactors() {
    reFactor = (maxRe - minRe) / double(width - 1);
    imFactor = (maxIm - minIm) / double(height - 1);
}

uint32_t Julia::mapColor(int iter, int maxIter) {
    if (iter == maxIter) return 0xFF000000u;
    uint8_t r = (iter * 9) % 256;
    uint8_t g = (iter * 7) % 256;
    uint8_t b = (iter * 5) % 256;
    return (0xFFu << 24) | (r << 16) | (g << 8) | b;
}

void Julia::render(SDL_Renderer* rend, SDL_Texture* tex) {
    const int maxIter = 500;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            double zr = minRe + x * reFactor;
            double zi = maxIm - y * imFactor;
            int n = 0;
            while (n < maxIter && zr*zr + zi*zi <= 4.0) {
                double tmp = zr*zr - zi*zi + c_re;
                zi = 2.0 * zr * zi + c_im;
                zr = tmp;
                ++n;
            }
            pixels[y * width + x] = mapColor(n, maxIter);
        }
    }
    SDL_UpdateTexture(tex, nullptr, pixels.data(), width * sizeof(uint32_t));
    SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(rend, 0,0,0,255);
    SDL_RenderClear(rend);
    SDL_RenderTexture(rend, tex, nullptr, nullptr);
    SDL_RenderPresent(rend);
}

void Julia::setConstant(double cre, double cim) {
    c_re = cre; c_im = cim;
}

void Julia::zoomAt(int mx, int my, int dir) {
    double factor = dir > 0 ? 0.8 : 1.25;
    double clickRe = minRe + mx * reFactor;
    double clickIm = maxIm - my * imFactor;
    double wspan = (maxRe - minRe) * factor;
    double hspan = (maxIm - minIm) * factor;
    minRe = clickRe - wspan/2;  maxRe = clickRe + wspan/2;
    minIm = clickIm - hspan/2;  maxIm = clickIm + hspan/2;
    updateFactors();
}

void Julia::pan(int dx, int dy) {
    minRe += dx * reFactor * 50.0;
    maxRe += dx * reFactor * 50.0;
    minIm -= dy * imFactor * 50.0;
    maxIm -= dy * imFactor * 50.0;
    updateFactors();
}
