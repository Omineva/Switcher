/*
 * sockets.h
 *
 *  Created on: 6 дек. 2022 г.
 *      Author: tvv
 */

#ifndef SOCKETS_H_
#define SOCKETS_H_

#include <arpa/inet.h>
#include <netinet/in.h>

class Sock {

	int m_sockfd;
	sockaddr_in m_ipaddr;

public:
	Sock();
	~Sock();
	bool init(const addrStruct &ips);
	in_addr get_addr();
	bool multicastIf(const in_addr &localInterface);
	bool send(const char* databuf, const sockaddr_in dstaddr);
};

#endif /* SOCKETS_H_ */
