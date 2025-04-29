/* date = April 04, 2025 11:38 AM */


#include "Dashboard.h"

#include <iostream>

namespace DS {
    // Which window manager to use for displaying live graphs. Tested options include: wxt, x11
    static const char *WINDOW_MANAGER = "wxt";

    Dashboard::Dashboard() {
        // TODO: configure this line for cross-platform; what other window managers should we account for?
        gp << "set term " << WINDOW_MANAGER << " 1 noraise\n";

        mta_power_history.resize(MAX_MOTOR_HISTORY);
        mtb_power_history.resize(MAX_MOTOR_HISTORY);

        start_time = std::chrono::high_resolution_clock::now();
    }

    Dashboard::~Dashboard() {
        printf("Exiting...\n");
        gp << "unset output\n";
        gp << "exit gnuplot\n";
    }

    void Dashboard::print() const {
        using namespace std;

#if defined(_WIN32) || defined(_WIN64)
        system("cls");
#else
        system("clear");
#endif

        cout << REFRESH_SYMBOLS[mta_refresh] << " Motor A: \n";
        cout
                << "    Voltage: " << mta.voltage << "\n"
                << "    Current: " << mta.current << "\n"
                << "    Speed: " << mta.speed << "\n"
                << "    Odometer: " << mta.odometer << "\n"
                << "    Battery Ah: " << mta.battery_ah << "\n";

        cout << REFRESH_SYMBOLS[mtb_refresh] << " Motor B: \n";
        cout
                << "    Voltage: " << mtb.voltage << "\n"
                << "    Current: " << mtb.current << "\n"
                << "    Speed: " << mtb.speed << "\n"
                << "    Odometer: " << mtb.odometer << "\n"
                << "    Battery Ah: " << mtb.battery_ah << "\n";

        cout << REFRESH_SYMBOLS[gps_refresh] << " GPS: \n";
        cout
                << "    Latitude: " << gps.latitude << "\n"
                << "    Longitude: " << gps.longitude << "\n"
                << "    HDOP: " << gps.hdop << "\n"
                << "    Altitude: " << gps.altitude << "\n";

        cout << REFRESH_SYMBOLS[arr_refresh] << " Array: \n";
        cout
                << "    1: " << arr.a1 << "\n"
                << "    2: " << arr.a2 << "\n"
                << "    3: " << arr.a3 << "\n"
                << "    4: " << arr.a4 << "\n"
                << "    5: " << arr.a5 << "\n"
                << "    6: " << arr.a6 << "\n";

        cout << REFRESH_SYMBOLS[bat_refresh] << " Battery: \n";
        cout
                << "    Max Voltage: " << bat.max_v << "\n"
                << "    Min Voltage: " << bat.min_v << "\n"
                << "    Avg Voltage: " << bat.avg_v << "\n"
                << "    Current: " << bat.current << "\n"
                << "    Max Temperature: " << bat.max_t << "\n"
                << "    Min Temperature: " << bat.min_t << "\n"
                << "    Avg Temperature: " << bat.avg_t << "\n"
                << "    State of Charge: " << bat.soc << "\n";

        cout << REFRESH_SYMBOLS[drv_refresh] << " Driver Input: \n";
        cout
                << "    Throttle Percent: " << drv.throt_pct << "\n"
                << "    Regen Percent: " << drv.regen_pct << "\n"
                << "    Raw Throttle: " << drv.throt_raw << "\n"
                << "    Raw Regen: " << drv.regen_raw << "\n"
                << "    Steering: " << drv.steering << "\n"
                << "    Ten-Volt Bus: " << drv.ten_v_bus << "\n";

        cout << REFRESH_SYMBOLS[sta_refresh] << " Status: \n";
        cout
                << "    Left: " << sta.left << "\n"
                << "    Right: " << sta.right << "\n"
                << "    Log: " << sta.log << "\n";
        cout << "Bitrate: " << this->bitrate << "\n";
    }

    void Dashboard::display() {
        if (!mta_power_history.empty()) {
            const int data_high = static_cast<int>(mta_power_history.back().first);
            const int data_low = data_high - 100;
            gp << "set xrange [" << data_low << ":" << data_high << "]\nset yrange [0:" << MAX_VISIBLE_POWER << "]\n";
            gp << "set term " << WINDOW_MANAGER << " 0\n";
            gp << "plot '-' using 1:2 with lines title 'Left Motor Power'\n";

            gp.send1d(mta_power_history);
        }
        if (!mtb_power_history.empty()) {
            const int data_high = static_cast<int>(mtb_power_history.back().first);
            const int data_low = data_high - 100;
            gp << "set xrange [" << data_low << ":" << data_high << "]\nset yrange [0:" << MAX_VISIBLE_POWER << "]\n";
            gp << "set term " << WINDOW_MANAGER << " 1\n";
            gp << "plot '-' using 1:2 with lines title 'Right Motor Power'\n";

            gp.send1d(mtb_power_history);
        }
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
                memcpy(&bat.max_t, &buffer.data[offsetof(bat_t, max_t)], sizeof(int16_t));
                memcpy(&bat.min_t, &buffer.data[offsetof(bat_t, min_t)], sizeof(int16_t));
                memcpy(&bat.avg_t, &buffer.data[offsetof(bat_t, avg_t)], sizeof(int16_t));
                memcpy(&bat.soc, &buffer.data[offsetof(bat_t, soc) - 2], sizeof(float));

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

    char Dashboard::REFRESH_SYMBOLS[] = {'|', '/', '-', '\\'};
} // DS
