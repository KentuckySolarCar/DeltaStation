/* date = March 31, 2025 8:12 PM */


#include "BufferParser.h"

#include <cstring>

namespace DS {
    BufferParser::Buffer::Buffer(const uint8_t back[MSG_LENGTH]) {
        // note to self: format of buffer is:
        // b[0-3]: header 'UKSC'
        // b[4]: message type
        // b[5]: message length
        // b[6-9]: unix timestamp

        this->type = back[MESSAGE_TYPE_BYTE];
        this->length = back[ACTUAL_MESSAGE_LENGTH_OFFSET];
        this->timestamp = *reinterpret_cast<const int *>(&back[0] + TIME_OFFSET);
        memcpy(this->data, &back[DATA_OFFSET], BUFFER_LENGTH - DATA_OFFSET);
    }

    void BufferParser::put_byte(uint8_t c) {
        // check if we should start reading
        if (!reading) {
            if (c == 'U') { // begin reading
                reading = true;
            } else {
                return;
            }
        }

        buffer[buf_idx] = c;

        // check if we got the right header
        if (buf_idx == 4 && strncmp(reinterpret_cast<const char *>(buffer), "UKSC", 4) != 0) {
            // if header is wrong, reset buffer reading
            reading = false;
            buf_idx = 0;
            return;
        }

        buf_idx++;
        if (buf_idx == BUFFER_LENGTH) {
            // validate buffer

            // re-encode ReedSolomon
            auto decoded = new uint8_t[MSG_LENGTH];
            rs.Decode(buffer, decoded);
            // create and digest buffer
            // emplace_back takes the parameters for the constructor of a class and places uses them to "emplace" a new
            // object in the vector using a constructor assumed by the compiler.

            delete decoded;
        }
    }
} // DS