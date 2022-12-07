/*
 * misc.cpp
 *
 *  Created on: 25 нояб. 2022 г.
 *      Author: Omineva
 */

#include "misc.h"
#include <iostream>			// TO DELETE

bool validateIp(char* str, sockaddr_in& addrInfoOut) {

	bool result;
	in_addr ip_to_num;

	if ( inet_pton(AF_INET, str, &ip_to_num) > 0 ) {
		memset(&addrInfoOut, 0, sizeof(addrInfoOut));
		addrInfoOut.sin_family = AF_INET;
		addrInfoOut.sin_addr = ip_to_num;
		result = true;
	}
	else {
//TODO: адекватное сообщение влог
		result = false;
	}

	return result;
}

bool validatePort(char* str, sockaddr_in& addrInfoOut) {

	bool result;
	char* tmpPtr = str;
	unsigned short port;

	while ( *tmpPtr != '\0' ) {
		switch(*tmpPtr) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			result = true;
			break;
		default:
//TODO: адекватное сообщение в лог
			result = false;
			break;
		}
		if ( !result ) break;
		++tmpPtr;
	}

	if ( result ) {
		int tmp = atoi(str);
		if ( tmp >= 0 && tmp < 65536 ) {
			port = static_cast<unsigned short>(tmp);
			addrInfoOut.sin_port = htons(port);
		}
		else {
//TODO: адекватное сообщение в лог
			result = false;
		}
	}

	return result;
}

void usage() {

	fprintf(stderr, "Usage: switcher [--mtg host port localhost localport] OR [--src host port --dst host port] "
			"OR [--send srchost srcport dsthost dstport] OR [--recv host port]\n");
	exit(1);
}

bool isMulticast(const in_addr ip) {

	unsigned char * first = (unsigned char *)&ip.s_addr;

	return ( *first >= 224 && *first <= 239) ? true : false;
}

void parseCmd(int argc, char *argv[], std::vector<addrStruct>& ipsOut, modes &modeOut) {

	char** ptr;
	ptr = argv + 1;

	while ( *ptr && *ptr[0] == '-') {

		addrStruct tmp;
		if (!strcmp(*ptr,"--mtg")) {
			++ptr;

			// check and save multicast IP address
			if ( !(*ptr) || !(validateIp(*ptr,tmp.addrInfo))) usage();
			++ptr;

			// check and save multicast port
			if ( !(*ptr) || !(validatePort(*ptr,tmp.addrInfo)) ) usage();

			if ( !(isMulticast(tmp.addrInfo.sin_addr)) ) usage();
			tmp.key = multicast;
			ipsOut.push_back(tmp);
			++ptr;

			// check and save local IP address
			if ( !(*ptr) || !(validateIp(*ptr,tmp.addrInfo))) usage();
			++ptr;

			// check and save local port
			if ( !(*ptr) || !(validatePort(*ptr,tmp.addrInfo)) ) usage();

			if ( isMulticast(tmp.addrInfo.sin_addr) ) usage();
			tmp.key = src;
			ipsOut.push_back(tmp);

			modeOut = multicast_generator;
			break;
		}

		if (!strcmp(*ptr,"--send")) {
			++ptr;

			// parse src host and port
			if ( !(*ptr) || !(validateIp(*ptr,tmp.addrInfo))) usage();
			++ptr;

			if ( !(*ptr) || !(validatePort(*ptr,tmp.addrInfo)) ) usage();
			++ptr;

			tmp.key = src;
			ipsOut.push_back(tmp);

			// parse dst host and port
			if ( !(*ptr) || !(validateIp(*ptr,tmp.addrInfo))) usage();
			++ptr;

			if ( !(*ptr) || !(validatePort(*ptr,tmp.addrInfo)) ) usage();

			tmp.key = dst;
			ipsOut.push_back(tmp);
			modeOut = sender;
		}

		if (!strcmp(*ptr,"--recv")) {
			++ptr;

			// parse src host and port
			if ( !(*ptr) || !(validateIp(*ptr,tmp.addrInfo))) usage();
			++ptr;

			if ( !(*ptr) || !(validatePort(*ptr,tmp.addrInfo)) ) usage();

			tmp.key = dst;
			ipsOut.push_back(tmp);
			modeOut = receiver;
		}

		if (!strcmp(*ptr,"--src")) {
			++ptr;

			// check and save IP address
			if ( !(*ptr) || !(validateIp(*ptr,tmp.addrInfo))) usage();
			++ptr;

			// check and save port
			if ( !(*ptr) || !(validatePort(*ptr,tmp.addrInfo)) ) usage();

			tmp.key = src;
			ipsOut.push_back(tmp);
			modeOut = switcher;
		}

		if (!strcmp(*ptr,"--dst")) {
			++ptr;

			// check and save IP address
			if ( !(*ptr) || !(validateIp(*ptr,tmp.addrInfo))) usage();
			++ptr;

			// check and save port
			if ( !(*ptr) || !(validatePort(*ptr,tmp.addrInfo)) ) usage();

			tmp.key = dst;
			ipsOut.push_back(tmp);
			modeOut = switcher;
		}

		++ptr;
	}
}
