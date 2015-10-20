#include <malamute.h>
#include <zyre.h>

#define MY_ENDPOINT "tcp://192.168.199.115:4444"

int main() {

    const char *CHANNEL = "MALAMUTE";

    char *leader_endpoint = strdup(MY_ENDPOINT);

    zactor_t *broker = zactor_new (mlm_server, NULL);
    zsock_send (broker, "ss", "BIND", leader_endpoint);
    zstr_send (broker, "VERBOSE");

    zyre_t *node = zyre_new("ace-zyre");
    assert(node);
    zyre_set_verbose (node);
    zyre_start (node);
    zyre_join (node, CHANNEL);

    char *leader_uuid = strdup(zyre_uuid(node));
    zsys_info("my uuid is %s", leader_uuid);

//    mlm_client_t *client = NULL;
    while (!zsys_interrupted)
    {
        zyre_event_t *event = zyre_event_new (node);
        // Returns event type, which is a zyre_event_type_t
        const char *sender_uuid = zyre_event_sender (event);
        if (zyre_event_type (event) == ZYRE_EVENT_SHOUT)
        {
            zsys_info ("GOT SHOUT");
            // Return the sending peer's id as a string
            // highest string becomes a leader            .
            zsys_info ("my_leader_uui %s", leader_uuid);
            zsys_info ("my_sender_uui %s", sender_uuid);
            if (strcmp (leader_uuid, sender_uuid) < 0 )
            {
                // We have a new leader
                zsys_info ("WINS %s", sender_uuid);
                // get leaders endpoint
                zmsg_t *msg = zyre_event_msg(event);
                char *str = zmsg_popstr(msg);
                free(leader_endpoint);
                leader_endpoint = strdup(str);
                // get new one leaders uuid
                free (leader_uuid);
                char *leader_uuid = strdup(sender_uuid);
                // stop own malamute
                zsys_info("MY_MLM_SERVER_STOP");
                // stop old client
                zsys_info("MY_MLM_CLIENT_STOP"); //mlm_client_destroy (&client);
                // create new client to malamute of the leader
                //client = mlm_client_new();
                //mlm_client_connect (client, leader_endpoint, 1000, "ACE");
                zsys_info("MY_MLM_CLIENT_START on %s", leader_endpoint);
            }
            else
            {
                if ( strcmp (leader_uuid, zyre_uuid(node))==0)
                {
                    // I am still the leader
                    zyre_shouts (node, CHANNEL, leader_endpoint);
                }
                else
                {
                    // leader didn't change nothing to do
                }

            }
        }
        else if (zyre_event_type (event) == ZYRE_EVENT_EXIT)
        {
            zsys_info ("GOT EXIT");
            // DO EXIT
            if ( strcmp (leader_uuid, sender_uuid)==0)
            {
                zsys_info ("LEADER IS OUT");
                free(leader_uuid);
                leader_uuid = strdup(zyre_uuid(node));
                free(leader_endpoint);
                leader_endpoint = strdup(MY_ENDPOINT);
                zyre_shouts (node, CHANNEL, leader_endpoint);
                zsys_info("MY_MLM_CLIENT_STOP");
                zsys_info("MY_MLM_SERVER_START");
            }
        }
        else
        {
            zsys_info ("GOT NOT SHOUT");
            if ( strcmp (leader_uuid, zyre_uuid(node))==0)
            {
                zyre_shouts (node, CHANNEL, leader_endpoint);
                zsys_info ("GOT NOT SHOUT:  I am still a leader -> shout");
            }
            else
            {
                zsys_info ("GOT NOT SHOUT  I am not a leader");
            }
        }
        zyre_event_destroy (&event);
    }

 //   mlm_client_destroy (&client);
    zyre_stop (node);
    zyre_destroy (&node);
    zactor_destroy (&broker);
    return 0;
}
