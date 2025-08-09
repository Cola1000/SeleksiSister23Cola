# Mandelbrotttt — Mandelbrot & Julia Viewer (SDL3)

Idk, dikasih tugas sama aslab sister, aku kerjain aja

## Features

- **Fractals:** Mandelbrot (default) and Julia (`T` to toggle)
- **Controls:**
  - Mouse **Scroll**: zoom in/out (zoom-in focuses under cursor)
  - **W/A/S/D**: pan up/left/down/right
  - **T**: toggle Mandelbrot ↔ Julia
  - **P**: save PNG → `img/fractal.png`
  - **B**: save BMP → `img/fractal.bmp`
- **CPU parallelism:** multi-threaded by default (uses `hardware_concurrency()`), or force single-thread with `--single`
- **Benchmark mode:** `--benchmark` runs single-thread **and** multi-thread renders offscreen, then writes a result image to `img/benchmark/benchmark.png` showing times and speedup
- **Command-line only**: no menus; logs to console for confirmations (e.g., saved image paths)

## Directory Layout (relevant parts)

```
Mandelbrotttt/
├── header/
│   ├── benchmark.h
│   ├── font5x7.h
│   ├── gui.h
│   ├── image_save.h
│   ├── julia.h
│   ├── mandelbrot.h
│   └── stb_image_write.h
├── src/
│   ├── benchmark.cpp            # --benchmark implementation
│   ├── gui.cpp                  # SDL3
│   ├── image_save.cpp           # PNG/BMP save
│   ├── julia.cpp
│   └── mandelbrot.cpp
├── img/
│   ├── fractal.png (created when you press P)
│   └── benchmark/
│       └── benchmark.png (pre-generated, overwritten by --benchmark)
├── legacy/                      # It's filed with old exe file.
│                                # You can try it, but i forgot the features :v
├── main.cpp
├── SDL3.dll (Windows runtime)
└── Fractals.exe                 # If you don't want to compile, just run this
```

> **Note:** `image_save.cpp` defines `STB_IMAGE_WRITE_IMPLEMENTATION` so `stb_image_write.h` actually links. Don’t define it anywhere else.

---

## Build Instructions

### 0) Prereqs

- **C++17** compiler (GCC/Clang/MSVC)
- **SDL3 development** libraries/headers
- **Windows (MinGW/MSYS2)** users: ensure `SDL3.dll` is findable at runtime (same folder as `a.exe` is fine)

> **MSYS2 (recommended on Windows)**  
> Install toolchain + SDL3:
> ```bash
> pacman -S --needed mingw-w64-x86_64-toolchain mingw-w64-x86_64-SDL3
> ```
> Then open the **MSYS2 MinGW x64** shell to build.

### 1) Build

#### Windows (MinGW)
```bash
g++ main.cpp src/*.cpp -Iheader -std=c++17 -O3 -lSDL3 -o output
```
If your SDL headers/libs are in custom paths, add:
```
-IC:/mingw64/include -LC:/mingw64/lib
```

#### Linux / macOS
```bash
g++ main.cpp src/*.cpp -Iheader -std=c++17 -O3 -lSDL3 -o output
```

> **If you see `std::filesystem` link errors** on very old GCC, try `-std=gnu++17` or add `-lstdc++fs` (older GCC only).

---

## Run

### Interactive viewer
```bash
./a [WIDTH HEIGHT] [--single] [--gpu]
```

- `WIDTH HEIGHT` (optional): window size. Default `720 480`.
- `--single` (optional): force single-thread Mandelbrot in interactive mode.
- `--gpu` (optional): placeholder/stub (currently same as CPU).

**Examples**
```bash
./a
./a 1280 720 # Creates a 1280x720 window
./a 1920 1080 --single # Creates a 1920x1080 window and single threaded calculation
```

**Controls**
- **Scroll**: zoom in/out (zoom-in at mouse cursor, zoom-out from center)
- **W/A/S/D**: pan
- **T**: toggle Mandelbrot ↔ Julia
- **P**: save PNG → `img/fractal.png` (path auto-created)
- **B**: save BMP → `img/fractal.bmp` (path auto-created)

### Benchmark mode
```bash
./a [WIDTH HEIGHT] --benchmark
```
- Renders Mandelbrot **twice** offscreen:
  1) **Single-thread**
  2) **Multi-thread** (uses all available cores)
- Saves a result card with times & speedup to:
  ```
  img/benchmark/benchmark.png
  ```
- Also prints raw numbers to the console.

**Examples**
```bash
./a --benchmark
./a 1024 768 --benchmark # Benchmark if window size is 1024x720
```

---

## What’s Included / How It Works

- **Mandelbrot** (`mandelbrot.cpp/.h`):
  - Correct iteration with `z₀=0`, `c` from pixel
  - Multi-threaded by splitting rows among worker threads
  - Single-threaded path for comparisons/`--single`
- **Julia** (`julia.cpp/.h`):
  - Default constant `c = -0.8 + 0.156i`
  - Same zoom/pan UX; toggle with `T`
- **GUI** (`gui.cpp/.h`):
  - SDL3 window, input handling, texture updates
  - Title hints for keys
- **Saving** (`image_save.cpp/.h` + `stb_image_write.h`):
  - Writes **PNG/BMP**; vertically flips for top-left origin
  - Ensures parent folders exist (creates `img/` and `img/benchmark/` automatically)
- **Benchmark** (`benchmark.cpp/.h` + `font5x7.h`):
  - Times single vs multi, computes speedup
  - Draws a simple card with a tiny 5×7 bitmap font
  - Saves to `img/benchmark/benchmark.png`

---

## Troubleshooting

- **Linker errors**: `undefined reference to stbi_write_png/bmp`  
  → Ensure `image_save.cpp` contains:
  ```cpp
  #define STB_IMAGE_WRITE_IMPLEMENTATION
  #include "header/stb_image_write.h"
  ```
  and **no other file** defines that macro.

- **`SDL_Init Error:` / empty message**  
  → Make sure you’re using SDL3 and the code checks `if (!SDL_Init(SDL_INIT_VIDEO))` (SDL3 returns `bool`).

- **Windows won’t start due to missing `SDL3.dll`**  
  → Place `SDL3.dll` next to `a.exe` or add its folder to `PATH`.

- **Old GCC + `std::filesystem` link error**  
  → Add `-lstdc++fs` or compile with `-std=gnu++17`.

---

## Credits

- **SDL3** — Simple DirectMedia Layer
- **stb_image_write.h** — (Public domain / MIT-like) single-header image writer by Sean Barrett
- **Tiny 5×7 bitmap font** — embedded for simple text rendering on benchmark card
- **The Builder** — inspiration: [YouTube: @TheBuilder](https://www.youtube.com/@TheBuilder)

---

## Images

### Benchmarks:
- Default
![Benchmark result](img/benchmark/benchmark(default).png)

- 5000 x 5000 px
![Benchmark result](img/benchmark/benchmark(5000x5000).png)

### Wife:
![Wife](img/wife.png)