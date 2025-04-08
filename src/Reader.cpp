/* date = April 03, 2025 7:10 PM */


#include "Reader.h"

#include <cstdint>
#include <cstdio>

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
} // DS