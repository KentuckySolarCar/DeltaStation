/* date = April 03, 2025 7:10 PM */


#include "Reader.h"

#include <cstdint>

#include "common.h"

namespace DS {
    Reader::Reader(const char *port, int baud) {
        const char err = back.openDevice(port, baud);

        if (err != 1) {
            printf("Serial error: couldn't open serial connection to port %s. Error code: %d\n", port, err);
            exit(2);
        }

        printf("Successful serial connection to port: %s\n", port);
    }

    Reader::~Reader() {
        back.closeDevice();
    }

    namespace {
        uint8_t _sample_buffer[MSG_LENGTH] {
            'U', 'K', 'S', 'C', 1, LEFT_MOTOR_MESSAGE, 0, //TODO
        };
        uint8_t _curr_pos = 0;
    }

    char Reader::_test_get_byte() {
        char ret = static_cast<char>(_sample_buffer[_curr_pos++]);
        if (_curr_pos >= MSG_LENGTH) {
            _curr_pos = 0;
        }
        return ret;
    }
} // DS