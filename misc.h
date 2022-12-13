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
#include <ifaddrs.h>		// getifaddrs, ifaddrs
#include <netinet/in.h>		// sockaddr_in, in_addr, htons
#include <stdlib.h>			// exit
#include <string>
#include <vector>

enum cmdKeyword {

	src,
	dst,
	local,
};

enum modes {

	sender,					// [--send host port --iface name]
	receiver,				// [--recv host port --iface name]
	switcher,				// [--src host port --dst host port --iface name]
};

struct addrStruct {

	sockaddr_in addrInfo;
	cmdKeyword key;
};

// size of data in datagram
const int MSGSIZE = 100;

bool validateIp(char* str, sockaddr_in& addrInfoOut);
bool validatePort(char* str, sockaddr_in& addrInfoOut);
void usage();
bool isMulticast(const in_addr ip);
bool getIpForIface(const char* iname, char* bufferOut, sockaddr_in& addrInfoOut);
void parseCmd(int argc, char *argv[], std::vector<addrStruct>& ipsOut, modes &modeOut);

#endif /* MISC_H_ */
