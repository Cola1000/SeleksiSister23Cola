#include <SDL3/SDL.h>
#include "gui.h"
#include <iostream>

int main(int argc, char* argv[]) {
    std::cout << "Mandelbrotttt - Fractal Viewer\n";
    Gui app(argc, argv);
    return app.run();
}