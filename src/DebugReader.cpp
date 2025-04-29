/* date = April 19, 2025 10:42 AM */


#include "DebugReader.h"

#include <climits>
#include <ctime>

namespace DS {
    static float CURR_RAND_V = 0.0;
    static float CURR_RAND_C = 0.0;
    static int CURR_UNIX_TIME = 0;
    uint8_t DebugReader::get_byte() {
        if (position == 10) {
            CURR_RAND_V = static_cast<float>(rand()) / static_cast<float>(INT_MAX / 10);
        }
        if (position == 14) {
            CURR_RAND_C = static_cast<float>(rand()) / static_cast<float>(INT_MAX / 10);
        }
        if (position == 6) {
            CURR_UNIX_TIME = std::time(nullptr);
        }
        // note to future reader: JetBrains editors recommend making as many
        // variables const as possible. This is likely an influence from
        // languages like Rust, where variables are immutable by default. In
        // this context, there is little consequence for having it mutable vs.
        // immutable.
        char b = 0;
        if (position == 10 || position == 11 || position == 12 || position == 13) {
            b = reinterpret_cast<char *>(&CURR_RAND_V)[position - 10];
            position++;
        } else if (position == 14 || position == 15 || position == 16 || position == 17) {
            b = reinterpret_cast<char *>(&CURR_RAND_C)[position - 14];
            position++;
        } else if (position == 4) {
            b = message_type;
            message_type = message_type == 1 ? 2 : 1;
            position++;
        } else if (position == 6 || position == 7 || position == 8 || position == 9) {
            b = reinterpret_cast<char *>(&CURR_UNIX_TIME)[position - 6];
            position++;
        } else {
            b = SAMPLE_BUFFER[position++];
        }

        if (position >= sizeof(SAMPLE_BUFFER)) {
            position = 0;
        }
        return b;
    }
} // DS