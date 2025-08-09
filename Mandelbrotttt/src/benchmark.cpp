#include "benchmark.h"
#include "mandelbrot.h"
#include "font5x7.h"
#include "image_save.h"

#include <chrono>
#include <string>
#include <vector>
#include <filesystem>
#include <iostream>
#include <functional>  
#include <thread>     
#include <cstdio>     

static long long time_ms(std::function<void()> fn) {
    auto t0 = std::chrono::high_resolution_clock::now();
    fn();
    auto t1 = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
}

int run_benchmark(int width, int height, const std::string& out_png) {
    try {
        std::filesystem::create_directories(std::filesystem::path(out_png).parent_path());
    } catch (...) {}

    Fractal f(width, height);

    f.compute_only(1);

    long long t_single = time_ms([&](){ f.compute_only(1); });

    int threads = int(std::thread::hardware_concurrency());
    if (threads <= 0) threads = 4;

    long long t_multi = time_ms([&](){ f.compute_only(0); }); // auto threads

    double speedup = (t_multi > 0) ? (double)t_single / (double)t_multi : 0.0;

    std::cout << "Single-thread: " << t_single << " ms\n";
    std::cout << "Multi-thread (" << threads << "): " << t_multi << " ms\n";
    std::cout << "Speedup: " << speedup << "x\n";

    // Create a simple image with textual table
    const int W = 640, H = 220;
    std::vector<uint32_t> img(W * H, 0xFF111111u); // dark gray background

    // Frame/border
    uint32_t white = 0xFFFFFFFFu, cyan = 0xFF66FFFFu, yellow = 0xFFFFFF66u;
    for (int x = 0; x < W; ++x) { img[x] = white; img[(H-1)*W + x] = white; }
    for (int y = 0; y < H; ++y) { img[y*W] = white; img[y*W + (W-1)] = white; }

    draw_text_rgba(img.data(), W, H, 20, 20,  "BENCHMARK", cyan, 3);
    draw_text_rgba(img.data(), W, H, 20, 80,  "SINGLE-TIME: " + std::to_string(t_single) + " MS", white, 2);
    draw_text_rgba(img.data(), W, H, 20, 110, "MULTI(" + std::to_string(threads) + ") TIME: " + std::to_string(t_multi) + " MS", white, 2);

    char buf[64];
    std::snprintf(buf, sizeof(buf), "SPEEDUP: %.2f X", speedup);
    draw_text_rgba(img.data(), W, H, 20, 150, buf, yellow, 3);

    save_png_from_buffer(img.data(), W, H, out_png);
    return 0;
}
