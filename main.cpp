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
					outgoingSock.sockClose();
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
				outgoingSock.sockClose();
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
			if( !outgoingSock.send(buff.c_str(), dstaddr) ){
				printf("Can't send datagram. Aborted.\n");
				outgoingSock.sockClose();
				exit(1);
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

		for( size_t i = 0; i < argNum; ++i ){

			if( ips[i].key == src ){
				if( !incomingSock.init(ips[i]) ) {
					printf("Initialization failed. Try again later.\n");
					incomingSock.sockClose();
					exit(1);
				}
			}

			if( ips[i].key == local ){
				incomingSock.set_localIpAddr(ips[i].addrInfo);
			}

			if( !(ips[i].key == local || ips[i].key == src) ) usage();
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
					srcSock.sockClose();
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

		srcSock.polling(dstSock);
		break;
	}
	default:
		printf("Unknown situation. Program is terminated now.");
		exit(1);
	}

	return 0;
}


