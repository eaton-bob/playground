#include <czmq.h>
#include <zyre.h>
#include <stdlib.h>

/* !\file   ups.c
   \details Generates UPS events once a sec, and publishes them to monitor;
            see rfc in top dir
 */

int main(int argc, char** argv) {
    if(argc != 2) {
        fprintf(stderr, "Usage: %s ups_name\n", argv[0]);
        exit(1);
    }

    char *addr = NULL;
    zyre_t *n = zyre_new(argv[1]);
    zyre_start(n);
    zyre_join(n, "BIOS");
    while(!zsys_interrupted && addr == NULL) {
        zyre_event_t *e = zyre_event_new(n);
        if(!e)
            break;
        if(zyre_event_headers(e) && zyre_event_header(e, "HAP_SERVER") != NULL) {
            addr = strdup(zyre_event_header(e, "HAP_SERVER"));
            printf("Address: %s\n", addr);
        }
        zyre_event_destroy(&e);
    }
    zyre_destroy(&n);

    if(addr == NULL)
        exit(1);

    zsock_t * sc = zsock_new(ZMQ_PUB);
    zsock_connect(sc, "%s", addr);
    bool state = random()%2;
    int timeout = 0;
    while(!zsys_interrupted) {
        if(timeout == 0) {
            state = !state;
            timeout = 5 + random()%20;
        }
        timeout--;
        if(state) {
            zstr_sendx(sc, argv[1], "ON", NULL);
            zsys_debug("UPS %s ON", argv[1]);
        } else {
            zstr_sendx(sc, argv[1], "OFF", NULL);
            zsys_debug("UPS %s OFF", argv[1]);
        }
        sleep(1);
    }
    zsock_destroy(&sc);
}
