#include "BufferParser.h"
#include "Dashboard.h"
#include "InputParameters.h"
#include "Reader.h"

// constexpr vs const: const is stored in the compiled binary, constexpr is optimized away by the compiler (and can also
// be used by templates.)

int main(int argc, char *argv[]) {
    DS::InputParameters in{};
    DS::BufferParser bp{};
    DS::Dashboard db{};

    in = DS::InputParameters(argc, argv);

    DS::Reader r{in.get_port(), in.get_baud()};

    while (true) {
        if (!r.get_backend().isDeviceOpen()) {
            fprintf(stderr, "Error: backend disconnected.");
            return -1;
        }
        if (r.available()) {
            bp.put_byte(r.get_byte());
        }
        if (bp.ready()) {
            db.consume(bp.get_buffer());
            db.print();
        }
    }

    return 0;
}
