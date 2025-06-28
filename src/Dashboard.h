/* date = April 04, 2025 11:38 AM */


#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <chrono>
#include <string>
#include <vector>

#include "BufferParser.h"
#include "IOSerial.h"
#include "Window.h"

namespace DS {

class Dashboard {
public:
    /**
     * This constructor f
     */
    Dashboard();
    ~Dashboard();

    void print(std::ostream &buf) const;

    void consume(const BufferParser::Buffer &buffer);

    void byte_increment() {
        this->bytes_read++;
    }

    [[nodiscard]] bool should_close() const {
        return closing;
    }

    void update();

    void send_strategy(float target_soc, int target_unix_time, uint32_t uint32);

    IOSerial *serial;

private:
    static char REFRESH_SYMBOLS[];
    static constexpr size_t MAX_MOTOR_HISTORY = 60;
    static constexpr size_t MAX_VISIBLE_POWER = 100;

    // Left motor
    struct mta_t {
        int32_t millis;
        float voltage, current, speed, odometer, battery_ah;
    } mta{};
    int mta_refresh{};
    std::vector<std::pair<double, double>> mta_power_history;

    // Right motor
    struct mtb_t {
        int32_t millis;
        float voltage, current, speed, odometer, battery_ah;
    } mtb{};
    int mtb_refresh{};
    std::vector<std::pair<double, double>> mtb_power_history;

    struct gps_t {
        int32_t millis;
        // NOTE: this is different from previous delta station
        double latitude, longitude;
        double hdop, altitude;
        uint8_t status;
    } gps{};
    int gps_refresh{};
    struct arr_t {
        int32_t millis;
        float a1, a2, a3, a4, a5, a6;
    } arr{};
    int arr_refresh{};
    struct bat_t {
        int32_t millis;
        float max_v, min_v, avg_v, current;
        float soc;
        int16_t max_t, min_t, avg_t; // 2 bytes of padding
    } bat{};
    int bat_refresh{};
    struct drv_t {
        int32_t millis;
        float throt_pct, regen_pct;
        int32_t throt_raw, regen_raw;
        float steering, ten_v_bus;
    } drv{};
    int drv_refresh{};
    struct sta_t {
        int32_t millis;
        int32_t left, right, log;
    } sta{};
    int sta_refresh{};

    std::chrono::system_clock::time_point start_time;

    std::chrono::system_clock::time_point prev_time;
    std::chrono::system_clock::time_point second_mark;

    double dt{};

    uint32_t bytes_read{};
    uint32_t bitrate{};
    Window *window = nullptr;

    bool closing = false;

    // TODO: we have a duplicate RS encoder in BufferParser.h. Consider using that one??
    RS::ReedSolomon<MSG_LENGTH, ECC_LENGTH> foo{};

    friend class Window;
};

} // DS

#endif //DASHBOARD_H
