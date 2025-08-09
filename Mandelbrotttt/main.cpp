#include <iostream>
#include <string>
#include <cctype>
#include "header/gui.h"
#include "header/benchmark.h"

static bool is_number(const char* s) {
    if (!s || !*s) return false;
    for (const char* p = s; *p; ++p) if (!std::isdigit((unsigned char)*p)) return false;
    return true;
}

int main(int argc, char* argv[]) {
    std::cout << "Mandelbrotttt - Fractal Viewer\n";

    bool doBenchmark = false;
    bool useSingle   = false;
    bool useGPU      = false;
    int  width = 720, height = 480;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--benchmark") {
            doBenchmark = true;
        } else if (a == "--single") {
            useSingle = true;
        } else if (a == "--gpu") {
            useGPU = true;
        } else if (is_number(a.c_str())) {
            width = std::stoi(a);
            if (i + 1 < argc && is_number(argv[i+1])) {
                height = std::stoi(argv[++i]);
            }
        }
    }

    if (doBenchmark) {
        int rc = run_benchmark(width, height, "img/benchmark/benchmark.png");
        if (rc == 0) {
            std::cout << "Benchmark image saved to img/benchmark/benchmark.png\n";
        } else {
            std::cerr << "Benchmark failed.\n";
        }
        return rc;
    }

    Gui app(width, height, useGPU, useSingle);
    return app.run();
}
