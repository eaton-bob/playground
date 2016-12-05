#include <czmq.h>

int
main (int argc, char **argv)
{
    if (argc < 2) {
        zsys_warning ("Need argument");
        zsys_info ("usage: %s <regex>", argv [0]);
        return EXIT_FAILURE;
    }

    zrex_t *regex = zrex_new (argv [1]);
    if (!regex) {
        zsys_error ("zrex_new () failed.");
        return EXIT_FAILURE;
    }
    if (!zrex_valid (regex)) {
        zsys_error ("regex not valid: %s", zrex_strerror (regex));
        return EXIT_FAILURE;
    }
    
    zsys_info ("zrex valid");

    return EXIT_SUCCESS;

}
