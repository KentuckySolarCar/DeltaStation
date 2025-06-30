/* date = April 04, 2025 11:38 AM */


#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <chrono>
#include <string>
#include <vector>

#include "BufferParser.h"
#include "IOSerial.h"
#include "Window.h"
#include "common.h"

namespace DS {

class Dashboard {
    typedef std::vector<std::pair<double, double>> Graphable;
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

    // Left motor
    struct mta_t {
        int32_t millis;
        float voltage, current, speed, odometer, battery_ah;
    } mta{};
    int mta_refresh{};
    Graphable mta_power_history;

    // Right motor
    struct mtb_t {
        int32_t millis;
        float voltage, current, speed, odometer, battery_ah;
    } mtb{};
    int mtb_refresh{};
    Graphable mtb_power_history;

    size_t mt_data_width;

    struct gps_t {
        int32_t millis;
        // NOTE: this is different from previous delta station
        double latitude, longitude;
        double hdop, altitude;
        uint8_t status;
    } gps{};
    int gps_refresh{};
    // TODO: refactor these names, as they're not descriptive
    struct arr_t {
        int32_t millis;
        float a1, a2;
        float a1_power, a2_power;
    } arr{};
    int arr_refresh{};
    Graphable a1_history;
    Graphable a2_history;
    Graphable a1_power_history;
    Graphable a2_power_history;
    size_t arr_data_width;

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
        // NOTE: ten_v_bus is actually at 12 VOLTS
        float steering, ten_v_bus;
        uint32_t lap_time;
    } drv{};
    int drv_refresh{};
    Graphable throttle_history, regen_history;
    size_t driver_input_width;

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
