/*
 * misc.h
 *
 *  Created on: 25 нояб. 2022 г.
 *      Author: Omineva
 */

#ifndef MISC_H_
#define MISC_H_

#include <arpa/inet.h>		// inet_pton
#include <cerrno>
#include <cstdio>			// fprintf
#include <cstring>			// strcmp
#include <list>
#include <netinet/in.h>		// sockaddr_in, in_addr, htons
#include <stdlib.h>			// exit
#include <string>
#include <vector>

enum cmdKeyword {

	multicast,
	src,
	dst,
};

enum modes {

	multicast_generator,	// [--mtg host port localhost localport]
	sender,					// [--send srchost srcport dsthost dstport]
	receiver,				// [--recv host port]
	switcher,				// [--src host port --dst host port]
};

struct addrStruct {

	sockaddr_in addrInfo;
	cmdKeyword key;
};

bool validateIp(char* str, sockaddr_in& addrInfoOut);
bool validatePort(char* str, sockaddr_in& addrInfoOut);
void usage();
bool isMulticast(const in_addr ip);
void parseCmd(int argc, char *argv[], std::vector<addrStruct>& ipsOut, modes &modeOut);

#endif /* MISC_H_ */
