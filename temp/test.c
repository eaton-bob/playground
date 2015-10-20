#include <czmq.h>
#include <malamute.h>

int main(int argc, char **argv) {
    //
    //
    const char *config_file = "malamute.cfg";
    if (argc >= 2)
        config_file = argv[1];

    //  Start Malamute server instance
    zactor_t *server = zactor_new (mlm_server, "Malamute");
    zstr_sendx (server, "LOAD", config_file, NULL);

    //  Accept and print any message back from server
    while (true) {
        char *message = zstr_recv (server);
        if (message) {
            puts (message);
            free (message);
        }
        else {
            puts ("interrupted");
            break;
        }
    }
    //  Shutdown all services
    zactor_destroy (&server);
    return EXIT_SUCCESS;
}


