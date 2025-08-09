#include "gui.h"
#include "image_save.h"
#include <iostream>

Gui::Gui(int w, int h, bool gpu, bool single)
  : width(w), height(h), useGPU(gpu), useSingle(single),
    mandel(width, height), julia(width, height) {}

Gui::~Gui() {
    if (texture)  SDL_DestroyTexture(texture);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window)   SDL_DestroyWindow(window);
    SDL_Quit();
}

bool Gui::initSDL() {
    // SDL3: SDL_Init returns bool (true on success, false on failure)
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << "\n";
        return false;
    }

    window = SDL_CreateWindow("Fractal Viewer", width, height, SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << "\n";
        return false;
    }

    renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << "\n";
        return false;
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                SDL_TEXTUREACCESS_STREAMING, width, height);
    if (!texture) {
        std::cerr << "SDL_CreateTexture Error: " << SDL_GetError() << "\n";
        return false;
    }

    return true;
}

int Gui::run() {
    if (!initSDL()) return 1;

    SDL_SetWindowTitle(window,
        "B=Save BMP  P=Save PNG  T=Toggle Julia/Mandelbrot  Scroll=Zoom  WASD=Pan"
    );

    renderCurrent();

    bool quit = false;
    SDL_Event e;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                quit = true;
            } else {
                handleEvent(e);
            }
        }
        SDL_Delay(10);
    }
    return 0;
}

void Gui::handleEvent(const SDL_Event& e) {
    switch (e.type) {
    case SDL_EVENT_KEY_DOWN: {
        auto sc = e.key.scancode;
        if (sc == SDL_SCANCODE_T) {
            showJulia = !showJulia;
            renderCurrent();
        } else if (sc == SDL_SCANCODE_B) {
            saveBMP("fractal.bmp");
        } else if (sc == SDL_SCANCODE_P) {
            savePNG("fractal.png");
        } else if (sc == SDL_SCANCODE_W) {
            if (showJulia) julia.pan(0,-1); else mandel.pan(0,-1);
            renderCurrent();
        } else if (sc == SDL_SCANCODE_S) {
            if (showJulia) julia.pan(0,+1); else mandel.pan(0,+1);
            renderCurrent();
        } else if (sc == SDL_SCANCODE_A) {
            if (showJulia) julia.pan(-1,0); else mandel.pan(-1,0);
            renderCurrent();
        } else if (sc == SDL_SCANCODE_D) {
            if (showJulia) julia.pan(+1,0); else mandel.pan(+1,0);
            renderCurrent();
        }
        break;
    }
    case SDL_EVENT_MOUSE_WHEEL: {
        if (showJulia) {
            julia.zoomAt(e.wheel.mouse_x, e.wheel.mouse_y, e.wheel.y);
        } else {
            if (e.wheel.y > 0)
                mandel.zoomAt(e.wheel.mouse_x, e.wheel.mouse_y, +1);
            else
                mandel.zoomAt(width/2, height/2, -1);
        }
        renderCurrent();
        break;
    }
    default: break;
    }
}

void Gui::renderCurrent() {
    if (showJulia) {
        julia.render(renderer, texture);
    } else if (useGPU) {
        mandel.render_gpu(renderer, texture);
    } else if (useSingle) {
        mandel.render_cpu(renderer, texture, 1);
    } else {
        mandel.render_cpu(renderer, texture, 0); // auto threads
    }
}

void Gui::saveBMP(const std::string& fn) {
    auto buf = showJulia ? julia.data() : mandel.data();
    save_bmp_from_buffer(buf, width, height, std::string("img/") + fn);
}

void Gui::savePNG(const std::string& fn) {
    auto buf = showJulia ? julia.data() : mandel.data();
    save_png_from_buffer(buf, width, height, std::string("img/") + fn);
}
