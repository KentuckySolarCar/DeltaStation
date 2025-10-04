/* date = May 05, 2025 5:45 PM */

// Tutorials/References used for writing this file:
// https://www.glfw.org/docs/latest/quick_guide.html for glfw boilerplate
// https://github.com/ocornut/imgui/blob/master/examples/example_glfw_opengl3/main.cpp for imgui with glfw
// https://github.com/epezent/implot/blob/master/implot.h for implot docs

#include "Window.h"

#include <iostream>
#include <ranges>
#include <algorithm>

#include "Dashboard.h"
#include "Graph.h"
#include "imgui.h"
#include "implot.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

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
    }
} // DS
