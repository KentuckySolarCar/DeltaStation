/* date = May 05, 2025 5:45 PM */

// Tutorials/References used for writing this file:
// https://www.glfw.org/docs/latest/quick_guide.html for glfw boilerplate
// https://github.com/ocornut/imgui/blob/master/examples/example_glfw_opengl3/main.cpp for imgui with glfw
// https://github.com/epezent/implot/blob/master/implot.h for implot docs

#include "Window.h"

#include <iostream>

#include "Dashboard.h"
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

    const char * DEFAULT_NAME = "Delta Station";
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

        // Akin to focusing on the current window, but also helps IGui know which window to attach onto.
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

    void Window::car_state_window() {
        ImGui::Begin("Car State");

        //auto buf = std::ostringstream();
        //parent->print(buf);

        if (ImGui::TreeNode("Motors:")) {
            ImGui::Text("Left Motor Voltage: %f V", parent->mta.voltage);
            ImGui::SameLine();
            ImGui::Text("Right Motor Voltage: %f V", parent->mtb.voltage);

            ImGui::Text("Left Motor Current: %f A", parent->mta.current);
            ImGui::SameLine();
            ImGui::Text("Right Motor Current: %f A", parent->mtb.current);

            ImGui::Checkbox("Show Power Graph", &this->show_power_graph);

            ImGui::Checkbox("Show Speed Graph", &this->show_speed_graph);

            ImGui::TreePop();
        }
        if (ImGui::TreeNode("GPS:")) {
            ImGui::Text("Position: %f N %f W", parent->gps.latitude, parent->gps.longitude);

            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Arrays:")) {
            ImGui::Text("TODO");
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Batteries:")) {
            ImGui::Text("TODO");
            ImGui::ProgressBar(parent->bat.soc);
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Driver Inputs:")) {
            ImGui::Text("TODO");
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Status:")) {
            ImGui::Text("TODO");
            ImGui::TreePop();
        }

        //ImGui::Text("%s", buf.str().c_str());
        ImGui::End();
    }

    void Window::motor_speed_window() const {
        ImGui::Begin("Motor Speed Data");

        if (ImPlot::BeginPlot("Motor Speed Graph")) {
            ImPlot::PlotBars("Left Motor Speed:", &parent->mta.speed, 1);
            ImPlot::PlotBars("Right Motor Speed:", &parent->mtb.speed, 1);
            ImPlot::EndPlot();
        }

        ImGui::End();
    }

    void Window::motor_power_window() const {
        ImGui::Begin("Motor Power Consumption Data");

        auto &mta = parent->mta_power_history;

        std::vector<double> x_left;
        std::vector<double> y_left;
        for (auto &[xd, yd] : mta) {
            x_left.push_back(static_cast<double>(xd));
            y_left.push_back(yd);
        }

        auto &mtb = parent->mtb_power_history;

        std::vector<double> x_right;
        std::vector<double> y_right;
        for (auto &[xd, yd] : mtb) {
            x_right.push_back(static_cast<double>(xd));
            y_right.push_back(yd);
        }

        ImPlot::SetNextAxesToFit();
        if (ImPlot::BeginPlot("Motor Power Graph")) {
            ImPlot::SetupAxes("Time Since Startup (seconds)", "Power Consumption (Watts)");
            ImPlot::PlotLine(
                "Left Motor Power Usage:",
                x_left.data(),
                y_left.data(),
                static_cast<int>(x_left.size())
            );
            ImPlot::PlotLine(
                "Right Motor Power Usage:",
                x_right.data(),
                y_right.data(),
                static_cast<int>(x_right.size())
            );
            ImPlot::EndPlot();
        }
        ImGui::End();
    }

    void Window::display() {
        car_state_window();
        if (show_power_graph)
            motor_power_window();
        if (show_speed_graph)
            motor_speed_window();
    }
} // DS
