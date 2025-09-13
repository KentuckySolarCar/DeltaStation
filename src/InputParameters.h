/* date = March 31, 2025 8:08 PM */


#ifndef INPUTPARAMETERS_H
#define INPUTPARAMETERS_H
#include <string>

#include "common.h"

namespace DS {
    class InputParameters {
    public:
        InputParameters() = default;

        /**
         * Parses command-line arguments to set up the static environment of this
         * program. This program can exit early in this function if the "help"
         * argument is called or if undefined arguments are passed.
         */
        InputParameters(int argc, char *argv[]);

        /**
         * @return The filepath/name for the port associated with the running
         * instance of Delta station.
         */
        [[nodiscard]] const char *get_port() const { return port; }
        [[nodiscard]] int get_baud() const { return baud; }

        [[nodiscard]] bool debug_mode() const { return debug; }

        const std::string &get_config() { return config_path; }

    private:
        const char *port{};
        int baud = -1;
        bool debug = false;
        std::string config_path = "config.toml";

        /**
           * Prints a message on how to use this program. Only printed if the first
           * command-line argument is `help`.
           */
        static void usage() {
            printf("Usage: ds --port PORT --baud BAUD [options]\n");
            printf("\t--port PORT: Specify PORT from which serial connection is found.\n");
            printf("\t--baud BAUD: Specify BAUD rate for serial connection.\n");
            printf("\t--config FILE: Specify FILE for config.\n");
            printf("\t--debug: Enter debug mode. --port & --baud are unnecessary with this.\n");
        }

        static bool streq(const char *s0, const char *s1);
    };
} // DS

#endif //INPUTPARAMETERS_H
