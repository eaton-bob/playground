#include <czmq.h>
#include <zyre.h>

#include <malamute.h>

void shout (zyre_t *node, const char *message) {
    assert (node);
    assert (message);
    zmsg_t *zyre_msg = zmsg_new ();
//    zmsg_addstr (zyre_msg, zyre_uuid (node));
//    zmsg_addstr (zyre_msg, "karol");
//    zmsg_addstr (zyre_msg, "MALAMUTE");
    zmsg_addstr (zyre_msg, message);
    zyre_shout (node, "MALAMUTE", &zyre_msg);
    zmsg_destroy (&zyre_msg);
}

int follower (zyre_t *node) {
    while (!zsys_interrupted) {
        //
        zclock_sleep (4000);
 
        zpoller_t *poller = zpoller_new (zyre_socket (node), NULL);
        zsock_t *which = (zsock_t *) zpoller_wait (poller, 1000);
        if (zpoller_expired (poller)) {
            zpoller_destroy (&poller);
            zsys_debug ("follower () We have no leader for too long");
            return 1;
        }
        else if (which == NULL) {
            zpoller_destroy (&poller);
            break;
        }
        else if (which == zyre_socket (node)) {            
            zmsg_t *zyre_msg = zyre_recv (node);
            char *command = zmsg_popstr (zyre_msg);
            if (streq (command, "SHOUT")) {
                zsys_debug ("There is someone still shouting... ok");
                continue;
            }
            free (command);
            zmsg_destroy (&zyre_msg);                        
        }       
    }
    return 0;
}

void leader (zyre_t *node, const char *endpoint, char *new_endpoint) {
    zactor_t *server = zactor_new (mlm_server, "Malamute");
    // zstr_sendx (server, "LOAD", config_file, NULL);
    zsock_send (server, "ss", "BIND", endpoint);

    while (!zsys_interrupted) {
        shout (node, endpoint);
        zsys_debug ("SHOUT");
        zclock_sleep (1000);
        // look for other shouts
        zpoller_t *poller = zpoller_new (zyre_socket (node), NULL);
        zsock_t *which = (zsock_t *) zpoller_wait (poller, 1000);
        if (zpoller_expired (poller)) {
            zsys_debug ("leader () zpoller_expored -> continue");
            continue;
        }
        else if (which == NULL) {
            zpoller_destroy (&poller);
            zsys_debug ("leader() which == NULL");
            break;
        }
        else if (which == zyre_socket (node)) {
            zsys_debug ("leader () which == zyre_socket");
            zmsg_t *zyre_msg = zyre_recv (node);
            zmsg_print (zyre_msg);
            char *command = zmsg_popstr (zyre_msg);
            if (!command) {
                zsys_error ("bad message");
                continue;
            }
            if (streq (command, "SHOUT")) {
                zsys_debug ("Yes, it's a shout.");
                char *uuid = zmsg_popstr (zyre_msg);
                char *name = zmsg_popstr (zyre_msg);
                char *channel = zmsg_popstr (zyre_msg);
                char *content = zmsg_popstr (zyre_msg);
                zsys_debug ("received SHOUT from (%s): %s", uuid, content);
                if (strcmp (zyre_uuid (node), uuid) < 0) {
                    zsys_debug ("leader () encountered higher uuid %s", uuid);
                    new_endpoint = strdup (content);
                    free (uuid); free (name); free (channel); free (content);
                    zmsg_destroy (&zyre_msg);                        
                    return;
                }
                else {
                    zsys_debug ("No, still leader.");

                }
            }
            free (command);
            zmsg_destroy (&zyre_msg);                        
        }
    }
    zactor_destroy (&server);
    return;
}



int main(int argc, char **argv) {
    if (argc < 2)
        return EXIT_FAILURE;

    // Create zyre node
    zyre_t *node = zyre_new ("karol");
    zyre_start (node);
    zyre_join (node, "MALAMUTE");
    zsys_debug ("karol's uuid: '%s'", zyre_uuid (node));

    int state = 0; // 0 - leader, shouts every second and listens for shouts, if uuid(shout received) > mine, switch states
                   // 1 - follower, check every 5 seconds, if no shout received, switch states

    while (!zsys_interrupted) {
        char *endpoint;
    
        if (state == 0) {
            leader (node, argv[1], endpoint);
            if (!endpoint)
                break;
            state = 1;
            continue;
        }
        else if (state == 1) {
            int rv = follower (node);
            if (rv) {
                state = 0;
                continue;
            }
            else
                break;
        }
    }

    //  Shutdown all services
    zyre_destroy (&node);
    return EXIT_SUCCESS;
}


