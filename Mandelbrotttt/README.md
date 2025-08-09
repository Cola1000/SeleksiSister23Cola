**README.md**

## Deskripsi Implementasi

### 1. Implementasi Serial (Single-threaded)

* **File**: `mandelbrot_serial.cpp`
* **Deskripsi**: Menghitung dan merender himpunan Mandelbrot baris per baris pada satu thread. Hasil ditampilkan live menggunakan SDL3, dengan opsi menyimpan hasil sebagai BMP.
* **Kelebihan**: Sederhana, mudah dipahami.
* **Kekurangan**: Lambat pada resolusi tinggi karena eksekusi serial.

### 2. Implementasi Paralel CPU (Multithreaded)

* **File**: `main.cpp`, `mandelbrot.h`, `mandelbrot.cpp`, `image_save.h`, `image_save.cpp`
* **Deskripsi**: Membagi tugas render baris ke beberapa thread (jumlah sesuai `hardware_concurrency()`), lalu menggabungkan buffer dan menampilkan via SDL3. Mencatat waktu eksekusi tiap thread dan total rendering.
* **Fitur**:

  * Zoom in/out dengan scroll wheel pada posisi kursor.
  * Panning menggunakan tombol W/A/S/D.
  * Simpan frame ke BMP (`B`) atau PNG (`P`).
  * Logging waktu proses tiap thread dan total.

<!-- ### 3. ~~(Bonus) Akselerasi GPU~~

* **File**: `mandelbrot_gpu.cu`, `gpu_utils.h`
* **Deskripsi**: Mengimplementasikan kernel Mandelbrot di CUDA/OpenCL untuk eksekusi parallel pada GPU. Bandingkan waktu dengan CPU. -->

### 3. (Bonus) GUI Interaktif & Julia Set

* **File**: `julia.cpp`, `julia.h` (GUI terintegrasi dengan main.cpp)
* **Deskripsi**: GUI dengan zoom/pan real-time dan pilihan render himpunan Julia berdasarkan posisi kursor.

## Cara Kompilasi dan Menjalankan

### Prasyarat

* **SDL3 development** terpasang (`libSDL3-dev` pada Linux, library SDL3 pada Windows).
* **Compiler**: GCC/Clang (C++17) atau MSVC (sesuaikan flags).
* **Threading**: Pastikan linking `-lpthread` pada Linux.

---

### A. Serial (Single-threaded)

```bash
# Pastikan directory sudah di Project (XXX/SeleksiSister23Cola/Mandelbrotttt)
cd <whatever_your_path_is>/SeleksiSister23Cola/Mandelbrotttt

# Linux/macOS
g++ main.cpp src/*.cpp -std=c++17 -O3 -lSDL3 -o output

# Windows (MinGW)
g++ main.cpp src/*.cpp -std=c++17 -O3 -IC:/mingw64/include/SDL3 -LC:/mingw64/lib -lSDL3 -mwindows -o output.exe
```

Menjalankan:

```bash
./output [width] [height]
# Contoh: ./output 1920 1080
```

---

### B. Paralel CPU (Multithreaded)

```bash
# Pastikan directory sudah di Project (XXX/SeleksiSister23Cola/Mandelbrotttt)
cd <whatever_your_path_is>/SeleksiSister23Cola/Mandelbrotttt

# Linux/macOS
g++ main.cpp src/*.cpp -Iheader -std=c++17 -O3 -lSDL3 -lpthread -o output

# Windows (MinGW)
g++ main.cpp src/*.cpp -Iheader -std=c++17 -O3 -lSDL3 -lpthread -mwindows -o output
```

Menjalankan:

```bash
./output [width] [height]
# Contoh: ./output 3840 2160
```

---

<!-- ### C. (Bonus) GPU Acceleration

```bash
nvcc mandelbrot_gpu.cu gpu_utils.cpp -std=c++17 -O3 \
    -lSDL3 -lpthread -o mandelbrot_gpu
```

Menjalankan:

```bash
./mandelbrot_gpu [width height]
``` -->

### D. (Bonus) GUI & Julia Set

Ikuti instruksi pada file masing-masing modul:

* **Scroll wheel**: zoom in/out
* **W/A/S/D**: pan
* **B**: simpan BMP (Idk man, this is so unconventional wkwk)
* **P**: simpan PNG (This too :v)
* **T**: ganti dari Mandelbrot jadi Julia Set (dan sebaliknya)

---

### Screenshots and Benchmark


---


Credits: 
- [The Builder](https://www.youtube.com/@TheBuilder)