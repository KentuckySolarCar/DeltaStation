/* date = April 04, 2025 11:38 AM */


#include "Dashboard.h"

#include <filesystem>
#include <iostream>

#include "Graph.h"
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
        (void)buf;
    }

    std::string Dashboard::id_name(size_t type) const {
        return config->get_id(type).value_or("");
    }

    void Dashboard::update_plots() {
        for (auto &g : this->config->graphs) {
            g.update(*this, (std::chrono::system_clock::now() - start_time).count() / 1e9);
        }
        // pseudocode:
        // for each graph, evaluate them with new collected data
        // store new y values with x values
        // update array of data plots
        // regenerate ImGui plots for each plot that is enabled.
    }

    void Dashboard::consume(const BufferParser::Buffer &buffer) {
        constexpr size_t DATA_OFFSET = 4;
        // TODO: semantics of dropped packet...
        if (!this->config.has_value()) return;
        const auto *entry = this->config->get(id_name(buffer.type));
        if (!entry) {
            std::cerr << "Undefined message found! Unknown id: " << static_cast<uint32_t>(buffer.type) << "\n";
            return;
        }
        memcpy(entry->as_ptr(), &buffer.data[DATA_OFFSET], entry->get_size());
    }

    void Dashboard::update() {
        window->update();

        if (write_lock.try_lock()) {
            if (this->config.has_value())
                update_plots();
            write_lock.unlock();
        }

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

        rs.Encode(data, encoded);

        serial->put_bytes(reinterpret_cast<const char *>(encoded), BUFFER_LENGTH);
    }

    void Dashboard::set_config(const std::string &path) {
        if (std::filesystem::exists(path))
            this->config = Config(path);
        else
            this->config = std::nullopt;
    }
    
    std::optional<std::string> Dashboard::get_config_path() {
        if (this->config.has_value()) {
            return this->config->config_path;
        }
        return std::nullopt;
    }

    void Dashboard::debug_print_packet_ids() {
        if (this->config.has_value()) {
            this->config->config["ds"].as_table()->for_each([](const toml::key &key, const toml::table &val) {
                std::cout << key << ": " << val["id"] << '\n';
            });
        }
    }

    bool Dashboard::has_key(const std::string &ident) const {
        // TODO: what if ident doesn't have '.'?
        const auto buffer_name = ident.substr(0, ident.find('.'));
        const auto field_name = ident.substr(ident.find('.')+1);
        return (*this->config)[buffer_name].get_value<void *>(field_name).has_value();
    }
} // DS
