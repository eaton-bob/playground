#include <czmq.h>
#include <stdlib.h>

int main(int argc, char** argv) {
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
