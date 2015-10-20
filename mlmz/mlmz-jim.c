/* !\file   mlmz.c
   \details Malamute launcher that finds others like it on a LAN and they
            elect a leader that would be the broker
 */

// TODO: Break the monolith into functions

#include <zyre.h>
#include <malamute.h>

// Sample code from http://man7.org/linux/man-pages/man3/getifaddrs.3.html
#define _GNU_SOURCE     /* To get defns of NI_MAXSERV and NI_MAXHOST */
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/if_link.h>

int main(int argc, char**argv) {
	zsys_info("NOTE: If you do not see any peers, try\n"
		"    export ZSYS_INTERFACE=br1\n"
		"  (whatever is correct for you, see `ip a`) and re-run %s", argv[0]);
	char *zsys_interface = getenv("ZSYS_INTERFACE"); // NULL if not set
	char *znodename = "jim";
	char *mnodename = "JIM-MLM";
	char *zchatname = "MALAMUTE";
	char *endpoint = NULL;	 // Used for malamute connection, mine or remote
	char *myendpoint = NULL; // What we announce
	mlm_client_t *client = NULL;
	int i;

	for (i=1; i<argc; i++) {
		if (streq(argv[i],"-mn")) {
			assert ( (i+1) < argc );
			mnodename = argv[++i];
		} else if (streq(argv[i],"-zn")) {
			assert ( (i+1) < argc );
			znodename = argv[++i];
		} else if (streq(argv[i],"-zc")) {
			assert ( (i+1) < argc );
			zchatname = argv[++i];
		} else if (streq(argv[i],"-em")) {
			assert ( (i+1) < argc );
			myendpoint = argv[++i];
		} else if (streq(argv[i],"-er")) {
			assert ( (i+1) < argc );
			endpoint = argv[++i];
		} else {
			printf("Usage: %s [-zn zyre-nodename] [-zc zyre-chatname] [-mn mlm-nodename] [-em myendpoint] [-er endpoint]\n", argv[0]);
			exit(1);
		}
	}

	if (myendpoint == NULL) {
		struct ifaddrs *ifaddr, *ifa;
		int family, s, n;
		char host[NI_MAXHOST];
		host[0]='\0';

		if (getifaddrs(&ifaddr) == -1) {
			perror("getifaddrs");
		} else {
			for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++) {
				if (ifa->ifa_addr == NULL)
					continue;
				if (zsys_interface != NULL) {
					// Only match this interface
					if ( ! streq(ifa->ifa_name, zsys_interface) )
						continue;
				} else {
					if ( streq(ifa->ifa_name, "lo") )
						continue;
				}

				family = ifa->ifa_addr->sa_family;

				if (family == AF_INET || family == AF_INET6) {
					s = getnameinfo(ifa->ifa_addr,
					    (family == AF_INET) ? sizeof(struct sockaddr_in) :
					                          sizeof(struct sockaddr_in6),
					    host, NI_MAXHOST,
					    NULL, 0, NI_NUMERICHOST);
					if (s != 0) {
						printf("getnameinfo() failed: %s\n", gai_strerror(s));
						host[0]='\0';
					}

					zsys_debug("My address at %s: <%s>", ifa->ifa_name, host);
					break;
				}
			}
			freeifaddrs(ifaddr);
		}

		if (host[0] == '\0') {
			myendpoint = "tcp://127.0.0.1:9999";
		} else {
			size_t len = strlen("tcp://:9999")+strlen(host);
			myendpoint = malloc(len+1);
			snprintf(myendpoint, len+1, "tcp://%s:9999", host);
		}
	}

	if (endpoint == NULL)
		endpoint = myendpoint;

	zsys_info("Using endpoint:   '%s' (where malamute client connects to)", endpoint);
	zsys_info("Using myendpoint: '%s' (what we announce if we are leader)", myendpoint);
	zsys_info("Using nodename: '%s'", znodename);
	zsys_info("Using chatname: '%s'", zchatname);

	int myState = 0;	// 0 = follower, 1 = leader (malamute server)
	int oldState = 0;
	zactor_t *broker = NULL;

	zyre_t *node = zyre_new(znodename);
	assert(node);
	zyre_start(node);
	zyre_join(node,zchatname);

        zsys_debug("Bumping verbosity...");
        zyre_set_verbose(node);

	char *uuid_local = strdup(zyre_uuid(node));
	char *uuid_leader = NULL;
	char *name_leader = NULL;
	int64_t time_leader = -1; //monotonic milliseconds

	while (!zsys_interrupted) {
		oldState = myState;
                if (myState == 1) {
			zyre_shouts (node, zchatname, myendpoint);
		}

		zsys_debug("Current leader is node '%s' with uuid=%s last seen %"PRId64" milliseconds ago",
			name_leader, uuid_leader,
			(time_leader>0) ? (zclock_mono() - time_leader) : 0);

//		zsys_debug("Waiting for zyre_event_new()");
                zpoller_t *poller = zpoller_new(zyre_socket(node), NULL);
                zyre_event_t *event = NULL;
                void *which = zpoller_wait (poller, 1000); // max 1 sec
		if (which && which == zyre_socket(node) )
                        event = zyre_event_new (node);
//		zsys_debug("Returned from zyre_event_new(): %s", event ? "ok" : "null");

		// Timeout so we start a server if there is none,
                // or reconnect to another leader as we find one
		if (time_leader > 0) {
			if ( (zclock_mono()-time_leader) > 4999) {
				zsys_debug("LOST current leader node '%s' with uuid=%s which was last seen %"PRId64" milliseconds ago",
					name_leader, uuid_leader,
					(zclock_mono() - time_leader));
				if (uuid_leader)
					free(uuid_leader);
				uuid_leader = NULL;
				if (name_leader)
					free(name_leader);
				name_leader = NULL;
				time_leader = -1;
			}
		}

		if (!event) {
                        if (uuid_leader == NULL) {
                                zsys_info("No known leader and no messages, I am lonely, so I will lead myself (and launch the mlm)");
                                // TODO: Make functions!
				if (uuid_leader)
					free(uuid_leader);
				uuid_leader = strdup(uuid_local);
				if (name_leader)
					free(name_leader);
				name_leader = strdup("<myself>");
				time_leader = -2;
				myState = 1;

        			zsys_info("State changed: starting broker");
                                broker = zactor_new (mlm_server, NULL);
                        	zsock_send (broker, "ss", "BIND", myendpoint);
        			zstr_send (broker, "VERBOSE");
                		zsys_info("I AM LEADER");
                        }
			continue ;
                }

		zyre_event_print(event);

		const char *uuid_remote = zyre_event_sender(event);

		if (strcmp (uuid_remote, uuid_local) > 0) {
			zsys_info("Remote node '%s' has higher uuid than mine (%s vs %s)",
			    zyre_event_name(event), uuid_remote, uuid_local );
			myState = 0;
			if (uuid_leader == NULL) {
				if (uuid_leader)
					free(uuid_leader);
				uuid_leader = strdup(uuid_remote);
				if (name_leader)
					free(name_leader);
				name_leader = strdup(zyre_event_name(event));
			} else {
				if (strcmp (uuid_remote, uuid_leader) > 0) {
					// TODO: Reconnect to new leader
                                        // read zyre-msg to get endpoint
					if (uuid_leader)
						free(uuid_leader);
					uuid_leader = strdup(uuid_remote);
					if (name_leader)
						free(name_leader);
					name_leader = strdup(zyre_event_name(event));
				}
			}
		} else if (strcmp (uuid_remote, uuid_local) < 0) {
			zsys_info("Remote node '%s' has lower uuid than mine (%s vs %s)",
			    zyre_event_name(event), uuid_remote, uuid_local );
			if (uuid_leader == NULL) {
				if (uuid_leader)
					free(uuid_leader);
				uuid_leader = strdup(uuid_local);
				if (name_leader)
					free(name_leader);
				name_leader = strdup("<myself>");
				time_leader = -2;
				myState = 1;
			} else {
				if (strcmp (uuid_local, uuid_leader) > 0) {
					if (uuid_leader)
						free(uuid_leader);
					uuid_leader = strdup(uuid_local);
					if (name_leader)
						free(name_leader);
					name_leader = strdup("<myself>");
					time_leader = -2;
					myState = 1;
				} else if (strcmp (uuid_local, uuid_leader) < 0) {
					// The known leader is still higher
					myState = 0;
				} // else I am the leader
			}
		} else {
			zsys_info("Remote node '%s' has same uuid as mine (%s vs %s)",
			    zyre_event_name(event), uuid_remote, uuid_local );
			zsys_info("I AM CONFUSED");
		}

		if (strcmp (uuid_remote, uuid_leader) == 0) {
			// We have seen the remote leader recently,
			// including maybe it was just assigned
			time_leader = zclock_mono();
		}

		if (myState == 0 && oldState == 1) {
			zsys_info("State changed: killing broker");
			assert (broker);
                        // oldState==1 means we had a broker
                        // so null is a bad error
			zactor_destroy (&broker);
			broker = NULL;

			// TODO: connect to a leader
                        // read zyre-msg to get endpoint
			if (client) {
				zsys_info("MY_MLM_CLIENT_STOP");
				mlm_client_destroy (&client);
                                client = NULL;
			}
			// create new client to malamute of the leader

			client = mlm_client_new();
//			mlm_client_connect (client, endpoint, 1000, mnodename);

			zsys_info("I AM NOW FOLLOWER of '%s' (%s) at %s", name_leader, uuid_leader, endpoint);
		} else if (myState == 1 && oldState != 1) {
			zsys_info("State changed: starting broker");
			broker = zactor_new (mlm_server, NULL);
			zsock_send (broker, "ss", "BIND", myendpoint);
			zstr_send (broker, "VERBOSE");
			zsys_info("I AM NOW THE LEADER at %s", myendpoint);
		}

		zyre_event_destroy(&event);
	}

        zsys_info ("The loop is broken, cleaning up");

        zsys_debug("Leaving...");
	zyre_leave(node, zchatname);

        zsys_debug("Stopping...");
	zyre_stop(node);

        zsys_debug("Destroying...");
	zyre_destroy(&node);

        zsys_debug("Freeing...");
	if (uuid_local)
		free(uuid_local);
	if (uuid_leader)
		free(uuid_leader);
	if (name_leader)
		free(name_leader);
	if (broker)
		zactor_destroy (&broker);
	if (client)
		mlm_client_destroy (&client);

        zsys_info("Sayonara");
	return 0;
}
