#include <czmq.h>

int
main (int argc, char **argv)
{
    // Same result for the following strings:
    //      "+"
    //      "*something"
    //      "+something"
    //      som(e|)*thing
    zrex_t *regex = zrex_new ("som(e|)*thing");
    assert (regex);
    zsys_info ("regex_new () ok.");

    if (!zrex_valid (regex)) {
        zsys_error ("regex not valid: %s", zrex_strerror (regex));
        return EXIT_FAILURE;
    }
    zsys_info ("zrex_valid () returned true.");

    const char *text = "There is something in the attic.";

    int rv = zrex_matches (regex, text);
    zsys_info ("matches ? %s", rv ? "YES" : "NO");

    return EXIT_SUCCESS;
}
