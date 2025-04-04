/* date = March 31, 2025 8:08 PM */


#ifndef INPUTPARAMETERS_H
#define INPUTPARAMETERS_H
#include <string>

namespace DS {

class InputParameters {
public:
    InputParameters() = default;
    /**
     * Parses command-line arguments to set up the static environment of this program. This program can exit early in this
     * function if the "help" argument is called or if undefined arguments are passed.
     */
    InputParameters(int argc, char *argv[]);

    const char *get_outfile() const { return outfile; }
    const char *get_port() const { return port; }
    int get_baud() const { return baud; }

private:
    const char *outfile{};
    const char *port{};
    int baud = -1;

    /**
     * Prints a message on how to use this program. Only printed if the first command-line argument is `help`.
     */
    static void usage() {
        printf("Usage: ds --port PORT --baud BAUD [--out FILE]\n");
        printf("\t--port: Specify PORT from which serial connection is found.\n");
        printf("\t--baud: Specify BAUD rate for serial connection.\n");
        printf("\t--out: Specify FILE to output.\n");
    }

    static bool streq(const char *s0, const char *s1);
};

} // DS

#endif //INPUTPARAMETERS_H
