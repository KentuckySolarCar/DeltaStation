/* date = March 31, 2025 8:12 PM */


#ifndef BUFFERPARSER_H
#define BUFFERPARSER_H

#include <optional>
#include <string>

#include "common.h"
#include "RS-FEC.h"

namespace DS {
    class BufferParser {
    public:
        /**
         * This enumeration differentiates between packets received from Main Board. The types correspond directly with an
         * enumeration of a similar nature in the bottom shell repository, and ought to be maintained as such.
         */
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
            NumTypes,
        };

        static std::optional<BufferType> from_id(const int id) {
            if (id == UndefinedMessage || id >= NumTypes) {
                return std::nullopt;
            }
            return static_cast<BufferType>(id);
        }

        /**
         * This struct is used as a temporary wrapper over a buffer. It is created directly from the BufferParser and
         * shipped straight to Dashboard.
         */
        struct Buffer {
            Buffer() = default;

            // The "explicit" keyword prevents C++ from allowing this construction:
            // char back[MSG_LENGTH] = {};
            // Buffer b = back;
            // Instead, one would explicitly call the constructor like this:
            // Buffer b = Buffer(back);
            // It is typically best practice to use "explicit" wherever possible.
            explicit Buffer(const uint8_t back[MSG_LENGTH]);

            // See above enumeration
            BufferType type{UndefinedMessage};
            // Data parsed from buffer (after RS-FEC decoding). Namely, bytes 10-40 of the incoming buffer.
            uint8_t data[MSG_LENGTH]{};
            // Message length
            uint8_t length{0};
            // Timestamp of received message
            int timestamp{0};
        };

        BufferParser() = default;

        /**
         * Put a byte into a temporary buffer. This function maintains two states:
         * - In validate mode, it checks if the incoming byte (and subsequent calls) would begin a new valid packet.
         * - In fill mode, it fills the buffer with data coming from serial until the buffer is full.
         * TODO: additional validation can be done on the data with the `GDS` ending to a packet.
         * @param c Character to put into the buffer.
         */
        void put_byte(uint8_t c);

        /**
         * This function gives a reference to the last valid buffer the BufferParser has created. Upon doing so, the parser
         * considers itself "empty", and ready to begin parsing a new buffer.
         * @return Parsed buffer.
         */
        Buffer &get_buffer() {
            buffer_ready = false;
            return packaged_buffer;
        }

        /**
         * Read return.
         * @return If a buffer has been parsed since the last one received.
         */
        bool ready() const { return buffer_ready; }

        static BufferType from_str(const std::string &str) {
            if (str == "mta")
                return LeftMotorMessage;
            if (str == "mtb")
                return RightMotorMessage;
            if (str == "gps")
                return GpsMessage;
            if (str == "arr")
                return MpptMessage;
            if (str == "bat")
                return BatteryMessage;
            if (str == "drv")
                return DriverInputMessage;

            return UndefinedMessage;
        }

    private:
        // true if we're currently filling out the buffer. false otherwise
        bool reading = false;
        // current index into buffer data (used while actively reading new bytes)
        uint8_t buf_idx = 0;
        // storage for incoming data
        uint8_t buffer[BUFFER_LENGTH] = {};

        // this buffer is consumed by Dashboard; read forward-facing API functions.
        Buffer packaged_buffer{};
        bool buffer_ready = false;

        // TODO: we have a duplicate RS encoder in Dashboard.h
        RS::ReedSolomon<MSG_LENGTH, ECC_LENGTH> rs{};
    };
} // DS

#endif //BUFFERPARSER_H
