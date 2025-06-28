/* date = May 05, 2025 5:45 PM */


#ifndef WINDOW_H
#define WINDOW_H

#include <string>

#include "GLFW/glfw3.h"

namespace DS {
class Dashboard;

class Window {
public:
    enum LimiterBits {
        PWM = 1 << 0,
        MotorCurrent = 1 << 1,
        Velocity = 1 << 2,
        BusCurrent = 1 << 3,
        BusVoltageUpperLimit = 1 << 4,
        BusVoltageLowerLimit = 1 << 5,
        Temperature = 1 << 6,
    };

    enum MotorErrorBits {
        HardwareOverCurrent = 1 << 0,
        SoftwareOvercurrent = 1 << 1,
        BusOvervoltage = 1 << 2,
        BadHallSequence = 1 << 3,
        WatchDogCausedLastReset = 1 << 4,
        ConfigReadError = 1 << 5,
        V15RailUnderLockout = 1 << 6,
        IGBTDesaturation = 1 << 7,
        AdapterNotPresent = 1 << 8,
        MotorOverspeed = 1 << 9,
    };

    enum GPSErrorBits {
        SDCardFail = 1 << 0,
        LogFileFail = 1 << 1,
        BadLocation = 1 << 2,
        BadDateTime = 1 << 3,
    };


    static void error_callback(int error_code, const char * description);

    // TODO: explain 'explicit' keyword
    explicit Window(Dashboard *parent);

    ~Window();

    void update();

    void motor_power_window() const;

    void car_state_window();

    void motor_speed_window() const;

    void send_data_window();

    void display();

    [[nodiscard]] bool should_close() const {
        return closing;
    }

    static std::string motor_error_string(MotorErrorBits b);

    static std::string gps_error_string(GPSErrorBits b);

    void imgui_date_time();

private:
    GLFWwindow *back = nullptr;
    Dashboard *parent{};

    bool closing = false;
    bool show_power_graph = true;
    bool show_speed_graph = true;

    float target_soc{};
    int target_unix_time;
    uint32_t target_interval{};
};

} // DS

#endif //WINDOW_H
