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

    void update_log() const;

    void update();

    void send_strategy(float target_soc, int target_unix_time, uint32_t uint32);

    void set_config(const std::string &path);

    void debug_print_packet_ids();

    IOSerial *serial{};

private:
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
    RS::ReedSolomon<MSG_LENGTH, ECC_LENGTH> foo{};

    Config config;

    friend class Window;
};

} // DS

#endif //DASHBOARD_H
