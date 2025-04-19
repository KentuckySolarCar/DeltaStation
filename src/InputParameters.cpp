/* date = March 31, 2025 8:08 PM */


#include "InputParameters.h"

#include <cstring>
#include <iostream>

namespace DS {
    InputParameters::InputParameters(int argc, char *argv[]) {
        // first argument is always the full path to the executable
        // we require four more arguments:
        // --port: where we're reading from
        // --baud: baud rate of serial connection
        // this takes the form of two arguments each.

        // read up on strncmp, and why it should be used compared to strcmp
        if (argc == 1 || streq(argv[1], "--help")) {
            usage();
            exit(0);
        }

        int curr_arg = 1;
        while (curr_arg < argc) {
            if (streq(argv[curr_arg], "--out")) {
                curr_arg++;

                if (curr_arg < argc) {
                    outfile = argv[curr_arg];
                    curr_arg++;
                } else {
                    printf("Input error: expected FILE\n");
                    usage();
                    exit(1);
                }
            } else if (streq(argv[curr_arg], "--port")) {
                curr_arg++;

                if (curr_arg < argc) {
                    port = argv[curr_arg];
                    curr_arg++;
                } else {
                    printf("Input error: expected PORT\n");
                    usage();
                    exit(1);
                }
            } else if (streq(argv[curr_arg], "--baud")) {
                curr_arg++;

                if (curr_arg < argc) {
                    baud = static_cast<int>(std::stol(argv[curr_arg]));
                    curr_arg++;
                } else {
                    printf("Input error: expected BAUD\n");
                    usage();
                    exit(1);
                }
            } else if (streq(argv[curr_arg], "--debug")) {
                curr_arg++;
                debug = true;
                printf("Using Debug Mode\n");
            } else {
                printf("Input error: unknown flag: %s\n", argv[curr_arg]);
                usage();
                exit(1);
            }
        }

        if (!debug && (!port || baud == -1)) {
            usage();
            exit(1);
        }

        printf("Current port: %s\n", port);
        printf("Current baud: %d\n", baud);
    }

    bool InputParameters::streq(const char *s0, const char *s1) {
        size_t l0 = strlen(s0);
        size_t l1 = strlen(s1);
        size_t min = l0 < l1 ? l0 : l1;

        return strncmp(s0, s1, min) == 0;
    }
} // DS