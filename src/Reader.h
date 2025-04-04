/* date = April 03, 2025 7:10 PM */


#ifndef READER_H
#define READER_H
#include <serialib.h>

namespace DS {

class Reader {
public:
    Reader() = default;

    Reader(const char *port, int baud);

    ~Reader();

    int available() {
        return back.available();
    }

    char get_byte() {
        char c;
        back.readChar(&c, reader_timeout);
        return c;
    }

    serialib &get_backend() { return back; }

    // NOTE: this is for testing purposes only. do not use this function outside of debugging.
    static char _test_get_byte();
    static char _test_available() {
        return 1;
    }

private:
    serialib back;
    size_t reader_timeout = 3000;
};

} // DS

#endif //READER_H
