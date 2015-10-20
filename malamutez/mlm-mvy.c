#include <malamute.h>
#include <zyre.h>
#include <stdlib.h>

static zactor_t *
s_broker(char *endpoint)
{
    //1. start malamute
    zactor_t *broker = zactor_new (mlm_server, NULL);
    assert (broker);
    zsys_info ("malamute->bind(\"%s\")", endpoint);
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

    time_t last_leader_shout;

    // 3. get the comments
    while (!zsys_interrupted) {

        zsock_t *which = zpoller_wait(poller, 1000);

        if (time(NULL) - last_leader_shout > 5) {
            to_shout = true;
            zstr_free (&UUID);
            UUID = strdup (zyre_uuid (node));
            zstr_free (&endpoint);
            endpoint = strdup ("tcp://192.168.1.105:9999");
            zactor_destroy (&broker);
            broker = s_broker (endpoint);
        }

        if (!which) {
            if (to_shout)
                zyre_shouts (node, "MALAMUTE", "%s", endpoint);
            continue;
        }

        zyre_event_t *event = zyre_event_new (node);
        if (!event)
            continue;

        switch (zyre_event_type (event)) {
            case ZYRE_EVENT_SHOUT:
                {
                int r = strcmp (UUID, zyre_event_sender (event));
                if (!r) {
                    last_leader_shout = time(NULL);
                    zsys_debug ("Leader (%s)SHOUTS", zyre_event_sender (event));
                }
                if (r >= 0)
                    break;

                zsys_debug ("UUID: %s, sender: %s, strcmp: %d",
                        UUID,
                        zyre_event_sender (event),
                        r
                        );

                to_shout = false;
                zstr_free (&UUID);
                UUID = strdup (zyre_event_sender (event));
                zsys_debug ("new UUID = %s", UUID);
                zstr_free (&endpoint);
                zmsg_t *msg = zyre_event_msg (event);
                endpoint = strdup (zmsg_popstr(msg));

                zactor_destroy (&broker);
                broker = s_broker (endpoint);
                zclock_sleep(1000);

                break;
                }

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
