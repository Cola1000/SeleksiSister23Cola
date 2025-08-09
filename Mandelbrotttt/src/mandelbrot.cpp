#include "mandelbrot.h"
#include <thread>
#include <chrono>
#include <iostream>

Fractal::Fractal(int w, int h)
 : width(w), height(h),
   minRe(-2.0), maxRe(1.0),
   pixels(w*h)
{
    update_factors();
}

void Fractal::update_factors() {
    minIm    = -(maxRe - minRe) * height / width / 2.0;
    maxIm    = -minIm;
    reFactor = (maxRe - minRe) / (width - 1);
    imFactor = (maxIm - minIm) / (height - 1);
}

void Fractal::clear_and_present(SDL_Renderer* r, SDL_Texture* t) {
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(r, 0,0,0,255);
    SDL_RenderClear(r);
    SDL_RenderTexture(r, t, nullptr, nullptr);
    SDL_RenderPresent(r);
}

void Fractal::render_cpu(SDL_Renderer* r, SDL_Texture* t, int threadCount) {
    int nThreads = (threadCount > 0 ? threadCount : std::thread::hardware_concurrency());
    // int nThreads = std::thread::hardware_concurrency();
    if (nThreads < 1) nThreads = 1; // fallback
    int rowsPer  = height / nThreads;
    auto t0 = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> pool;

    //Handle single and multi-thread
    if (nThreads == 1) { //Single thread
        render_section(pixels.data(), width, height, minRe, maxRe, minIm, maxIm, 0, height);
        auto t1 = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
        std::cout << "Thread 0 rows 0-" << (height-1) << " in " << ms << "ms\n";
        std::cout << "Total: " << ms << "ms\n";
        return;
    } else { //Multi-thread
        for (int i = 0; i < nThreads; ++i) {
            int y0 = i*rowsPer, y1 = (i+1==nThreads?height:y0+rowsPer);
            pool.emplace_back([=]() {
                auto s = std::chrono::high_resolution_clock::now();
                render_section(pixels.data(), width, height,
                               minRe,maxRe,minIm,maxIm, y0,y1);
                auto e = std::chrono::high_resolution_clock::now();
                long ms= std::chrono::duration_cast<std::chrono::milliseconds>(e-s).count();
                std::cout<<"Thread "<<i<<" rows "<<y0<<"-"<<y1-1<<" in "<<ms<<"ms\n";
            });
        }
    }

    for(auto &th: pool) th.join();

    auto t1 = std::chrono::high_resolution_clock::now();
    std::cout<<"Total "<<std::chrono::duration_cast<std::chrono::milliseconds>(t1-t0).count()<<"ms\n";

    SDL_UpdateTexture(t, nullptr, pixels.data(), width*sizeof(uint32_t));
    clear_and_present(r,t);
}

void Fractal::render_gpu(SDL_Renderer* r, SDL_Texture* t, int threadCount) {
    // stub: same as CPU for now (I aint got a GPU anyway T^T)
    render_cpu(r,t);
}

void Fractal::zoomAt(int mx, int my, int dir) {
    double factor = dir>0?0.8:1.25;
    double cre = minRe + mx*reFactor;
    double cim = maxIm - my*imFactor;
    double wspan = (maxRe-minRe)*factor;
    double hspan = (maxIm-minIm)*factor;
    minRe = cre - wspan/2;  maxRe = cre + wspan/2;
    minIm = cim - hspan/2;  maxIm = cim + hspan/2;
    update_factors();
}

void Fractal::pan(int dx, int dy) {
    double dRe = dx * reFactor * 50;
    double dIm = dy * imFactor * 50;
    minRe += dRe; maxRe += dRe;
    minIm -= dIm; maxIm -= dIm;
    update_factors();
}

// Meth stuff
void Fractal::render_section(uint32_t* pix,
                             int w,int h,
                             double minR,double maxR,
                             double minI,double maxI,
                             int y0,int y1)
{
    double rF=(maxR-minR)/(w-1), iF=(maxI-minI)/(h-1);
    const int mI=500;
    for(int y=y0;y<y1;++y){
        for(int x=0;x<w;++x){
            double c_re = minR + x*rF;
            double c_im = maxI - y*iF;
            double zr = 0.0;
            double zi = 0.0;
            int n=0;
            while(n<mI && zr*zr+zi*zi<=4.0){
                double tmp = zr*zr - zi*zi + c_re;
                zi = 2*zr*zi + c_im;
                zr = tmp;
                ++n;
            }
            uint8_t s=uint8_t(255 - (255.0*n/mI));
            pix[y*w + x] = (255u<<24)|(s<<16)|(s<<8)|s;
        }
    }
}
