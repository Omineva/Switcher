/*
 * sockets.cpp
 *
 *  Created on: 6 дек. 2022 г.
 *      Author: Omineva
 */

#include "misc.h"
#include "sockets.h"
#include <cstdio>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

Sock::Sock() {

	m_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	printf("m_sockfd = %d\n", m_sockfd);
	if ( m_sockfd < 0 ) {
		perror("Error (create socket failed)");
		exit(1);
	}
}

Sock::~Sock() {

// TODO: destroy object
}

bool Sock::init(const addrStruct &ips) {

	m_ipaddr = ips.addrInfo;

	int res = bind(m_sockfd, (sockaddr*)&m_ipaddr, sizeof(m_ipaddr));
	printf("bind (local iface) res = %d\n", res);
	if ( res != 0 ) {
		perror("Error (bind failed)");
		close(m_sockfd);
		m_sockfd = -1;
		return false;
	}

	return true;
}

in_addr Sock::get_addr() {

	return m_ipaddr.sin_addr;
}

bool Sock::multicastIf(const in_addr &localInterface) {

	if ( m_sockfd != -1 ) {
		int res = setsockopt(m_sockfd, IPPROTO_IP, IP_MULTICAST_IF,(char *)&localInterface,sizeof(localInterface));
		printf("setsockopt (multicast iface) res = %d\n", res);
		if ( res != 0 ) {
			perror("Error (setting up multicast interface failed)");
			close(m_sockfd);
			m_sockfd = -1;
			return false;
		}
	}
	else return false;

	return true;
}

bool Sock::send(const char* databuf, const sockaddr_in dstaddr) {

	int datalen = 80;
	if ( m_sockfd != -1 ) {
		int res = sendto(m_sockfd, databuf, datalen, 0, (sockaddr*)&dstaddr, sizeof(dstaddr));
		printf("sending datagram: %s\n", databuf);
		if ( res < 0 ) {
			perror("Error (sending multicast message failed)");
			close(m_sockfd);
			m_sockfd = -1;
			return false;
		}
	}
	else return false;

	return true;
}

