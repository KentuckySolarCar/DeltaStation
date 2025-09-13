/* date = April 04, 2025 11:38 AM */


#include "Dashboard.h"

#include <iostream>

#include "Window.h"

namespace DS {
    Dashboard::Dashboard() {
        start_time = std::chrono::system_clock::now();
        window = new Window(this);
    }

    Dashboard::~Dashboard() {
        std::cout << "Exiting...\n";
    }

    void Dashboard::print(std::ostream &buf) const {
        using namespace std;

        buf
                << "    Voltage: " << *config["mta"].get_value<float>("voltage") << "\n";
        /*
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
                << "    Power In 1: " << arr.a1_power << "\n"
                << "    Power In 2: " << arr.a2_power << "\n";

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
        */
    }

    void Dashboard::consume(const BufferParser::Buffer &buffer) {
        constexpr size_t DATA_OFFSET = 4;
        const auto *entry = this->config.get(buffer.type);
        if (!entry) {
            std::cerr << "Undefined message found! Unknown id: " << static_cast<uint32_t>(buffer.type) << "\n";
            return;
        }
        memcpy(entry->as_ptr(), &buffer.data[DATA_OFFSET], entry->get_size());

        // TODO: register fields to plot
        // update_plots(buffer);
    }

    void Dashboard::update_log() const {
    }

    void Dashboard::update() {
        window->update();
        update_log();

        this->closing = window->should_close();

        auto time = std::chrono::system_clock::now();
        this->dt = static_cast<double>((time - prev_time).count()) / 1e9;

        if (static_cast<double>((time - this->second_mark).count()) >= 1e9) {
            this->second_mark = time;
            this->bitrate = this->bytes_read * 8;
            this->bytes_read = 0;
        }
        prev_time = time;

        std::cout << *config["mta"].get_value<float>("voltage") << "\n";
    }

    void Dashboard::send_strategy(float target_soc, int target_unix_time, uint32_t target_interval) {
        // data format for telemetry:
        // header (4 bytes): "GCS " (space)
        // target_soc (4 bytes) target_unix_time (4 bytes) target_interval (4 bytes)
        // footer (4 bytes): "UKSC"
        // padding (20 bytes)
        // RS-FEC (12 bytes)
        // total: 52 bytes
        uint8_t data[40];
        uint8_t encoded[52];
        int offset = 0;

        memcpy(data + offset, "GCS ", 4);
        offset += 4;
        memcpy(data + offset, &target_soc, sizeof(target_soc));
        offset += sizeof(target_soc);
        memcpy(data + offset, &target_unix_time, sizeof(target_unix_time));
        offset += sizeof(target_unix_time);
        memcpy(data + offset, &target_interval, sizeof(target_interval));
        offset += sizeof(target_interval);

        memcpy(data + offset, "UKSC", 4);
        offset += 4;
        memset(data + offset, 0, 40 - offset);
        //offset = 40;

        foo.Encode(data, encoded);

        serial->put_bytes(reinterpret_cast<const char *>(encoded), BUFFER_LENGTH);
    }

    void Dashboard::set_config(const std::string &path) {
        this->config = Config(path);
    }

    void Dashboard::debug_print_packet_ids() {
        this->config.config["ds"].as_table()->for_each([](const toml::key &key, const toml::table &val) {
            std::cout << key << ": " << val["id"] << '\n';
        });
    }
} // DS
