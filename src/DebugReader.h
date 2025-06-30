/* date = April 19, 2025 10:42 AM */


#ifndef DEBUGREADER_H
#define DEBUGREADER_H
#include <cstdint>

#include "IOSerial.h"
#include "common.h"

namespace DS {

class DebugReader : public IOSerial {
public:
    DebugReader() = default;
    ~DebugReader() override = default;

    int available() override {
        return 1;
    }

    uint8_t get_byte() override;

    void put(const std::string &s) override;

    void put_byte(char c) override;

    void put_bytes(const char *buf, int len) override;

private:
    int position = 0;
    char message_type = 1;
    static constexpr char SAMPLE_BUFFER[52] = {
        'U', 'K', 'S', 'C', // header
        0, // message type (overridden)
        0, // message length (to be used later in advanced error checking)
        0, 0, 0, 0, // timestamp (manually updated)
        0, 0, 0, 0, // voltage placeholder (randomized)
        0, 0, 0, 0, // current placeholder (randomized)
        'G', 'D', 'S'
    };
};

} // DS

#endif //DEBUGREADER_H
