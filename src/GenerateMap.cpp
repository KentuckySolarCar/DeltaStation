#include "GenerateMap.h"
#include "imgui.h"

#include <algorithm>
#include <fstream>
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace DS {
    size_t write_file(void* ptr, size_t size, size_t nmemb, void* stream) {
        std::ofstream* f = (std::ofstream*)stream;
        f->write((char*)ptr, size * nmemb);
        return size * nmemb;
    }

    int lon_to_tile_x(double lon, int z) {
        return int((lon + 180.0) / 360.0 * (1 << z));
    }

    int lat_to_tile_y(double lat, int z) {
        double lat_rad = lat * PI / 180.0;
        return int(
            (1.0 - std::log(std::tan(lat_rad) + 1.0 / std::cos(lat_rad)) / PI)
            / 2.0 * (1 << z)
        );
    }

    bool download_tile(int z, int x, int y, const std::filesystem::path& path) {
        std::filesystem::create_directories(path.parent_path());

        CURL* curl = curl_easy_init();
        if (!curl) return false;

        std::string url =
            "https://tile.openstreetmap.org/" +
            std::to_string(z) + "/" +
            std::to_string(x) + "/" +
            std::to_string(y) + ".png";

        std::ofstream file(path, std::ios::binary);

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_file);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "TileStitcher/1.0");

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        file.close();

        return res == CURLE_OK;
    }

    Image load_tile(const std::filesystem::path& path) {
        int w, h, c;
        unsigned char* data = stbi_load(path.string().c_str(), &w, &h, &c, 4);
        if (!data) throw std::runtime_error("Failed to load tile");

        Image img{w, h};
        img.px.assign(data, data + w * h * 4);
        stbi_image_free(data);
        return img;
    }

    int make_map(double lon, double lat) try {
        //std::filesystem::remove_all(TILE_DIR);
        int center_x = lon_to_tile_x(lon, ZOOM);
        int center_y = lat_to_tile_y(lat, ZOOM);

        int tiles_needed = int(std::ceil(double(IMAGE_SIZE) / TILE_SIZE));
        int half = tiles_needed / 2;

        Image out;
        out.w = tiles_needed * TILE_SIZE;
        out.h = tiles_needed * TILE_SIZE;
        out.px.resize(out.w * out.h * 4, 255);

        for (int dy = -half; dy <= half; ++dy) {
            for (int dx = -half; dx <= half; ++dx) {
                int tx = center_x + dx;
                int ty = center_y + dy;

                std::filesystem::path tile_path =
                    std::filesystem::path(TILE_DIR) /
                    std::to_string(ZOOM) /
                    std::to_string(tx) /
                    (std::to_string(ty) + ".png");

                if (!std::filesystem::exists(tile_path)) {
                    download_tile(ZOOM, tx, ty, tile_path);
                }

                Image tile = load_tile(tile_path);

                int ox = (dx + half) * TILE_SIZE;
                int oy = (dy + half) * TILE_SIZE;

                for (int y = 0; y < TILE_SIZE; ++y) {
                    for (int x = 0; x < TILE_SIZE; ++x) {
                        int dst = ((oy + y) * out.w + (ox + x)) * 4;
                        int src = (y * TILE_SIZE + x) * 4;
                        std::copy_n(&tile.px[src], 4, &out.px[dst]);
                    }
                }
            }
        }

        // Crop to requested IMAGE_SIZE
        Image final_img{IMAGE_SIZE, IMAGE_SIZE};
        final_img.px.resize(IMAGE_SIZE * IMAGE_SIZE * 4);

        int start = (out.w - IMAGE_SIZE) / 2;

        for (int y = 0; y < IMAGE_SIZE; ++y) {
            std::memcpy(
                &final_img.px[y * IMAGE_SIZE * 4],
                &out.px[((y + start) * out.w + start) * 4],
                IMAGE_SIZE * 4
            );
        }

        stbi_write_png(
            "map.png",
            final_img.w,
            final_img.h,
            4,
            final_img.px.data(),
            final_img.w * 4
        );

        //std::filesystem::remove_all(TILE_DIR);
        return 0;
    }
    catch (const std::exception& e) {
        return 1;
    }

    bool load_texture_from_memory(const void* data, size_t data_size, GLuint* out_texture, int* out_width, int* out_height)
    {
        // Load from file
        int image_width = 0;
        int image_height = 0;
        unsigned char* image_data = stbi_load_from_memory((const unsigned char*)data, (int)data_size, &image_width, &image_height, NULL, 4);
        if (image_data == NULL)
            return false;

        // Create a OpenGL texture identifier
        GLuint image_texture;
        glGenTextures(1, &image_texture);
        glBindTexture(GL_TEXTURE_2D, image_texture);

        // Setup filtering parameters for display
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Upload pixels into texture
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
        stbi_image_free(image_data);

        *out_texture = image_texture;
        *out_width = image_width;
        *out_height = image_height;

        return true;
    }

    bool load_texture_from_file(const char* file_name, GLuint* out_texture, int* out_width, int* out_height)
    {
        FILE* f = fopen(file_name, "rb");
        if (f == NULL)
            return false;
        fseek(f, 0, SEEK_END);
        size_t file_size = (size_t)ftell(f);
        if (file_size == -1)
            return false;
        fseek(f, 0, SEEK_SET);
        void* file_data = IM_ALLOC(file_size);
        fread(file_data, 1, file_size, f);
        fclose(f);
        bool ret = load_texture_from_memory(file_data, file_size, out_texture, out_width, out_height);
        IM_FREE(file_data);
        return ret;
    }
};
