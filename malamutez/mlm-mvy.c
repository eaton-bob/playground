#include <malamute.h>
#include <zyre.h>
#include <stdlib.h>

static zactor_t *
s_broker(char *endpoint)
{
    //1. start malamute
    zactor_t *broker = zactor_new (mlm_server, NULL);
    assert (broker);
    zsys_info ("################################################################################");
    zsys_info ("malamute->bind(\"%s\")", endpoint);
    zsys_info ("################################################################################");
    zsock_send (broker, "ss", "BIND", endpoint);
    zstr_send (broker, "VERBOSE");
    return broker;
}

int main() {

    if (!getenv("ENDPOINT")) {
        fprintf (stderr, "variable ENDPOINT must be declared\n");
        exit (EXIT_FAILURE);
    }

    char *endpoint = strdup(getenv("ENDPOINT"));

    //1. start malamute
    zactor_t *broker = s_broker(endpoint);

    //2. shout about it through zyre
    zyre_t *node = zyre_new (getenv("USER"));
    assert (node);
    char * UUID = strdup (zyre_uuid (node));
    zsys_info ("UUID: %s", UUID);
    bool to_shout = true;

    //zyre_set_verbose (node);
    zyre_start (node);
    zyre_join (node, "MALAMUTE");

    zpoller_t *poller = zpoller_new(zyre_socket (node), NULL);

    // 3. get the comments
    while (!zsys_interrupted) {

        zsock_t *which = zpoller_wait(poller, 1000);
        if (!which && to_shout) {
            zyre_shouts (node, "MALAMUTE", "%s", endpoint);
            continue;
        }

        zyre_event_t *event = zyre_event_new (node);
        if (!event)
            continue;

        switch (zyre_event_type (event)) {
            case ZYRE_EVENT_SHOUT:
                if (strcmp (UUID, zyre_event_sender (event)) >= 0)
                    goto event_destroy;

                zsys_debug ("UUID: %s, sender: %s, strcmp: %d",
                        UUID,
                        zyre_event_sender (event),
                        strcmp (UUID, zyre_event_sender (event))
                        );

                to_shout = false;
                zstr_free (&UUID);
                UUID = strdup (zyre_event_sender (event));
                zstr_free (&endpoint);
                zmsg_t *msg = zyre_event_msg (event);
                endpoint = strdup (zmsg_popstr(msg));

                zactor_destroy (&broker);
                broker = s_broker (endpoint);
                zclock_sleep(1000);

                break;

            case ZYRE_EVENT_EXIT:
                if (streq (UUID, zyre_event_sender (event)))
                {
                    to_shout = true;
                    zstr_free (&UUID);
                    UUID = strdup (zyre_uuid (node));
                    zstr_free (&endpoint);
                    endpoint = strdup ("tcp://192.168.1.105:9999");
                }
                break;

        }

event_destroy:
        zyre_event_destroy (&event);
    }

    zpoller_destroy (&poller);
    zstr_free (&UUID);
    zstr_free (&endpoint);
    zyre_destroy (&node);
    zactor_destroy (&broker);
}
