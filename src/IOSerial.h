/* date = April 03, 2025 7:10 PM */


#ifndef READER_H
#define READER_H

#include <serialib.h>
#include <cstdint>
#include <iostream>

#include "common.h"

namespace DS {

class IOSerial {
public:
    IOSerial() = default;

    IOSerial(const char *port, int baud);

    virtual ~IOSerial();

    virtual int available() {
        return back.available();
    }

    virtual uint8_t get_byte() {
        char c;
        back.readChar(&c, reader_timeout);
        return c;
    }

    virtual void put_byte(const char c) {
        if (const int err = back.writeChar(c)) {
            // TODO: recover gracefully
            std::cerr << "Error while writing char to serial output: " << err << '\n';
            exit(-1);
        }
    }

    virtual void put(const std::string &s) {
        if (const int err = back.writeString(s.c_str())) {
            // TODO: recover gracefully
            std::cerr << "Error while writing string to serial output: " << err << '\n';
            exit(-1);
        }
    }

    virtual void put_bytes(const char *buf, const int len) {
        if (const int err = back.writeBytes(buf, len)) {
            // TODO: recover gracefully
            std::cerr << "Error while writing buffer to serial output: " << err << '\n';
            exit(-1);
        }
    }

    serialib &get_backend() { return back; }

private:
    serialib back;
    size_t reader_timeout = 3000;
};

} // DS

#endif //READER_H
