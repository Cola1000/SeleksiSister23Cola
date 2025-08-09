
/*
// WHO TF IS JULIA?!!!
*/


#ifndef JULIA_H
#define JULIA_H

#include <cstdint>
#include <vector>
#include <SDL3/SDL.h>

class Julia {
public:
    Julia(int width, int height);

    void render(SDL_Renderer* rend, SDL_Texture* tex);
    void setConstant(double cre, double cim);

    const uint32_t* data() const { return pixels.data(); }

    double minRe, maxRe, minIm, maxIm;
    double reFactor, imFactor;
    void zoomAt(int mx, int my, int dir);
    void pan(int dx, int dy);

private:
    int width, height;
    std::vector<uint32_t> pixels;
    double c_re, c_im;

    void updateFactors();
    static uint32_t mapColor(int iter, int maxIter);
};

#endif
