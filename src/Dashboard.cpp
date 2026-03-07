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
            if (this->config.has_value()) {
                update_plots();
                for (auto entries : this->config->id_name_pairs) {
                    // needed due to const qualification
                    std::string name = entries.first;
                    dump_entry(name, entries.second);
                }
            }
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
        if (std::filesystem::exists(path)) {
            this->config = Config(path);
            this->init_csv_storage();
        }
        else {
            this->config = std::nullopt;
        }
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

    static std::optional<uint32_t> init_timestamp = std::nullopt;
    std::filesystem::path Dashboard::get_csv_storage_path() {
        std::string debug_opt = "";
        if (this->debug_mode) {
            debug_opt = "_debug";
        }
        std::filesystem::path storage_path{"./csv_storage" + debug_opt};

        if (!init_timestamp.has_value()) {
            init_timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count();
        }

        storage_path /= std::to_string(*init_timestamp);
        storage_path /= config->config_name;
        std::filesystem::create_directories(storage_path);

        return storage_path;

    }
    void Dashboard::init_csv_storage() {
        for (auto id_name_pair : config->id_name_pairs) {
            std::string filename = id_name_pair.first + ".csv";
            std::vector<std::string> headers;

            std::filesystem::path p{filename};
            std::ofstream out{get_csv_storage_path() / p};

            int field_counter = 0;
            for (auto name_idx_pairs : id_name_pair.second.get_fields()) {
                out << name_idx_pairs.first;

                field_counter++;
                if (field_counter <= id_name_pair.second.get_fields().size()) {
                    out << ',';
                }
            }
            out << "unix_timestamp";
            out << std::endl;

            out.close();
        }
    }

    void dump_field(std::ofstream &out, std::string &name, Config::Entry &e) {
        switch (e.get(name)->ty) {
            case Config::I8: out << *e.get_value<int8_t>(name); break;
            case Config::I16: out << *e.get_value<int16_t>(name); break;
            case Config::I32: out << *e.get_value<int32_t>(name); break;
            case Config::I64: out << *e.get_value<int64_t>(name); break;
            case Config::U8: out << *e.get_value<uint8_t>(name); break;
            case Config::U16: out << *e.get_value<uint16_t>(name); break;
            case Config::U32: out << *e.get_value<uint32_t>(name); break;
            case Config::U64: out << *e.get_value<uint64_t>(name); break;
            case Config::F32: out << *e.get_value<float>(name); break;
            case Config::F64: out << *e.get_value<double>(name); break;
            default: break;
        }
    }

    void Dashboard::dump_entry(std::string &name, Config::Entry &e) {
        std::string filename = name + ".csv";
        std::vector<std::string> headers;

        std::filesystem::path p{filename};

        std::ofstream out{get_csv_storage_path() / p, std::ios_base::app};

        int field_counter = 0;
        for (auto name_idx_pairs : e.get_fields()) {
            std::string s = name_idx_pairs.first;
            dump_field(out, s, e);

            field_counter++;
            if (field_counter <= e.get_fields().size()) {
                out << ',';
            }
        }
        out << std::chrono::duration_cast<std::chrono::seconds>(
                   std::chrono::system_clock::now().time_since_epoch())
                   .count();
        out << std::endl;

        out.close();
    }
} // DS
