/* date = May 05, 2025 5:45 PM */


#ifndef WINDOW_H
#define WINDOW_H
#include "GLFW/glfw3.h"

namespace DS {
class Dashboard;

class Window {
public:
    static void error_callback(int error_code, const char * description);

    // TODO; explain 'explicit' keyword
    explicit Window(Dashboard *parent);

    ~Window();

    void update();

    void motor_power_window() const;

    void car_state_window();

    void motor_speed_window() const;

    void display();

    [[nodiscard]] bool should_close() const {
        return closing;
    }

private:
    GLFWwindow *back = nullptr;
    Dashboard *parent{};

    bool closing = false;
    bool show_power_graph = true;
    bool show_speed_graph = true;
};

} // DS

#endif //WINDOW_H
