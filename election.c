#include <malamute.h>
#include <zyre.h>

#define MY_ENDPOINT "tcp://192.168.199.115:4444"
#define TIMEOUT 5000

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
    bool isLeader = true;
    time_t leaderLastSeen = time(NULL);
    zpoller_t *poller = zpoller_new(zyre_socket (node), NULL);
    while (!zsys_interrupted)
    {
        zsock_t *which = zpoller_wait(poller, TIMEOUT);
        if (which == NULL)
        {
            // time is out, no information from the leader
            // we will end up here is:
            //    I am a leader
            //    no event at all had happend
            if ( !isLeader )
            {
                // become leader first
                zsys_info ("LEADER IS OUT");
                isLeader = true;
                free(leader_uuid);
                leader_uuid = strdup(zyre_uuid(node));
                free(leader_endpoint);
                leader_endpoint = strdup(MY_ENDPOINT);
                zsys_info("MY_MLM_CLIENT_STOP");
                zsys_info("MY_MLM_SERVER_START");
            }
            // and then shout
            zyre_shouts (node, CHANNEL, leader_endpoint);
        }
        zyre_event_t *event = zyre_event_new (node);
        const char *sender_uuid = zyre_event_sender (event);

        if (zyre_event_type (event) == ZYRE_EVENT_SHOUT)
        {
            // highest string becomes a leader            .
            zsys_info ("my_leader_uui %s", leader_uuid);
            zsys_info ("my_sender_uui %s", sender_uuid);
            int compare_result = strcmp (sender_uuid, leader_uuid);
            if (compare_result == 0)
            {
                // leader is still there
                leaderLastSeen = time(NULL);
            }
            else if (compare_result > 0 )
            {
                // We have a new leader
                zsys_info ("WINS %s", sender_uuid);
                leaderLastSeen = time(NULL);
                // get leaders endpoint
                zmsg_t *msg = zyre_event_msg(event);
                char *str = zmsg_popstr(msg);
                free(leader_endpoint);
                leader_endpoint = strdup(str);
                // get new one leaders uuid
                free (leader_uuid);
                char *leader_uuid = strdup(sender_uuid);
                // stop own malamute
                if (isLeader)
                    zsys_info("MY_MLM_SERVER_STOP");
                isLeader = false;
                // stop old client
                zsys_info("MY_MLM_CLIENT_STOP"); //mlm_client_destroy (&client);
                // create new client to malamute of the leader
                //client = mlm_client_new();
                //mlm_client_connect (client, leader_endpoint, 1000, "ACE");
                zsys_info("MY_MLM_CLIENT_START on %s", leader_endpoint);
            }
            else
            {
                if (isLeader)
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
        else
        {

            if ( time(NULL) - leaderLastSeen > 5 )
            {
                // no messages from leader for defined period of time
                if ( !isLeader )
                {
                    // become leader first
                    zsys_info ("LEADER IS OUT");
                    isLeader = true;
                    free(leader_uuid);
                    leader_uuid = strdup(zyre_uuid(node));
                    free(leader_endpoint);
                    leader_endpoint = strdup(MY_ENDPOINT);
                    zsys_info("MY_MLM_CLIENT_STOP");
                    zsys_info("MY_MLM_SERVER_START");
                }
                // and then shout
                zyre_shouts (node, CHANNEL, leader_endpoint);
                continue;
            }
            if (isLeader)
            {
                zyre_shouts (node, CHANNEL, leader_endpoint);
                zsys_info ("GOT NOT SHOUT:  I am still a leader -> shout");
            }
            else
            {
                // we still have time to wait for shout from leader
                zsys_info ("GOT NOT SHOUT  I am not a leader");
            }
        }
        zyre_event_destroy (&event);
    }

    zyre_stop (node);
    zyre_destroy (&node);
    zactor_destroy (&broker);
    return 0;
}
