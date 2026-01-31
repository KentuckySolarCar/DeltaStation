/* date = May 05, 2025 5:45 PM */

// Tutorials/References used for writing this file:
// https://www.glfw.org/docs/latest/quick_guide.html for glfw boilerplate
// https://github.com/ocornut/imgui/blob/master/examples/example_glfw_opengl3/main.cpp for imgui with glfw
// https://github.com/epezent/implot/blob/master/implot.h for implot docs

#include "Window.h"

#include <iostream>
#include <ranges>
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <cstring>

#include "Dashboard.h"
#include "Graph.h"
#include "imgui.h"
#include "implot.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#include "../libs/stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../libs/stb/stb_image_write.h"

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

bool download_tile(int z, int x, int y, const fs::path& path) {
    fs::create_directories(path.parent_path());

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

Image load_tile(const fs::path& path) {
    int w, h, c;
    unsigned char* data = stbi_load(path.string().c_str(), &w, &h, &c, 4);
    if (!data) throw std::runtime_error("Failed to load tile");

    Image img{w, h};
    img.px.assign(data, data + w * h * 4);
    stbi_image_free(data);
    return img;
}

int make_map(double lon, double lat) try {
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

            fs::path tile_path =
                fs::path(TILE_DIR) /
                std::to_string(ZOOM) /
                std::to_string(tx) /
                (std::to_string(ty) + ".png");

            if (!fs::exists(tile_path)) {
                std::cout << "Downloading tile "
                          << ZOOM << "/" << tx << "/" << ty << "\n";
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
        "../src/gps/map.png",
        final_img.w,
        final_img.h,
        4,
        final_img.px.data(),
        final_img.w * 4
    );

    std::cout << "Saved map.png\n";
    std::filesystem::remove_all(TILE_DIR);
    return 0;
}
catch (const std::exception& e) {
    std::cerr << e.what() << "\n";
    return 1;
}

// Image code from https://github.com/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples#example-for-opengl-users

// Simple helper function to load an image into a OpenGL texture with common settings
bool LoadTextureFromMemory(const void* data, size_t data_size, GLuint* out_texture, int* out_width, int* out_height)
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

// Open and read a file, then forward to LoadTextureFromMemory()
bool LoadTextureFromFile(const char* file_name, GLuint* out_texture, int* out_width, int* out_height)
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
    bool ret = LoadTextureFromMemory(file_data, file_size, out_texture, out_width, out_height);
    IM_FREE(file_data);
    return ret;
}

namespace DS {
    // this function is used as a callback whenever glfw throws an error.
    void Window::error_callback(const int error_code, const char *description) {
        (void) error_code;
        std::cerr << "GLFW error: " << description << std::endl;
    }

    const char *DEFAULT_NAME = "Delta Station";
    int DEFAULT_WIDTH = 1080;
    int DEFAULT_HEIGHT = 720;

    Window::Window(Dashboard *parent) {
        // Initialize the GLFW library with error checking.
        if (!glfwInit()) {
            std::cerr << "Could not initialize GLFW library." << std::endl;
            exit(2);
        }

        // Set the callback for printing errors to the function above.
        glfwSetErrorCallback(error_callback);

        // Give hints as to which version of OpenGL we should use.
        // TODO: this will need to be tested on tons of platforms before we can ship this comfortably.
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // Initialize the GLFW window, returning a pointer to a handle that lets us use the glfw on our window object.
        back = glfwCreateWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT, DEFAULT_NAME, nullptr, nullptr);
        if (!back) {
            std::cerr << "Could not initialize GLFW window." << std::endl;
            exit(2);
        }

        // Akin to focusing on the current window, but also helps ImGui know which window to attach onto.
        glfwMakeContextCurrent(back);
        // Enables V-Sync (less screen-tearing)
        glfwSwapInterval(1);

        this->parent = parent;

        // ImGui initialization
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImPlot::CreateContext();

        // Lets us use the mouse on the ImGui instances. Return value not needed.
        (void) ImGui::GetIO();

        // Change if you want a different color mode
        ImGui::StyleColorsLight();

        // Initialize ImGui backend features.
        ImGui_ImplGlfw_InitForOpenGL(back, true);
        ImGui_ImplOpenGL3_Init("#version 150");

