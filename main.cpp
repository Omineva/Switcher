/*
 * main.cpp
 *
 *  Created on: 6 дек. 2022 г.
 *      Author: Omineva
 */

#include "misc.h"
#include "sockets.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {

	std::vector<addrStruct> ips;
	modes mode;

	parseCmd(argc,argv,ips,mode);

	for (size_t i = 0; i < ips.size(); ++i) {
		printf("ips.key = %d, ips.ipaddr = %u, mode = %d\n",ips[i].key,ips[i].addrInfo.sin_addr.s_addr, mode);
		if ( isMulticast(ips[i].addrInfo.sin_addr) ) {
			printf("ips[%ld].addrInfo.sin_addr = true\n",i);
		}
	}

	switch(mode) {
	case sender:
	{
		// TODO: add check of ips in array
		Sock srcSock;
		sockaddr_in dstaddr;

		for (size_t i = 0; i < ips.size(); ++i) {
			if ( ips[i].key == src ) {
				if ( !srcSock.init(ips[i]) ) {
					printf("Initialization failed. Try again later.\n");
					exit(1);
				}
			}
			if ( ips[i].key == dst ) {
				dstaddr = ips[i].addrInfo;
			}
		}

		char databuf[80] = "Unicast datagram - 0";
		int count = 0, ones = 0;

		while (true) {
			databuf[19] = '0' + ones;
			++ones;
			if ( ones == 10 ) {
				ones = 0;
			}

			// sending datagrams to dstaddr
			if ( !srcSock.send(databuf, dstaddr) ) {
				printf("Can't send datagram. Aborted.\n");
				break;
			}

			++count;
			if ( count == 100 ) {
				break;
			}
			sleep(2);
		}
		break;
	}
	case receiver:
		printf("receiver");
		break;
	case multicast_generator:
	{
		Sock mc;
		sockaddr_in multiaddr;

		for (size_t i = 0; i < ips.size(); ++i) {
			if ( ips[i].key == multicast ) {
				multiaddr = ips[i].addrInfo;
			}
			if ( ips[i].key == src ) {
				if ( !mc.init(ips[i]) ) {
					printf("Initialization failed. Try again later.\n");
					exit(1);
				}
			}
		}

		if ( mc.multicastIf(mc.get_addr()) ) {

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
		break;
	default:
		printf("Unknown situation. Program is terminated now.");
		exit(1);
	}

	// TODO: destroy all
	return 0;
}


