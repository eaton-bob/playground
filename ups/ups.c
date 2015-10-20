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
    zyre_join(n, "MONITORS");

    char *hap_server = NULL;
    while (!zsys_interrupted) { 
        zmsg_t *zyre_msg = zyre_recv (n);
        zmsg_print (zyre_msg);
        char *command = zmsg_popstr (zyre_msg);
        if (!streq (command, "SHOUT"))
            continue;
        char *uuid = zmsg_popstr (zyre_msg);
        char *name = zmsg_popstr (zyre_msg);
        char *channel = zmsg_popstr (zyre_msg);
        hap_server = zmsg_popstr (zyre_msg);
        free (uuid); free (name); free (channel); free (command);
        break;
    }
    zsys_debug ("initial HAP server: %s", hap_server);
    
    addr = hap_server;
    if(addr == NULL)
        exit(1);

    zsock_t * sc = zsock_new(ZMQ_PUB);
    zsock_connect(sc, "%s", addr);
    zsys_debug ("socket created, connected.");
    bool state = random()%2;
    int timeout = 0;
    int cummulative = 0;
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
        cummulative++;
        if (cummulative == 5) {
            char *new_hap;
            while (!zsys_interrupted) { 
                zmsg_t *zyre_msg = zyre_recv (n);
                zmsg_print (zyre_msg);
                char *command = zmsg_popstr (zyre_msg);
                if (!streq (command, "SHOUT"))
                    continue;
                char *uuid = zmsg_popstr (zyre_msg);
                char *name = zmsg_popstr (zyre_msg);
                char *channel = zmsg_popstr (zyre_msg);
                new_hap = zmsg_popstr (zyre_msg);
                free (uuid); free (name); free (channel); free (command);
                break;
            }
            if (!streq (new_hap, hap_server)) {
                free (hap_server); hap_server = new_hap;
                zsock_connect(sc, "%s", addr);
            }
            cummulative = 0;
        }
    }
    if (hap_server)
        free (hap_server);
    zsock_destroy(&sc);
}
