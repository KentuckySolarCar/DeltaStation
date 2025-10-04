#include <iostream>

#include <toml++/toml.hpp>

#include "BufferParser.h"
#include "Dashboard.h"
#include "DebugReader.h"
#include "InputParameters.h"
#include "IOSerial.h"
#include "expr/Lexer.h"

// constexpr vs const: const is stored in the compiled binary, constexpr is optimized away by the compiler (and can also
// be used by templates.)

int main(const int argc, char *argv[]) {
    auto in = DS::InputParameters(argc, argv);
    // TODO: local on stack or global with singletons?
    DS::BufferParser bp{};
    DS::Dashboard db{};

    if (in.debug_mode()) {
        db.serial = new DS::DebugReader();
        std::cout << "Serial output connected to standard output.\n";
        DS::Expr::test_lexer();
    } else {
        db.serial = new DS::IOSerial(in.get_port(), in.get_baud());
    }

    DS::IOSerial *s = db.serial;

    db.set_config(in.get_config());

    if (in.debug_mode())
        db.debug_print_packet_ids();

    while (!db.should_close()) {
        if (!in.debug_mode() && !s->get_backend().isDeviceOpen()) {
            std::cerr << "Serialib Error: backend disconnected." << std::endl;
            exit(-1);
        }
        if (s->available()) {
            bp.put_byte(s->get_byte());
            db.byte_increment();
        }
        if (bp.ready()) {
            db.consume(bp.get_buffer());
        }

        db.update();
    }
}
