/*
 * misc.cpp
 *
 *  Created on: 25 нояб. 2022 г.
 *      Author: Omineva
 */

#include "misc.h"

bool validateIp(char* str, sockaddr_in& addrInfoOut) {

	bool result;
	in_addr ip_to_num;

	if ( inet_pton(AF_INET, str, &ip_to_num) > 0 ) {
		memset(&addrInfoOut, 0, sizeof(addrInfoOut));
		addrInfoOut.sin_family = AF_INET;
		addrInfoOut.sin_addr = ip_to_num;
		addrInfoOut.sin_port = 0;
		result = true;
	}
	else {
//TODO: адекватное сообщение в лог
		result = false;
	}

	return result;
}

bool validatePort(char* str, sockaddr_in& addrInfoOut) {

	bool result;
	char* tmpPtr = str;
	unsigned short port;

//TODO: переделать в регулярку?...
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

	fprintf(stderr, "Usage: switcher [--mtg host port --iface name] \n\tOR [--src host port --dst host port --iface name] "
			"\n\tOR [--send host port --iface name] \n\tOR [--recv host port --iface name]\n");
	exit(1);
}

bool isMulticast(const in_addr ip) {

	unsigned char * first = (unsigned char *)&ip.s_addr;

	return ( *first >= 224 && *first <= 239) ? true : false;
}

// check if exists named interface and returns it's ONLY first ip-adress
bool getIpForIface(const char* iname, char* bufferOut, sockaddr_in& addrInfoOut) {

	bool result = false;
	struct ifaddrs* ptr_ifaddrs = 0;

	if( getifaddrs(&ptr_ifaddrs) != 0 ) {
		perror("Error (get interface ip-addresses failed)");
		freeifaddrs(ptr_ifaddrs);
		return false;
	}

	for(	struct ifaddrs* ptr_entry = ptr_ifaddrs;
			ptr_entry != 0;
	        ptr_entry = ptr_entry->ifa_next
	    )
	{
		sa_family_t address_family = ptr_entry->ifa_addr->sa_family;

		if( !strcmp(iname,ptr_entry->ifa_name) && (address_family == AF_INET) ) {

			if( ptr_entry->ifa_addr != 0 ){

				/* Convert a Internet address in binary network format for interface
				   type AF in buffer starting at CP to presentation form and place
				   result in buffer of length LEN astarting at BUF.  */
				if( inet_ntop(
					address_family,
					&((struct sockaddr_in*)(ptr_entry->ifa_addr))->sin_addr,
					bufferOut,
					INET_ADDRSTRLEN) == 0 ){
						perror("Error (ip convert to string)");
						result = false;
						break;
				}

				sockaddr_in* tmp = (struct sockaddr_in*)(ptr_entry->ifa_addr);
				addrInfoOut.sin_family = tmp->sin_family;
				addrInfoOut.sin_addr = tmp->sin_addr;
				addrInfoOut.sin_port = 0;

				result = true;
				break;
			}
		}
	}
	freeifaddrs(ptr_ifaddrs);
	return result;
}

void parseCmd(int argc, char *argv[], std::vector<addrStruct>& ipsOut, modes &modeOut) {

	char** ptr;
	ptr = argv + 1;

	if( argc == 1 ) usage();

	while( *ptr && *ptr[0] == '-' ){

		addrStruct tmp;

		if( !strcmp(*ptr,"--mtg") ){
			++ptr;

			// check and save multicast IP address
			if( !(*ptr) || !(validateIp(*ptr,tmp.addrInfo)) ) usage();
			++ptr;

			// check and save multicast port
			if( !(*ptr) || !(validatePort(*ptr,tmp.addrInfo)) ) usage();

			if( !(isMulticast(tmp.addrInfo.sin_addr)) ) usage();

			tmp.key = multicast;
			ipsOut.push_back(tmp);
			modeOut = multicast_generator;

		} else if( !strcmp(*ptr,"--send") ){
			++ptr;

			// parse dst host and port
			if( !(*ptr) || !(validateIp(*ptr,tmp.addrInfo))) usage();
			++ptr;

			if( !(*ptr) || !(validatePort(*ptr,tmp.addrInfo)) ) usage();

			tmp.key = dst;
			ipsOut.push_back(tmp);
			modeOut = sender;

		} else if( !strcmp(*ptr,"--iface") ){

			++ptr;
			char buffer[INET_ADDRSTRLEN] = {0};

			if( *ptr && getIpForIface(*ptr,buffer,tmp.addrInfo) ) {
				printf("Using ip-address: %s\n",buffer);

				tmp.key = local;
				ipsOut.push_back(tmp);

			} else {
				printf("Unkown interface.\n");
				usage();
			}
		} else if( !strcmp(*ptr,"--recv") ){
			++ptr;

			if( !(*ptr) || !(validateIp(*ptr,tmp.addrInfo))) usage();
			++ptr;

			if( !(*ptr) || !(validatePort(*ptr,tmp.addrInfo)) ) usage();

			tmp.key = src;
			ipsOut.push_back(tmp);
			modeOut = receiver;

		} else if( !strcmp(*ptr,"--src") ){
			++ptr;

			// check and save IP address
			if( !(*ptr) || !(validateIp(*ptr,tmp.addrInfo)) ) usage();
			++ptr;

			// check and save port
			if( !(*ptr) || !(validatePort(*ptr,tmp.addrInfo)) ) usage();

			tmp.key = src;
			ipsOut.push_back(tmp);
			modeOut = switcher;

		} else if( !strcmp(*ptr,"--dst") ){
			++ptr;

			// check and save IP address
			if( !(*ptr) || !(validateIp(*ptr,tmp.addrInfo))) usage();
			++ptr;

			// check and save port
			if( !(*ptr) || !(validatePort(*ptr,tmp.addrInfo)) ) usage();

			tmp.key = dst;
			ipsOut.push_back(tmp);
			modeOut = switcher;

		} else usage();

		++ptr;
	}
}
