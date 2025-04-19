/* date = April 04, 2025 11:38 AM */


#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include <gnuplot-iostream.h>

#include "BufferParser.h"

namespace DS {

class Dashboard {
public:
    Dashboard();
    ~Dashboard();

    void print() const;
    void display();

    void consume(const BufferParser::Buffer &buffer);

private:
    static char REFRESH_SYMBOLS[];
    static constexpr size_t MAX_MOTOR_HISTORY = 60;
    static constexpr size_t MAX_VISIBLE_POWER = 100;

    Gnuplot gp;

    // Left motor
    struct mta_t {
        int32_t millis;
        float voltage, current, speed, odometer, battery_ah;
    } mta{};
    int mta_refresh{};
    int mta_data_position = 0;
    std::vector<std::tuple<float, float>> mta_power_history;

    // Right motor
    struct mtb_t {
        int32_t millis;
        float voltage, current, speed, odometer, battery_ah;
    } mtb{};
    int mtb_refresh{};
    int mtb_data_position = 0;
    std::vector<std::tuple<float, float>> mtb_power_history;

    struct gps_t {
        int32_t millis;
        // NOTE: this is different from previous delta station
        double latitude, longitude;
        float hdop, altitude;
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
        int16_t max_t, min_t, avg_t; // 2 bytes of padding
        float soc;
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
};

} // DS

#endif //DASHBOARD_H
