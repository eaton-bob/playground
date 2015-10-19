//
//  UPS
//  Binds REP socket to tcp://*:5560
//  Publish "ON" or "OFF"
//
#include <zmq.hpp>
#include <string>
#include <iostream>
#include <unistd.h>


bool ison()
{
	return true;
}

int main () {
    //  Prepare our context and socket
    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_PUB);
    socket.connect ("tcp://*:5560");

    while (1) {

        //  Publish status
		
		if(ison())
		{
			zmq::message_t pub (2);
			memcpy ((void *) pub.data (), "ON", 2);
			socket.send (pub);
		}
		else
		{
			zmq::message_t pub (3);
			memcpy ((void *) pub.data (), "OFF", 3);
			socket.send (pub);
		}
		
		// wait 1 sec
		sleep(1);
    }
    return 0;
}
