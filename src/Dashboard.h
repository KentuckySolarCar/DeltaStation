/* date = April 04, 2025 11:38 AM */


#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <chrono>
#include <vector>

#include "BufferParser.h"
#include "IOSerial.h"
#include "Window.h"
#include "common.h"

#include <toml++/toml.hpp>

#include "Config.h"

namespace DS {

class Dashboard {
    typedef std::vector<std::pair<double, double>> Graphable;
public:
    /**
     * This constructor initializes an empty window. Most of the graphics initialization is handled
     * in Window's constructor.
     */
    Dashboard();
    ~Dashboard();

    /**
     * Dump contents of the buffers in config to an output stream. Use `std::cout` as the output
     * stream here if you want to dump contents to the console.
     * @param buf output stream to dump contents to.
     */
    void print(std::ostream &buf) const;

    [[nodiscard]] std::string id_name(size_t type) const;

    void update_plots();

    /**
     * Accepts a prepared buffer from the buffer parser and updates the corresponding ID data
     * accordingly.
     * @param buffer buffer from BufferParser class to consume.
     */
    void consume(const BufferParser::Buffer &buffer);

    // Note: used by main function for tracking bitrate of data received by DeltaStation.
    void byte_increment() {
        this->bytes_read++;
    }

    /**
     * States if window instance is actively closing, i.e. if close button was pressed by user or
     * if the `Alt+F4` keystroke was pressed.
     * @return If window is closing.
     */
    [[nodiscard]] bool should_close() const {
        return closing;
    }

    void update();

    void send_strategy(float target_soc, int target_unix_time, uint32_t uint32);

    void set_config(const std::string &path);

    void debug_print_packet_ids();

    [[nodiscard]] bool has_key(const std::string &ident) const;

    template<typename T>
    [[nodiscard]] std::optional<T> get_value(const std::string &ident) const {
        // Every identity MUST be of the form "{buffer_name}.{field_name}"
        const std::string buffer_name = ident.substr(0, ident.find('.'));
        const std::string field_name = ident.substr(ident.find('.')+1);
        const Config::Entry *entry = this->config.get(buffer_name);

        if (!entry) return std::nullopt;

        const std::optional<Config::Field> field = entry->get(field_name);
        if (field.has_value()) {
            const auto tn = Config::field_type_to_str(field->ty);
            if (!tn) return std::nullopt;

            if (type_str<T>() == *tn) {
                // mem transmute!
                T ret;
                memcpy(&ret, field->data, sizeof(ret));
                return ret;
            }
        }

        return std::nullopt;
    }

    template<typename T>
    [[nodiscard]] T get_value_unchecked(const std::string &ident) const {
        // Every identity MUST be of the form "{buffer_name}.{field_name}"
        const std::string buffer_name = ident.substr(0, ident.find('.'));
        const std::string field_name = ident.substr(ident.find('.')+1);
        const Config::Entry *entry = this->config.get(buffer_name);
        if (!entry) {
            T ret;
            memset(&ret, 0, sizeof(ret));
            return ret;
        }
        const std::optional<Config::Field> field = entry->get(field_name);
        if (field.has_value()) {
            T ret;
            memcpy(&ret, field->data, sizeof(ret));
            return ret;
        }
        T ret;
        memset(&ret, 0, sizeof(ret));
        return ret;
    }

    IOSerial *serial{};

private:
    std::vector<Graph> &get_graphs() {
        return config.graphs;
    }

    template<typename T>
    static constexpr const char *type_str() {
        if constexpr (std::same_as<T, int8_t>) {
            return "i8";
        } else if constexpr (std::same_as<T, int16_t>) {
            return "i16";
        } else if constexpr (std::same_as<T, int32_t>) {
            return "i32";
        } else if constexpr (std::same_as<T, int64_t>) {
            return "i64";
        } else if constexpr (std::same_as<T, uint8_t>) {
            return "u8";
        } else if constexpr (std::same_as<T, uint16_t>) {
            return "u16";
        } else if constexpr (std::same_as<T, uint32_t>) {
            return "u32";
        } else if constexpr (std::same_as<T, uint64_t>) {
            return "u64";
        } else if constexpr (std::same_as<T, float>) {
            return "f32";
        } else if constexpr (std::same_as<T, double>) {
            return "f64";
        } else {
            static_assert(false, "Bad type!");
            return "";
        }
    }

    // time-keeping
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point prev_time;
    std::chrono::system_clock::time_point second_mark;
    double dt{};

    // bitrate
    uint32_t bytes_read{};
    uint32_t bitrate{};

    // window management
    Window *window = nullptr;
    bool closing = false;

    // TODO: we have a duplicate RS encoder in BufferParser.h. Consider using that one??
    RS::ReedSolomon<MSG_LENGTH, ECC_LENGTH> rs{};

    Config config;

    friend class Window;
};

} // DS

#endif //DASHBOARD_H