        this->target_unix_time = static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
    }

    Window::~Window() {
        // Gracefully close all library features used.
        ImPlot::DestroyContext();
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(back);
        glfwTerminate();
    }

    void Window::update() {
        // Query events received from the user, including closing the window.
        glfwPollEvents();

        // Update OpenGL backend information when window is resized.
        int display_w, display_h;
        glfwGetFramebufferSize(back, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        // If the window is closing, signal to Dashboard that we should quit.
        if (this->closing || glfwWindowShouldClose(back)) {
            this->closing = true;
        }

        // Tell ImGui backends that a new frame is being rendered.
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Prepare render buffer for displaying to the screen.
        this->display();

        // Draw render to framebuffer.
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap rendered framebuffer to front.
        glfwSwapBuffers(back);
    }

    std::string Window::gps_error_string(GPSErrorBits b) {
        std::string s{};
        if (b & SDCardFail)
            s += "SD card initialization failure\n";
        if (b & LogFileFail)
            s += "Log file opening failure\n";
        if (b & BadLocation)
            s += "Invalid GPS location\n";
        if (b & BadDateTime)
            s += "Invalid GPS date/time\n";
        return s;
    }

    void Window::imgui_date_time() {
        // TODO: move out of function
        static char buf[9];

        if (ImGui::InputTextWithHint("Input Time", "e.g. 14:30:01", buf, 9)) {
            int hours, minutes, seconds;
            int err = sscanf(buf, "%02d:%02d:%02d", &hours, &minutes, &seconds);
            if (err == 3 && hours < 24 && minutes < 60 && seconds < 60) {
                auto curr = std::chrono::system_clock::now();
                time_t tm = std::chrono::system_clock::to_time_t(curr);
                auto lt = std::localtime(&tm);
                lt->tm_hour = hours;
                lt->tm_min = minutes;
                lt->tm_sec = seconds;

                target_unix_time = static_cast<int>(mktime(lt));
            } else {
                printf("asdf\n");
            }
        }
    }

    std::string Window::motor_error_string(MotorErrorBits b) {
        std::string s{};
        if (b & HardwareOverCurrent)
            s += "Hardware overcurrent\n";
        if (b & SoftwareOvercurrent)
            s += "Software overcurrent\n";
        if (b & BusOvervoltage)
            s += "Bus overvoltage\n";
        if (b & BadHallSequence)
            s += "Bad hall-effect sensor sequence\n";
        if (b & WatchDogCausedLastReset)
            s += "Watch dog caused the last reset\n";
        if (b & ConfigReadError)
            s += "Configuration read error\n";
        if (b & V15RailUnderLockout)
            s += "15V rail under lockout\n";
        if (b & IGBTDesaturation)
            s += "IGBT desaturation\n";
        if (b & AdapterNotPresent)
            s += "Adapter not present\n";
        if (b & MotorOverspeed)
            s += "Motor overspeed\n";
        return s;
    }

    void Window::car_state_window() {
        ImGui::Begin("Car State");

        ImGui::Text("Bitrate: %u", this->parent->bitrate);

        ImGui::End();
    }

    void Window::map_window() {
        ImGui::Begin("Map");

        static int my_image_width = 0;
        static int my_image_height = 0;
        static GLuint my_image_texture = 0;
        
        /*
        std::optional<double> lat_opt = parent->get_value<double>("gps.latitude");
        double lat = 0.0;
        if (lat_opt.has_value()) {
            lat = lat_opt.value();
        } else {
            ImGui::Text("Could not find GPS latitude. Check config and add \"gps.latitude\".");
        }

        std::optional<double> lon_opt = parent->get_value<double>("gps.longitude");
        double lon = 0.0;
        if (lon_opt.has_value()) {
            lon = lon_opt.value();
        } else {
            ImGui::Text("Could not find GPS longitute. Check config and add \"gps.longitude\".");
        }
        */
        
        static double lon = -85.5153;
        static double lat = 37.0389;
        
        //if (lat_opt.has_value() && lon_opt.has_value()) {
            ImGui::Text("Longidude: %f", lon);
            ImGui::Text("Latitude: %f", lat);

            static auto time = std::chrono::system_clock::now() - std::chrono::seconds(5); // prev time is 5 seconds ago so image comes up immediately
            auto currentTime = std::chrono::system_clock::now(); // update every time

            static int return_code = 0; // whether image was generated successfully
            if (currentTime >= time + std::chrono::seconds(5)) {
                if (my_image_texture != 0) // if image loaded
                {
                    glDeleteTextures(1, &my_image_texture); // delete from memory
                    my_image_texture = 0;
                }
                
                time = currentTime;
                lon += 1.0;
                lat += 1.0;

                return_code = make_map(lon, lat); // generate new map image
                
                bool ret = LoadTextureFromFile("../src/gps/map.png", &my_image_texture, &my_image_width, &my_image_height); // load image into memory
                IM_ASSERT(ret);
            }
        
            if (return_code == 1) {
                ImGui::Text("Error generating map. Probably no internet.");
            }

            ImGui::Image((ImTextureID)(intptr_t)my_image_texture, ImVec2(my_image_width, my_image_height));
        //}
        
        ImGui::End();
    }


    void Window::send_data_window() {
        ImGui::Begin("Data Sender");

        ImGui::InputFloat("Target SoC", &this->target_soc, 0.0001, 0.0001, "%.6f");
        this->target_soc = std::clamp(this->target_soc, 0.0f, 1.0f);
        ImGui::InputInt("Target Unix Time", &this->target_unix_time, 1, 1);

        imgui_date_time();

        constexpr int step = 1;
        ImGui::InputScalar("Target Interval", ImGuiDataType_U32, &this->target_interval, &step, &step);

        if (ImGui::Button("Press me!")) {
            parent->send_strategy(this->target_soc, this->target_unix_time, this->target_interval);
        }

        ImGui::End();
    }

    void Window::display() {
        for (auto g: parent->get_graphs()) {
            ImGui::Begin(g.get_name());

            g.display();

            ImGui::End();
        }
        car_state_window();
        map_window();
    }
} // DS
