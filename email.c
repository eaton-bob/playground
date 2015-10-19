#include <czmq.h>
#include <assert.h>

int main() {
    zsock_t *client = zsock_new_sub("tcp://*:5561", "");
    assert (client);

    char *msg = zstr_recv(client);
    if (msg && streq(msg, "ALERT"))
        zsys_info("Got ALERT, sending an email");
    free(msg);

    zsock_destroy(&server);
}
