/* date = May 05, 2025 5:45 PM */


#ifndef WINDOW_H
#define WINDOW_H

#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "GLFW/glfw3.h"

namespace DS {
class Dashboard;

class Window {
    typedef std::vector<std::pair<double, double>> Graphable;
public:
    /**
     * Enumeration used for limiters from the motor controller.
     * TODO: what is a limiter?
     */
    enum LimiterBits {
        PWM = 1 << 0,
        MotorCurrent = 1 << 1,
        Velocity = 1 << 2,
        BusCurrent = 1 << 3,
        BusVoltageUpperLimit = 1 << 4,
        BusVoltageLowerLimit = 1 << 5,
        Temperature = 1 << 6,
    };

    /**
     * Enumeration used for errors from the motor controller.
     */
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

    /**
     * Enumeration used for GPS and SD card errors.
     */
    enum GPSErrorBits {
        SDCardFail = 1 << 0,
        LogFileFail = 1 << 1,
        BadLocation = 1 << 2,
        BadDateTime = 1 << 3,
    };

    /**
     * Constructor for the window instance.
     * BUG: this constructor should only be called once, as it calls library
     * initialization code. Eventually we will move to a Singleton design
     * pattern as delta station doesn't need multiple windows.
     * @param parent The Dashboard instance to which this window will be
     * attached.
     */
    // TODO: explain 'explicit' keyword
    explicit Window(Dashboard *parent);

    ~Window();

    // called every frame, regenerates ImGui windows.
    void update();

    void display();

    [[nodiscard]] bool should_close() const {
        return closing;
    }

    void app_state_window();
    void car_state_window();
    void map_window();
    void send_data_window();

    static std::string motor_error_string(MotorErrorBits b);

    static std::string gps_error_string(GPSErrorBits b);

    static void graph_vectors(const char *names[], Graphable *vecs[], size_t count, double data_width);

    void imgui_date_time();

    // variables for controlling app state window
    std::optional<std::string> selected_path = std::nullopt;
    std::mutex path_write_lock;

    Dashboard *parent{};

private:
    // error callback used by GLFW when an error occurs on its backend.
    static void error_callback(int error_code, const char * description);

    GLFWwindow *back = nullptr;

    bool closing = false;

    float target_soc{};
    int target_unix_time;
    uint32_t target_interval{};

    // determines whether constructor has completed. useful for knowing if ImGUI has fully initialized or not during construction.
    bool init = false;

    // used by the map_window function call to properly handle map generation semantics
    std::mutex map_generate_lock;
};

} // DS

#endif //WINDOW_H
