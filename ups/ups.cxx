//
//  UPS
//  Binds REP socket to tcp://*:5560
//  Publish "ON" or "OFF"
//
#include <zmq.hpp>
#include <string>
#include <sstream>
#include <iostream>
#include <unistd.h>


bool ison()
{
	return true;
}


int main (int argc, char ** argv) {

	if(argc<2)
	{
		std::cout << "Usage: " << argv[0] << " <upsname>" << std::endl;
		return -1;
	}
	
	std::string upsname = argv[1];
	
    //  Prepare our context and socket
    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_PUB);
    socket.connect ("tcp://*:5560");

    while (1) {

        //  Publish status
		socket.send(upsname.data(), upsname.size(), ZMQ_SNDMORE);
		if(ison())
			socket.send("ON", 2, 0);
		else
			socket.send("OFF", 3, 0);
		
		// wait 1 sec
		sleep(1);
    }
    return 0;
}
