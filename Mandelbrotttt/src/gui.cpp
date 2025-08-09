#include "gui.h"
#include "image_save.h"
#include <iostream>

Gui::Gui(int argc, char* argv[])
  : width(720), height(480),
    useGPU(false), useSignle(false), doBenchmark(false), 
    showJulia(false),
    mandel(width, height),
    julia(width, height)
{
    // parse args: --gpu W H
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--gpu") {
            useGPU = true;
        }
        else if (a == "--single") {
            useSignle = true;
        }
        else if (a == "--benchmark") {
            doBenchmark = true;
        }
        else if (i + 1 < argc) {
            width  = std::stoi(a);
            height = std::stoi(argv[++i]);
        }
    }
    
    // Recreate fractals with new dimensions
    mandel = Fractal(width, height);
    julia = Julia(width, height);
}

Gui::~Gui() {
    if (texture)  SDL_DestroyTexture(texture);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window)   SDL_DestroyWindow(window);
    SDL_Quit();
}

bool Gui::initSDL() {
    SDL_SetMainReady();   
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << "\n";
        return false;
    }

    window = SDL_CreateWindow(
        "Fractal Viewer",
        width,
        height,
        SDL_WINDOW_RESIZABLE
    );
    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << "\n";
        SDL_Quit();
        return false;
    }

    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        width, height
    );
    if (!texture) {
        std::cerr << "SDL_CreateTexture Error: " << SDL_GetError() << "\n";
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    std::cout << "SDL initialized successfully\n";
    return true;
}

int Gui::run() {
    std::cout << "Starting application...\n";
    
    if (!initSDL()) {
        std::cerr << "Failed to initialize SDL\n";
        return 1;
    }

    std::cout << "Rendering initial fractal...\n";
    renderCurrent();
    
    SDL_SetWindowTitle(window,
        "T=Toggle Julia/Mandelbrot  B=Save BMP  P=Save PNG  Scroll=Zoom  WASD=Pan"
    );

    bool quit = false;
    SDL_Event e;
    std::cout << "Entering main loop...\n";
    
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                std::cout << "Quit event received\n";
                quit = true;
            } else {
                handleEvent(e);
            }
        }
        SDL_Delay(10);
    }
    
    std::cout << "Exiting application\n";
    return 0;
}

void Gui::handleEvent(const SDL_Event& e) {
    switch (e.type) {
      case SDL_EVENT_KEY_DOWN:
        if (e.key.scancode == SDL_SCANCODE_T) {
            showJulia = !showJulia;
            std::cout << "Switched to " << (showJulia ? "Julia" : "Mandelbrot") << " set\n";
        }
        else if (e.key.scancode == SDL_SCANCODE_B) {
            saveBMP("fractal.bmp");
            return;
        }
        else if (e.key.scancode == SDL_SCANCODE_P) {
            savePNG("fractal.png");
            return;
        }
        else if (e.key.scancode == SDL_SCANCODE_W) {
            if (showJulia) julia.pan(0,-1); else mandel.pan(0,-1);
        }
        else if (e.key.scancode == SDL_SCANCODE_S) {  // Fixed: This is now pan down
            if (showJulia) julia.pan(0,+1); else mandel.pan(0,+1);
        }
        else if (e.key.scancode == SDL_SCANCODE_A) {
            if (showJulia) julia.pan(-1,0); else mandel.pan(-1,0);
        }
        else if (e.key.scancode == SDL_SCANCODE_D) {
            if (showJulia) julia.pan(+1,0); else mandel.pan(+1,0);
        }
        renderCurrent();
        break;

      case SDL_EVENT_MOUSE_WHEEL:
        if (showJulia) {
            julia.zoomAt(e.wheel.mouse_x, e.wheel.mouse_y, e.wheel.y);
        } else {
            mandel.zoomAt(e.wheel.mouse_x, e.wheel.mouse_y, e.wheel.y);
        }
        renderCurrent();
        break;

      default:
        break;
    }
}

void Gui::renderCurrent() {
    if (showJulia) {
        julia.render(renderer, texture);
    }
    else if (useGPU) {
        mandel.render_gpu(renderer, texture);
    }
    else if (useSignle) {
        mandel.render_cpu(renderer, texture, 1);
    }
    else {
        mandel.render_cpu(renderer, texture);
    }
}

void Gui::saveBMP(const std::string& fn) {
    auto buf = showJulia ? julia.data() : mandel.data();
    save_bmp_from_buffer(buf, width, height);
}

void Gui::savePNG(const std::string& fn) {
    auto buf = showJulia ? julia.data() : mandel.data();
    save_png_from_buffer(buf, width, height);
}