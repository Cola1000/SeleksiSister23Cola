#ifndef MANDELBROT_H
#define MANDELBROT_H

#include <cstdint>
#include <vector>
#include <SDL3/SDL.h>

class Fractal {
public:
    Fractal(int width, int height);

    void render_cpu(SDL_Renderer* r, SDL_Texture* t, int threadCount = 0);;
    void render_gpu(SDL_Renderer* r, SDL_Texture* t, int threadCount = 0);

    void zoomAt(int mx, int my, int dir);
    void pan(int dx, int dy);

    const uint32_t* data() const { return pixels.data(); }

    double minRe, maxRe, minIm, maxIm;
    double reFactor, imFactor;

private:
    int width, height;
    std::vector<uint32_t> pixels;

    void update_factors();
    void clear_and_present(SDL_Renderer* r, SDL_Texture* t);
    static void render_section(uint32_t* pix,
                               int w,int h,
                               double minR,double maxR,
                               double minI,double maxI,
                               int y0,int y1);
};

#endif
