#include "mandelbrot.h"
#include <thread>
#include <chrono>
#include <iostream>
#include <algorithm>

Fractal::Fractal(int w, int h)
 : width(w), height(h),
   minRe(-2.0), maxRe(1.0),
   pixels(w*h, 0xFF000000u)
{
    update_factors();
}

void Fractal::reset() {
    minRe = -2.0; maxRe = 1.0;
    update_factors();
}

void Fractal::update_factors() {
    minIm    = -(maxRe - minRe) * height / double(width) / 2.0;
    maxIm    = -minIm;
    reFactor = (maxRe - minRe) / double(width - 1);
    imFactor = (maxIm - minIm) / double(height - 1);
}

void Fractal::clear_and_present(SDL_Renderer* r, SDL_Texture* t) {
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(r, 0,0,0,255);
    SDL_RenderClear(r);
    SDL_RenderTexture(r, t, nullptr, nullptr);
    SDL_RenderPresent(r);
}

void Fractal::render_cpu(SDL_Renderer* r, SDL_Texture* t, int threadCount) {
    // compute
    compute_only(threadCount);
    // present
    SDL_UpdateTexture(t, nullptr, pixels.data(), width * sizeof(uint32_t));
    clear_and_present(r, t);
}

void Fractal::render_gpu(SDL_Renderer* r, SDL_Texture* t) {
    // I don't have GPU, so.... no.... wkwk (kalau ada waktu atau apalah)
    render_cpu(r, t);
}

void Fractal::compute_only(int threadCount) {
    int nThreads = threadCount > 0 ? threadCount : int(std::thread::hardware_concurrency());
    if (nThreads <= 0) nThreads = 4;

    int rowsPer  = std::max(1, height / nThreads);

    auto t0 = std::chrono::high_resolution_clock::now();

    if (nThreads == 1) {
        render_section(pixels.data(), width, height,
                       minRe, maxRe, minIm, maxIm, 0, height);
    } else {
        std::vector<std::thread> pool;
        pool.reserve(nThreads);
        for (int i = 0; i < nThreads; ++i) {
            int y0 = i * rowsPer;
            int y1 = (i + 1 == nThreads ? height : y0 + rowsPer);
            pool.emplace_back([=]() {
                render_section(const_cast<uint32_t*>(pixels.data()), width, height,
                               minRe, maxRe, minIm, maxIm, y0, y1);
            });
        }
        for (auto &th : pool) th.join();
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    std::cout << (nThreads == 1 ? "[single]" : "[multi]") << " compute "
              << ms << " ms (" << nThreads << " threads)\n";
}

void Fractal::zoomAt(int mx, int my, int dir) {
    double factor = dir > 0 ? 0.8 : 1.25;
    double cre = minRe + mx * reFactor;
    double cim = maxIm - my * imFactor;
    double wspan = (maxRe - minRe) * factor;
    double hspan = (maxIm - minIm) * factor;
    minRe = cre - wspan / 2;  maxRe = cre + wspan / 2;
    minIm = cim - hspan / 2;  maxIm = cim + hspan / 2;
    update_factors();
}

void Fractal::pan(int dx, int dy) {
    double dRe = dx * reFactor * 50.0;
    double dIm = dy * imFactor * 50.0;
    minRe += dRe; maxRe += dRe;
    minIm -= dIm; maxIm -= dIm;
    update_factors();
}

void Fractal::render_section(uint32_t* pix,
                             int w, int h,
                             double minR, double maxR,
                             double minI, double maxI,
                             int y0, int y1)
{
    const int maxIter = 500;
    double rF = (maxR - minR) / double(w - 1);
    double iF = (maxI - minI) / double(h - 1);

    for (int y = y0; y < y1; ++y) {
        for (int x = 0; x < w; ++x) {
            double c_re = minR + x * rF;
            double c_im = maxI - y * iF;
            double zr = 0.0, zi = 0.0;
            int n = 0;
            while (n < maxIter && zr*zr + zi*zi <= 4.0) {
                double tmp = zr*zr - zi*zi + c_re;
                zi = 2.0 * zr * zi + c_im;
                zr = tmp;
                ++n;
            }
            uint8_t s = uint8_t(255 - (255.0 * n / maxIter));
            pix[y * w + x] = (255u << 24) | (s << 16) | (s << 8) | s; // ARGB gray
        }
    }
}
