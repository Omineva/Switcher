/*
 * sockets.cpp
 *
 *  Created on: 6 дек. 2022 г.
 *      Author: Omineva
 */

#include "misc.h"
#include "sockets.h"
#include <sys/poll.h>
#include <unistd.h>

Sock::Sock() {

	m_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if( m_sockfd < 0 ){
		perror("Error (create socket failed)");
		exit(1);
	}
}

Sock::~Sock() {

	sockClose();
}

void Sock::sockClose() {

	close(m_sockfd);
	m_sockfd = -1;
}

bool Sock::init(const addrStruct &ips) {

	m_ipaddr = ips.addrInfo;

	if( bind(m_sockfd, (sockaddr*)&m_ipaddr, sizeof(m_ipaddr)) != 0 ){
		perror("Error (bind failed)");
		sockClose();
		return false;
	}
	return true;
}

int Sock::get_sockfd() {

	return m_sockfd;
}

sockaddr_in Sock::get_addr() {

	return m_ipaddr;
}

sockaddr_in Sock::get_localIpAddr() {

	return m_localip;
}

void Sock::set_addr(const sockaddr_in &ip) {

	m_ipaddr = ip;
}

void Sock::set_localIpAddr(const sockaddr_in &ip) {

	m_localip = ip;
}

bool Sock::multicastIf(const in_addr &localInterface) {

	if( m_sockfd != -1 ){
		if( setsockopt(m_sockfd, IPPROTO_IP, IP_MULTICAST_IF,(char *)&localInterface,sizeof(localInterface)) != 0 ){
			perror("Error (setting up multicast interface failed)");
			sockClose();
			return false;
		}
	} else return false;

	return true;
}

bool Sock::addMulticastGroup(const in_addr &local) {

	ip_mreq group;
	group.imr_multiaddr = m_ipaddr.sin_addr;
	group.imr_interface = local;

	if( m_sockfd != -1 ){
		if( setsockopt(m_sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP,(char *)&group, sizeof(group)) != 0 ){
			perror("Error (setting up IP_ADD_MEMBERSHIP failed)");
			sockClose();
			return false;
		}
	} else return false;

	return true;
}

bool Sock::send(const char* databuf, const sockaddr_in dstaddr) {

	if( m_sockfd != -1 ){
		if( sendto(m_sockfd, databuf, MSGSIZE, 0, (sockaddr*)&dstaddr, sizeof(dstaddr)) < 0 ){
			perror("Error (sending message failed)");
			sockClose();
			return false;
		} else printf("sending datagram: %s\n", databuf);
	} else return false;

	return true;
}

bool Sock::polling() {

	char databuf[MSGSIZE];
	struct pollfd fds[1];
	int numfds = 1;
	int timeout = -1;

	memset(fds, 0, sizeof(fds));
	fds[0].fd = m_sockfd;
	fds[0].events = POLLIN;

	while (true) {
		if( poll(fds, numfds, timeout) > 0 ){
			if( m_sockfd != -1 ){
				if( read(m_sockfd, databuf, sizeof(databuf)) < 0 ){
					perror("Error (reading message failed)");
					sockClose();
					return false;
				} else printf("received datagram: %s\n",databuf);
			}
		} else {
			perror("No poll events");
			sockClose();
			break;
		}
	}
	return true;
}

bool Sock::polling(Sock &dst) {

	char databuf[MSGSIZE];
	struct pollfd fds[1];
	int numfds = 1;
	int timeout = -1;

	memset(fds, 0, sizeof(fds));
	fds[0].fd = m_sockfd;
	fds[0].events = POLLIN;

	while (true) {
		if( poll(fds, numfds, timeout) > 0 ){
			if( m_sockfd != -1 ){
				if( read(m_sockfd, databuf, sizeof(databuf)) < 0 ){
					perror("Error (reading message failed)");
					sockClose();
					return false;
				} else printf("received datagram: %s\n",databuf);
			}
			if( dst.get_sockfd() != -1){
				sockaddr_in tmp = dst.get_addr();
				if( sendto(dst.get_sockfd(), databuf, sizeof(databuf), 0,(sockaddr*)&tmp,sizeof(tmp)) < 0 ){
					perror("Error (re-sending message failed)");
					dst.sockClose();
					return false;
				} else printf("re-sent datagram: %s\n\n",databuf);
			}
		} else {
			perror("No poll events");
			sockClose();
			break;
		}
	}
	return true;
}
