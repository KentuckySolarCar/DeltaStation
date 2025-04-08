/* date = April 04, 2025 11:38 AM */


#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <cstdint>
#include <string>
#include <unordered_map>

#include "BufferParser.h"

namespace DS {

class Dashboard {
public:
    Dashboard();

    void print() const;

    void consume(const BufferParser::Buffer & buffer);

private:
    static char REFRESH_SYMBOLS[];

    struct mta_t {
        int32_t millis;
        float voltage, current, speed, odometer, battery_ah;
    } mta{};
    int mta_refresh{};
    struct mtb_t {
        int32_t millis;
        float voltage, current, speed, odometer, battery_ah;
    } mtb{};
    int mtb_refresh{};
    struct gps_t {
        int32_t millis;
        float latitude, longitude, hdop, altitude;
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
