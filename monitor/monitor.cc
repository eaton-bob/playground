#include <czmq.h>
#include <string>
#include <map>
#include <utility>
#include <iostream>
#include <vector>

#include "monitor.h"

/* \file    monitor.cc
   \details listens on :5560 for events from counteragents like UPS and
            selectively pushes alerts to :5561; see rfc in top dir
 */

int main (int argc, char **argv) {

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

    std::cout << text << std::endl;     

    while (!zsys_interrupted) {
        zsock_t *which = (zsock_t *) zpoller_wait (poller, 1000);
        uint64_t timestamp = zclock_mono ();

        if (!which && zpoller_expired (poller)) {
            std::vector<std::string> to_delete;
            for (const auto& item : upses) {
                if (item.second.second + 1000 > timestamp) {
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
    }
    zsock_destroy (&sub);
    zsock_destroy (&pub);
    return 0;
}

