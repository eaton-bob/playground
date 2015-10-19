#include <czmq.h>
#include <stdlib.h>

/* !\file   ups.c
   \details Generates UPS events once a sec, and publishes them to monitor;
            see rfc in top dir
 */

int main(int argc, char** argv) {
    if (argc < 1) {
        fprintf(stderr, "Required argument: host/ip of 'monitor':5560");
        return 1;
    }

    zsock_t * sc = zsock_new(ZMQ_PUB);
    zsock_connect(sc, "tcp://%s:5560", argv[1]);
    while(!zsys_interrupted) {
        if(random()%1) {
            zstr_send(sc, "ON");
        } else {
            zstr_send(sc, "OFF");
        }
        sleep(1);
    }
    zsock_destroy(&sc);
}
