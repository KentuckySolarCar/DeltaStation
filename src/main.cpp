#include <csignal>

#include "BufferParser.h"
#include "Dashboard.h"
#include "DebugReader.h"
#include "InputParameters.h"
#include "Reader.h"

// constexpr vs const: const is stored in the compiled binary, constexpr is optimized away by the compiler (and can also
// be used by templates.)

int main(const int argc, char *argv[]) {
    auto in = DS::InputParameters(argc, argv);
    DS::BufferParser bp{};
    DS::Dashboard db{};

    DS::Reader *r;

    if (in.debug_mode()) {
        r = new DS::DebugReader();
    } else {
        r = new DS::Reader(in.get_port(), in.get_baud());
    }

    while (true) {
        if (!in.debug_mode() && !r->get_backend().isDeviceOpen()) {
            fprintf(stderr, "Error: backend disconnected.");
            exit(-1);
        }
        if (r->available()) {
            bp.put_byte(r->get_byte());
        }
        if (bp.ready()) {
            db.consume(bp.get_buffer());
            db.print();
        }

        db.display();
    }
}
