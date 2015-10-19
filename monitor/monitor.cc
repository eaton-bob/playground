#include <czmq.h>

int main (int argc, char **argv) {
    if (argc != 3) {
        zsys_error ("alers <SUB socket> <PUB socket>");
        return EXIT_FAILURE;
    }

    zsock_t *sub = zsock_new_sub (argv[1], "");
    assert (sub);

    zsock_t *pub = zsock_new_rep (argv[2]);
    assert (pub); 

    //  Set-up poller (oroginally we had rep.. too lazy to remove)
    zpoller_t *poller = zpoller_new (sub, NULL);
    assert (poller);

    int state = 0; // 0 - off, 1 - on

    while (!zsys_interrupted) {
        zsock_t *which = (zsock_t *) zpoller_wait (poller, -1);
        if (!which) {
            zsys_error ("The zpoller_wait () call was interrupted (SIGINT), or the ZMQ context was destroyed");
            break;
        }

        if (which == sub) {
            char *str = zstr_recv (which);
            zsys_debug ("RECV (%s): %s", argv[1], str);
            if (streq (str, "ON") && state == 0) {
                state = 1;
                zstr_sendx (pub, "ALERT", "ON", NULL);
                zsys_debug ("PUBLISH");
            }
            else if (streq (str, "OFF") && state == 1) {
                state = 0;
                zstr_sendx (pub, "ALERT", "OFF", NULL);
                zsys_debug ("PUBLISH");
            }
            zstr_free (&str);
        }
    }
    zsock_destroy (&sub);
    zsock_destroy (&pub);
    return 0;
}

