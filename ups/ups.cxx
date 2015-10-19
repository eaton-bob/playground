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

	if(argc<3)
	{
		std::cout << "Usage: " << argv[0] << " <address> <upsname>" << std::endl;
		return -1;
	}
	
	std::string upsname = argv[2];
	
    //  Prepare our context and socket
    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_PUB);
    socket.connect ((std::string("tcp://") + argv[1] + ":5560").c_str());

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
