#ifndef GUI_H
#define GUI_H

#include <SDL3/SDL.h>
#include <string>
#include "mandelbrot.h"
#include "julia.h"

class Gui {
public:
    Gui(int w, int h, bool useGPU, bool useSingle);
    ~Gui();

    int run();

private:
    SDL_Window*   window   = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture*  texture  = nullptr;

    int  width, height;
    bool useGPU;
    bool useSingle;
    bool showJulia = false;

    Fractal mandel;
    Julia   julia;

    bool initSDL();
    void handleEvent(const SDL_Event& e);
    void renderCurrent();
    void saveBMP(const std::string& fn);
    void savePNG(const std::string& fn);
};

#endif
