#include <zyre.h>

/* !\file    email.c
   \details  Listens on :5561 to receive alerts from counterparts like monitor
             and forwards them to email; see rfc in top dir
*/

static char*
s_find_the_endpoint(void)
{
    zyre_t *node = zyre_new("email");
    assert (node);

    zyre_set_verbose (node);
    zyre_start (node);
    zyre_join (node, "BIOS");

    char *endpoint = NULL;

    while (!zsys_interrupted || !endpoint) {
        zyre_event_t *event = zyre_event_new (node);
        if (!event)
            break;

        if (zyre_event_headers (event)) {
            const char *ip_recv = zyre_event_header (event, "GAP_SERVER");
            if (ip_recv)
                endpoint = strdup (ip_recv);
        }
        zyre_event_print (event);
        zyre_event_destroy (&event);
    }
    zyre_stop (node);
    zyre_destroy (&node);
    return endpoint;
}


#define HB_WATERMARK 5

int main (int argc, char** argv) 
{
    zsys_info ("Getting the endpoint ...");
    char *endpoint = s_find_the_endpoint ();
    assert (endpoint);
    zsys_info ("Got %s ...", endpoint);

    zsock_t *client = zsock_new_sub (endpoint, "");
    assert (client);
    zstr_free (&endpoint);

    zpoller_t *pool = zpoller_new (client, NULL);

    int hb_missed = 0;
    while (!zsys_interrupted) {
        char *msg, *ups, *state;
        zsocket_t *which = zpoller_wait (pool, 1000);

        //  Did not get the heartbeet at least
        if (!which) {
            hb_missed += 1;
            if (hb_missed < HB_WATERMARK)
                continue;

            hb_missed = 0;
            zsys_info ("Connection lost, getting the endpoint ...");
            char *endpoint = s_find_the_endpoint();
            assert (endpoint);
            zsys_info("Got %s ...", endpoint);

            zsock_destroy(&client);

            client = zsock_new_sub (endpoint, "");
            assert (client);
            zstr_free (&endpoint);
        }
        int r = zstr_recvx (which, &ups, &msg, &state, NULL);
        if (msg) {
            if (streq (msg, "ALERT"))
                zsys_info ("Got ALERT for ups '%s', state '%s', sending an email",
                    ups, state);
            else if (streq(msg, "ART"))
                zsys_debug ("Got heartbeat message");
            else
                zsys_error ("UNEXPECTED: ups = %s, msg = %s, state = %s\n",
                    ups, msg, state);
        }
        zstr_free (&msg);
        zstr_free (&ups);
        zstr_free (&state);
    }
    zsock_destroy (&client);
    return 0;
}
