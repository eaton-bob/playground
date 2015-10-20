#include <czmq.h>
#include <zyre.h>
#include <string>
#include <map>
#include <utility>
#include <iostream>
#include <string>
#include <vector>
#include <sys/syscall.h>

#include "monitor.h"

/* \file    monitor.cc
   \details listens on :5560 for events from counteragents like UPS and
            selectively pushes alerts to :5561; see rfc in top dir
*/

void shout (zyre_t *node, const char *ip_addr) {
    assert (node);
    assert (ip_addr);
    
    zmsg_t *zyre_msg = zmsg_new ();
    zmsg_addstr (zyre_msg, std::string("tcp://").append (ip_addr).append(":5560").c_str ());
    zmsg_addstr (zyre_msg, std::string("tcp://").append (ip_addr).append(":5561").c_str ());
    zyre_shout (node, "MALAMUTE", &zyre_msg);
    zmsg_destroy (&zyre_msg);
}

int follower (zyre_t *node) {
    zsys_info ("Entering 'follower' state.");
    while (!zsys_interrupted) {
        zclock_sleep (4000);

        zpoller_t *poller = zpoller_new (zyre_socket (node), NULL);
        zsock_t *which = (zsock_t *) zpoller_wait (poller, 1000);
        if (zpoller_expired (poller)) {
            zpoller_destroy (&poller);
            zsys_info ("We have had no leader for too long!");
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
                zsys_info ("There is someone still shouting... ok");
                continue;
            }
            free (command);
            zmsg_destroy (&zyre_msg);
        }
    }
    return 0;
}

int leader (zyre_t *node, const char *ip_addr) {
    zsys_info ("Entering 'leader' state.");

    zsock_t *sub = zsock_new (ZMQ_SUB);
    int rv = zsock_bind  (sub, "tcp://*:5560");
    assert (rv != -1);
    zsock_set_subscribe (sub, "");

    zsock_t *pub = zsock_new (ZMQ_PUB);
    rv = zsock_bind (pub, "tcp://*:5561");
    assert (rv != -1);

    zpoller_t *poller = zpoller_new (sub, zyre_socket (node), NULL);
    assert (poller);

    // key: ups name value: state, timestamp
    std::map<std::string, std::pair<int, uint64_t>> upses;

    while (!zsys_interrupted) {
        shout (node, ip_addr);
        zsys_debug ("SHOUT");
        zclock_sleep (400);
        // look for other shouts

        zsock_t *which = (zsock_t *) zpoller_wait (poller, 400);
        zstr_sendx (pub, "HE", "ART", "BEET", NULL);
        uint64_t timestamp = zclock_mono ();

        if (zpoller_expired (poller)) {
            std::vector<std::string> to_delete;
            for (const auto& item : upses) {
                if (item.second.second + 1000 < timestamp) {
                    zstr_sendx (pub, item.first.c_str (), "ALERT", "GONE", NULL);
                    zsys_debug ("PUBLISH: ups_name: '%s'\tstate: '%s'", item.first.c_str (), "GONE");
                    to_delete.push_back (item.first);
                }
            }
            for (const auto& item : to_delete) {
                upses.erase (item);
            }
            continue;

        }
        else if (which == NULL) {
            
            zsys_debug ("leader() which == NULL");
            break;
        }
        else if (which == sub) {
            char *ups_name = NULL, *state = NULL;
            rv = zstr_recvx (which, &ups_name, &state, NULL);
            assert (rv != -1);

            auto needle = upses.find (ups_name);
            if (needle == upses.end ()) {
                // insert
                zsys_debug ("adding ups_name '%s'", ups_name);
                upses.emplace (std::make_pair (ups_name, std::make_pair (streq (state, "ON") ? 1 : 0, timestamp)));
                zstr_sendx (pub, ups_name, "ALERT", state, NULL);
                zsys_debug ("PUBLISH: ups_name: '%s'\tstate: '%s'", ups_name, state);
            }
            else if (!streq (state, needle->second.first == 1 ? "ON" : "OFF")) {
                needle->second = std::make_pair(streq (state, "ON") ? 1 : 0, timestamp);
                zstr_sendx (pub, ups_name, "ALERT", state, NULL);
                zsys_debug ("PUBLISH: ups_name: '%s'\tstate: '%s'", ups_name, state);
            }

            zstr_free (&ups_name);
            zstr_free (&state);
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
                zsys_debug ("received SHOUT from '%s'", uuid);
                if (strcmp (zyre_uuid (node), uuid) < 0) {
                    zsys_debug ("leader () encountered higher uuid %s", uuid);
                    free (uuid); free (name); free (channel);
                    zmsg_destroy (&zyre_msg);
                    zsock_destroy (&sub);
                    zsock_destroy (&pub);
                    zpoller_destroy (&poller);
                    return 1;
                }
                else {
                    zsys_debug ("No, still leader.");
                }
            }
            free (command);
            zmsg_destroy (&zyre_msg);
        }
    }
    zsock_destroy (&sub);
    zsock_destroy (&pub);
    zpoller_destroy (&poller);
    return 0;
}


int main (int argc, char **argv) {
    if (argc < 2)
        return EXIT_FAILURE;
    std::cout << text << std::endl;     

    zyre_t *node = zyre_new (std::string ("monitor.").append(std::to_string (getpid ())).append(std::to_string (syscall(SYS_gettid))).c_str());
//    zyre_set_header (node, "HAP_SERVER", "%s", std::string("tcp://").append (argv[1]).append(":5560").c_str ());
//    zyre_set_header (node, "GAP_SERVER", "%s", std::string("tcp://").append (argv[1]).append(":5561").c_str ());
    zyre_start (node);
    zyre_join (node, "MONITORS");
    zsys_info ("Zyre node created - uuid: '%s'", zyre_uuid (node));

/*
    zsock_t *sub = zsock_new (ZMQ_SUB);
    assert (sub);

    int rv = zsock_bind  (sub, "tcp://*:5560");
    assert (rv != -1);
    zsock_set_subscribe (sub, "");

    zsock_t *pub = zsock_new (ZMQ_PUB);
    assert (pub);
    rv = zsock_bind (pub, "tcp://*:5561");
    assert (rv != -1);

    // Set-up poller (originally we had rep... too lazy to remove)
    zpoller_t *poller = zpoller_new (sub, NULL);
    assert (poller);

    // key: ups name value: state, timestamp
    std::map<std::string, std::pair<int, uint64_t>> upses;
*/

    int state = 0; // 0 - leader, shouts every second and listens for shouts, if uuid(shout received) > mine, switch states
                   // 1 - follower, check every 5 seconds, if no shout received, switch states


    while (!zsys_interrupted) {

        if (state == 0) {
            int rv = leader (node, argv[1]);
            if (rv == 1)
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
    zyre_destroy (&node);
    return 0;
}

