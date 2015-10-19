#include <czmq.h>
#include <assert.h>

/* !\file    email.c
   \details  Listens on :5561 to receive alerts from counterparts like monitor
             and forwards them to email; see rfc in top dir
*/

int main() {
    zsock_t *client = zsock_new_sub("tcp://*:5561", "");
    assert (client);

    char *msg = zstr_recv(client);
    if (msg && streq(msg, "ALERT"))
        zsys_info("Got ALERT, sending an email");
    free(msg);

    zsock_destroy(&client);
}
