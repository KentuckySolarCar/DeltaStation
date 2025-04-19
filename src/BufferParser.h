/* date = March 31, 2025 8:12 PM */


#ifndef BUFFERPARSER_H
#define BUFFERPARSER_H

#include "common.h"
#include "RS-FEC.h"

namespace DS {

class BufferParser {
public:
    enum BufferType {
        UndefinedMessage = 0,
        LeftMotorMessage = 1,
        RightMotorMessage = 2,
        GpsMessage = 3,
        MpptMessage = 4,
        BatteryMessage = 5,
        DriverInputMessage = 6,
        StatusMessage = 7,
        SensorMessage = 8,
    };

    struct Buffer {
        Buffer() = default;

        // The "explicit" keyword prevents C++ from allowing this construction:
        // char back[MSG_LENGTH] = {};
        // Buffer b = back;
        // Instead, one would explicitly call the constructor like this:
        // Buffer b = Buffer(back);
        // It is typically best practice to use "explicit" wherever possible.
        explicit Buffer(const uint8_t back[MSG_LENGTH]);

        BufferType type{UndefinedMessage};
        uint8_t data[MSG_LENGTH]{};
        uint8_t length{0};
        int timestamp{0};
    };

    BufferParser() = default;

    void put_byte(uint8_t c);

    Buffer &get_buffer() {
        buffer_ready = false;
        return packaged_buffer;
    }

    bool ready() const { return buffer_ready; }

private:
    // true if we're currently filling out the buffer. false otherwise
    bool reading = false;
    // current index into buffer data (used while actively reading new bytes)
    uint8_t buf_idx = 0;
    // storage for incoming data
    uint8_t buffer[BUFFER_LENGTH] = {};

    // this buffer is consumed by
    Buffer packaged_buffer{};
    bool buffer_ready = false;

    RS::ReedSolomon<MSG_LENGTH, ECC_LENGTH> rs{};
};

} // DS

#endif //BUFFERPARSER_H
