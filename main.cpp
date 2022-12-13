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
					exit(1);
				}
			}

			if( ips[i].key == dst ){
				dstaddr = ips[i].addrInfo;
			}

			if( !(ips[i].key == local || ips[i].key == dst) ) usage();
		}

		int count = 0;
		std::string templateDatagramMsg = "Unicast datagram №";

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
					exit(1);
				}
			}

			if( ips[i].key == local ){
				localaddr = ips[i].addrInfo;
			}

			if( !(ips[i].key == local || ips[i].key == src) ) usage();
		}

		if( isMulticast(incomingSock.get_addr().sin_addr) ){
			incomingSock.addMulticastGroup(localaddr.sin_addr);
		}

		incomingSock.polling();
		break;
	}
	case multicast_generator:
	{
		size_t argNum = ips.size();
		if( argNum != 2 ) usage();

		Sock mc;
		sockaddr_in multiaddr;

		for( size_t i = 0; i < argNum; ++i ){

			if( ips[i].key == multicast ){
				multiaddr = ips[i].addrInfo;
			}

			if( ips[i].key == local ){
				if( !mc.init(ips[i]) ){
					printf("Initialization failed. Try again later.\n");
					exit(1);
				}
			}

			if( !(ips[i].key == local || ips[i].key == multicast) ) usage();
		}

		if( mc.multicastIf(mc.get_addr().sin_addr) ){

			char databuf[80] = "Multicast datagram - 0";
			int count = 0, ones = 0;

			while (true) {
				databuf[21] = '0' + ones;
				++ones;
				if ( ones == 10 ) {
					ones = 0;
				}

				// sending datagrams to multicast addr
				if ( !mc.send(databuf, multiaddr) ) {
					printf("Can't send datagram. Aborted.\n");
					break;
				}

				++count;
				if ( count == 100 ) {
					break;
				}
				sleep(2);
			}
		}
		break;
	}
	case switcher:
	{
		size_t argNum = ips.size();
		if( argNum != 3 ) usage();

		Sock srcSock;
		Sock dstSock;
		sockaddr_in localaddr;

		for( size_t i = 0; i < argNum; ++i ){

			if( ips[i].key == src ){
				if( !srcSock.init(ips[i]) ){
					printf("Initialization failed. Try again later.\n");
					exit(1);
				}
			}

			if( ips[i].key == dst ){
				dstSock.set_addr(ips[i].addrInfo);
			}

			if( ips[i].key == local ){
				localaddr = ips[i].addrInfo;
			}

			if( !(ips[i].key == src || ips[i].key == dst || ips[i].key == local) ) usage();
		}

		if( isMulticast(srcSock.get_addr().sin_addr) ){
			srcSock.addMulticastGroup(localaddr.sin_addr);
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


