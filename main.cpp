/*
 * main.cpp
 *
 *  Created on: 6 дек. 2022 г.
 *      Author: Omineva
 */

#include "misc.h"
#include "sockets.h"
#include <sstream>
#include <unistd.h>

int main(int argc, char *argv[]) {

	std::vector<addrStruct> ips;
	modes mode;

	parseCmd(argc,argv,ips,mode);

	switch( mode ){
	case sender:
	{
		size_t argNum = ips.size();
		if( argNum != 2 ) usage();

		Sock outgoingSock;
		sockaddr_in dstaddr;

		for( size_t i = 0; i < argNum; ++i ){

			if( ips[i].key == local ){
				if( !outgoingSock.init(ips[i]) ){
					printf("Initialization failed. Try again later.\n");
					// TODO: destroy objects
					exit(1);
				}
			}

			if( ips[i].key == dst ){
				dstaddr = ips[i].addrInfo;
			}

			if( !(ips[i].key == local || ips[i].key == dst) ) usage();
		}

		int count = 0;
		std::string templateDatagramMsg;

		// check if dstaddr is multicast => to set the interface for sending outbound multicast datagrams
		if( isMulticast(dstaddr.sin_addr) ){
			if( outgoingSock.multicastIf(outgoingSock.get_addr().sin_addr) ){
				templateDatagramMsg = "Mulicast datagram №";
			} else {
				printf("Can't set interface for sending outbound multicast datagrams.\n");
				// TODO: destroy objects
				exit(1);
			}
		} else {
			templateDatagramMsg = "Unicast datagram №";
		}

		while (true) {

			std::string buff = templateDatagramMsg;
			std::ostringstream convert;
			convert << count;
			buff += convert.str();

			// sending datagrams to dstaddr
			if ( !outgoingSock.send(buff.c_str(), dstaddr) ) {
				printf("Can't send datagram. Aborted.\n");
				break;
			}

			++count;
			sleep(1);
		}
		break;
	}
	case receiver:
	{
		size_t argNum = ips.size();
		if( argNum != 2 ) usage();

		Sock incomingSock;
		sockaddr_in localaddr;

		for( size_t i = 0; i < argNum; ++i ){

			if( ips[i].key == src ){
				if( !incomingSock.init(ips[i]) ) {
					printf("Initialization failed. Try again later.\n");
					// TODO: destroy objects
					exit(1);
				}
			}

			if( ips[i].key == local ){
				localaddr = ips[i].addrInfo;
			}

			if( !(ips[i].key == local || ips[i].key == src) ) usage();
		}

		// check receiving from multicast group => to join a multicast group on a local interface to receive multicast datagrams
		if( isMulticast(incomingSock.get_addr().sin_addr) ){
			if( !incomingSock.addMulticastGroup(localaddr.sin_addr) ){
				printf("Can't join interface for receiving inbound multicast datagrams.\n");
				// TODO: destroy objects
				exit(1);
			}
		}

		incomingSock.polling();
		break;
	}
	case switcher:
	{
		size_t argNum = ips.size();
		if( argNum != 3 ) usage();

		Sock srcSock;
		Sock dstSock;

		for( size_t i = 0; i < argNum; ++i ){

			if( ips[i].key == src ){
				if( !srcSock.init(ips[i]) ){
					printf("Initialization failed. Try again later.\n");
					// TODO: destroy objects
					exit(1);
				}
			}

			if( ips[i].key == dst ){
				dstSock.set_addr(ips[i].addrInfo);
			}

			if( ips[i].key == local ){
				srcSock.set_localIpAddr(ips[i].addrInfo);
				dstSock.set_localIpAddr(ips[i].addrInfo);
			}

			if( !(ips[i].key == src || ips[i].key == dst || ips[i].key == local) ) usage();
		}

		// check if srcaddr is multicast => to join a multicast group on a local interface to receive multicast datagrams
		if( isMulticast(srcSock.get_addr().sin_addr) ){
			if( !srcSock.addMulticastGroup(srcSock.get_localIpAddr().sin_addr) ){
				printf("Can't join interface for receiving inbound multicast datagrams.\n");
				// TODO: destroy objects
				exit(1);
			}
		}

		// check if dstaddr is multicast => to set the interface for sending outbound multicast datagrams
		if( isMulticast(dstSock.get_addr().sin_addr) ){
			if( !dstSock.multicastIf(dstSock.get_localIpAddr().sin_addr) ){
				printf("Can't set interface for sending outbound multicast datagrams.\n");
				// TODO: destroy objects
				exit(1);
			}
		}

		srcSock.polling(dstSock);
		break;
	}
	default:
		printf("Unknown situation. Program is terminated now.");
		exit(1);
	}

	// TODO: destroy all
	return 0;
}


