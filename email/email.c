#include <czmq.h>
#include <assert.h>

/* !\file    email.c
   \details  Listens on :5561 to receive alerts from counterparts like monitor
             and forwards them to email; see rfc in top dir
*/

int main(int argc, char** argv) {
    zsock_t *client = zsock_new_sub("tcp://*:5561", "");
    assert (client);

    char *msg, *ups, *state;
	int r = zstr_recvx(client, &ups, &msg, &state, NULL);
    if (msg && streq(msg, "ALERT"))
        zsys_info("Got ALERT for ups '%s', state '%s', sending an email", ups, state);
    free(msg);
    free(ups);
    free(state);

    zsock_destroy(&client);
}
