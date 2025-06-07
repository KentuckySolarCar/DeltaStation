#include <csignal>
#include <iostream>

#include "BufferParser.h"
#include "Dashboard.h"
#include "DebugReader.h"
#include "InputParameters.h"
#include "Reader.h"

// constexpr vs const: const is stored in the compiled binary, constexpr is optimized away by the compiler (and can also
// be used by templates.)

int main(const int argc, char *argv[]) {
    auto in = DS::InputParameters(argc, argv);
    // TODO: local on stack or global with singletons?
    // TODO: definitely incorporate buffer parser and reader into dashboard
    DS::BufferParser bp{};

    DS::Reader *r;

    if (in.debug_mode()) {
        r = new DS::DebugReader();
    } else {
        r = new DS::Reader(in.get_port(), in.get_baud());
    }

    DS::Dashboard db{};

    while (!db.should_close()) {
        if (!in.debug_mode() && !r->get_backend().isDeviceOpen()) {
            std::cerr << "Serialib Error: backend disconnected." << std::endl;
            exit(-1);
        }
        if (r->available()) {
            bp.put_byte(r->get_byte());
            db.byte_increment();
        }
        if (bp.ready()) {
            db.consume(bp.get_buffer());
        }

        db.update();
    }
}
