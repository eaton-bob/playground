#include <malamute.h>

int main() {

    char *endpoint = "tcp://127.0.0.1:9999";

    zactor_t *broker = zactor_new (mlm_server, NULL);
    zsock_send (broker, "ss", "BIND", endpoint);
    zstr_send (broker, "VERBOSE");

    zclock_sleep (1000);
    zactor_destroy (&broker);

}
