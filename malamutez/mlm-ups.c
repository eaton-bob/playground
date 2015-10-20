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

static void
ups_actor (zsock_t *pipe, void *endpoint)
{
    //  Do some initialization
    mlm_client_t *client = mlm_client_new();
    mlm_client_connect(client, (char*)endpoint, 5000, "ups");
    mlm_client_set_producer(client, "metrics");
    zpoller_t *poller = zpoller_new(pipe, NULL);
    mlm_client_verbose = 1;
    zsock_signal (pipe, 0);

    bool terminated = false;
    while (!terminated) {
        zsock_t *which = zpoller_wait(poller, 1000);
        mlm_client_sendx(client, "battery.ups1", "100", NULL);
        if(which == NULL) {
            continue;
        }
        zmsg_t *msg = zmsg_recv (pipe);
        if (!msg)
            break;              //  Interrupted
        char *command = zmsg_popstr (msg);
        //  All actors must handle $TERM in this way
        if (streq (command, "$TERM"))
            terminated = true;
        else
        //  This is an example command for our test actor
        if (streq (command, "ECHO"))
            zmsg_send (&msg, pipe);
        else {
            puts ("E: invalid message to actor");
            assert (false);
        }
        free (command);
        zmsg_destroy (&msg);
    }
    mlm_client_destroy(&client);
}

static zactor_t *
s_ups(char *endpoint)
{
    //1. start malamute
    zactor_t *broker = zactor_new (ups_actor, endpoint);
    assert (broker);
    return broker;
}

int main() {

    if (!getenv("ENDPOINT")) {
        fprintf (stderr, "variable ENDPOINT must be declared\n");
        exit (EXIT_FAILURE);
    }

    char *endpoint = strdup(getenv("ENDPOINT"));
    zactor_t *ups = NULL;
    char *lastendpoint = strdup("nothing");

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
            endpoint = strdup (getenv("ENDPOINT"));
            zactor_destroy (&broker);
            broker = s_broker (endpoint);
        }

        if (!which && to_shout) {
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
                if (!r)
                    last_leader_shout = time(NULL);
                if (r >= 0)
                    goto event_destroy;

                zsys_debug ("UUID: %s, sender: %s, strcmp: %d",
                        UUID,
                        zyre_event_sender (event),
                        r
                        );

                to_shout = false;
                zstr_free (&UUID);
                UUID = strdup (zyre_event_sender (event));
                zstr_free (&endpoint);
                zmsg_t *msg = zyre_event_msg (event);
                endpoint = strdup (zmsg_popstr(msg));

                zactor_destroy (&broker);
                zclock_sleep(1000);

                break;
                }

        }

event_destroy:
        zyre_event_destroy (&event);
        if(strcmp(endpoint, lastendpoint) != 0) {
            zstr_free (&lastendpoint);
            lastendpoint = strdup(endpoint);
            zactor_destroy(&ups);
            ups = s_ups(endpoint);
        }
    }

    zpoller_destroy (&poller);
    zstr_free (&UUID);
    zstr_free (&endpoint);
    zyre_destroy (&node);
    zactor_destroy (&broker);
}
