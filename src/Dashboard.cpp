/* date = April 04, 2025 11:38 AM */


#include "Dashboard.h"

#include <iostream>

#include "Window.h"

namespace DS {
    Dashboard::Dashboard() {
        mta_power_history.resize(MAX_MOTOR_HISTORY);
        mtb_power_history.resize(MAX_MOTOR_HISTORY);

        start_time = std::chrono::high_resolution_clock::now();
        window = new Window(this);
    }

    Dashboard::~Dashboard() {
        std::cout << "Exiting...\n";
    }

    void Dashboard::print(std::ostream &buf) const {
        using namespace std;

        buf << REFRESH_SYMBOLS[mta_refresh] << " Motor A: \n";
        buf
                << "    Voltage: " << mta.voltage << "\n"
                << "    Current: " << mta.current << "\n"
                << "    Speed: " << mta.speed << "\n"
                << "    Odometer: " << mta.odometer << "\n"
                << "    Battery Ah: " << mta.battery_ah << "\n";

        buf << REFRESH_SYMBOLS[mtb_refresh] << " Motor B: \n";
        buf
                << "    Voltage: " << mtb.voltage << "\n"
                << "    Current: " << mtb.current << "\n"
                << "    Speed: " << mtb.speed << "\n"
                << "    Odometer: " << mtb.odometer << "\n"
                << "    Battery Ah: " << mtb.battery_ah << "\n";

        buf << REFRESH_SYMBOLS[gps_refresh] << " GPS: \n";
        buf
                << "    Latitude: " << gps.latitude << "\n"
                << "    Longitude: " << gps.longitude << "\n"
                << "    HDOP: " << gps.hdop << "\n"
                << "    Altitude: " << gps.altitude << "\n";

        buf << REFRESH_SYMBOLS[arr_refresh] << " Array: \n";
        buf
                << "    1: " << arr.a1 << "\n"
                << "    2: " << arr.a2 << "\n"
                << "    3: " << arr.a3 << "\n"
                << "    4: " << arr.a4 << "\n"
                << "    5: " << arr.a5 << "\n"
                << "    6: " << arr.a6 << "\n";

        buf << REFRESH_SYMBOLS[bat_refresh] << " Battery: \n";
        buf
                << "    Max Voltage: " << bat.max_v << "\n"
                << "    Min Voltage: " << bat.min_v << "\n"
                << "    Avg Voltage: " << bat.avg_v << "\n"
                << "    Current: " << bat.current << "\n"
                << "    Max Temperature: " << bat.max_t << "\n"
                << "    Min Temperature: " << bat.min_t << "\n"
                << "    Avg Temperature: " << bat.avg_t << "\n"
                << "    State of Charge: " << bat.soc << "\n";

        buf << REFRESH_SYMBOLS[drv_refresh] << " Driver Input: \n";
        buf
                << "    Throttle Percent: " << drv.throt_pct << "\n"
                << "    Regen Percent: " << drv.regen_pct << "\n"
                << "    Raw Throttle: " << drv.throt_raw << "\n"
                << "    Raw Regen: " << drv.regen_raw << "\n"
                << "    Steering: " << drv.steering << "\n"
                << "    Ten-Volt Bus: " << drv.ten_v_bus << "\n";

        buf << REFRESH_SYMBOLS[sta_refresh] << " Status: \n";
        buf
                << "    Left: " << sta.left << "\n"
                << "    Right: " << sta.right << "\n"
                << "    Log: " << sta.log << "\n";
        buf << "Bitrate: " << this->bitrate << "\n";
    }

    void Dashboard::consume(const BufferParser::Buffer &buffer) {
        auto time = std::chrono::high_resolution_clock::now();
        switch (buffer.type) {
            case BufferParser::UndefinedMessage: {
                std::cerr << "Undefined message found!" << "\n";
            }
            break;
            case BufferParser::LeftMotorMessage: {
                memcpy(&mta, &buffer.data[0], sizeof(mta));
                mta.millis = buffer.timestamp;
                if (++mta_refresh > 3) mta_refresh = 0;

                mta_power_history.emplace_back(static_cast<double>((time - start_time).count()) / (1e9),
                                               mta.current * mta.voltage);
            }
            break;
            case BufferParser::RightMotorMessage: {
                memcpy(&mtb, &buffer.data[0], sizeof(mtb));
                mtb.millis = buffer.timestamp;
                if (++mtb_refresh > 3) mtb_refresh = 0;

                mtb_power_history.emplace_back(static_cast<double>((time - start_time).count()) / (1e9),
                                               mtb.current * mtb.voltage);
            }
            break;
            case BufferParser::GpsMessage: {
                size_t offset = 0;
                memcpy(&gps.millis, &buffer.data[offset], sizeof(gps.millis));

                offset += sizeof(gps.millis);
                memcpy(&gps.latitude, &buffer.data[offset], sizeof(gps.latitude) + sizeof(gps.longitude));

                offset += sizeof(gps.latitude) + sizeof(gps.longitude);
                memcpy(&gps.hdop, &buffer.data[offset], sizeof(gps.hdop) + sizeof(gps.altitude));

                if (++gps_refresh > 3) gps_refresh = 0;
            }
            break;
            case BufferParser::MpptMessage: {
                memcpy(&arr, &buffer.data[0], sizeof(arr));
                if (++arr_refresh > 3) arr_refresh = 0;
            }
            break;
            case BufferParser::BatteryMessage: {
                // due to padding issues, we need to set elements explicitly here
                memcpy(&bat.millis, &buffer.data[0], sizeof(int32_t));
                memcpy(&bat.max_v, &buffer.data[offsetof(bat_t, max_v)], sizeof(float));
                memcpy(&bat.min_v, &buffer.data[offsetof(bat_t, min_v)], sizeof(float));
                memcpy(&bat.avg_v, &buffer.data[offsetof(bat_t, avg_v)], sizeof(float));
                memcpy(&bat.current, &buffer.data[offsetof(bat_t, current)], sizeof(float));
                memcpy(&bat.soc, &buffer.data[offsetof(bat_t, soc)], sizeof(float));
                memcpy(&bat.max_t, &buffer.data[offsetof(bat_t, max_t)], sizeof(int16_t));
                memcpy(&bat.min_t, &buffer.data[offsetof(bat_t, min_t)], sizeof(int16_t));
                memcpy(&bat.avg_t, &buffer.data[offsetof(bat_t, avg_t)], sizeof(int16_t));

                if (++bat_refresh > 3) bat_refresh = 0;
            }
            break;
            case BufferParser::DriverInputMessage: {
                memcpy(&drv, &buffer.data[TIME_OFFSET], sizeof(drv));
                if (++drv_refresh > 3) drv_refresh = 0;
            }
            break;
            case BufferParser::StatusMessage: {
                memcpy(&sta, &buffer.data[TIME_OFFSET], sizeof(sta));
                if (++sta_refresh > 3) sta_refresh = 0;
            }
            break;
            case BufferParser::SensorMessage: {
                std::cerr << "Sensor message not supported right now." << "\n";
            }
            break;
        }

    }

    void Dashboard::update() {
        window->update();

        this->closing = window->should_close();

        auto time = std::chrono::system_clock::now();
        this->dt = static_cast<double>((time - prev_time).count()) / 1e9;

        if (static_cast<double>((time - this->second_mark).count()) >= 1e9) {
            this->second_mark = time;
            this->bitrate = this->bytes_read * 8;
            this->bytes_read = 0;
        }
        prev_time = time;
    }

    char Dashboard::REFRESH_SYMBOLS[] = {'|', '/', '-', '\\'};
} // DS
