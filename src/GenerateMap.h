#ifndef GENERATE_MAP_H
#define GENERATE_MAP_H

#include <iostream>
#include <cmath>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <cstring>

#include "Graph.h"
#include "imgui.h"

constexpr int ZOOM = 13; // 0–19
constexpr int IMAGE_SIZE = 200;
constexpr int TILE_SIZE = 256;
constexpr const char* TILE_DIR = "tiles";
constexpr double PI = 3.14159265358979323846;

#include <curl/curl.h>

namespace fs = std::filesystem;

struct Image {
    int w, h;
    std::vector<unsigned char> px; // RGBA
};

size_t write_file(void* ptr, size_t size, size_t nmemb, void* stream);
int lon_to_tile_x(double lon, int z);
bool download_tile(int z, int x, int y, const fs::path& path);
Image load_tile(const fs::path& path);
int make_map(double lon, double lat);

// Image code from https://github.com/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples#example-for-opengl-users
bool load_texture_from_memory(const void* data, size_t data_size, GLuint* out_texture, int* out_width, int* out_height);
bool load_texture_from_file(const char* file_name, GLuint* out_texture, int* out_width, int* out_height);

#endif // GENERATE_MAP_H