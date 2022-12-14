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

public:
	Sock();
	~Sock();
	void sockClose();
	bool init(const addrStruct &ips);
	int get_sockfd();
	sockaddr_in get_addr();
	sockaddr_in get_localIpAddr();
	void set_addr(const sockaddr_in &ip);
	void set_localIpAddr(const sockaddr_in &ip);
	bool multicastIf(const in_addr &localInterface);
	bool addMulticastGroup(const in_addr &local);
	bool send(const char* databuf, const sockaddr_in dstaddr);
	bool polling();
	bool polling(Sock &dst);

private:
	int m_sockfd;
	sockaddr_in m_ipaddr;
	sockaddr_in m_localip;
};

#endif /* SOCKETS_H_ */
