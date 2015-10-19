#include <czmq.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char** argv) {

    char *endpoint = "tcp://*:5561";
    if (argc <= 1)
        zsys_info("You can use email-cli tcp://ip-address:5561\n");
    else
        endpoint = argv[1];

    zsock_t *client = zsock_new_sub(endpoint, "");
    assert (client);

    while (!zsys_interrupted) {
        char *msg, *ups, *state;
        int r = zstr_recvx(client, &ups, &msg, &state, NULL);
        if (msg && streq(msg, "ALERT"))
            zsys_info("Got ALERT for ups '%s', state '%s', sending an email", ups, state);
        else
            zsys_error("UNEXPECTED: ups = %s, msg = %s, state = %s\n", ups, msg, state);
        zstr_free(&msg);
        zstr_free(&ups);
        zstr_free(&state);
    }

    zsock_destroy(&client);
}
