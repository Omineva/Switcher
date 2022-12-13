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
	if ( res != 0 ) {
		perror("Error (bind failed)");
		close(m_sockfd);
		m_sockfd = -1;
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

	if ( m_sockfd != -1 ) {
		int res = setsockopt(m_sockfd, IPPROTO_IP, IP_MULTICAST_IF,(char *)&localInterface,sizeof(localInterface));
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

bool Sock::addMulticastGroup(const in_addr &local) {

	ip_mreq group;
	group.imr_multiaddr = m_ipaddr.sin_addr;
	group.imr_interface = local;

	if ( m_sockfd != -1 ) {
		int res = setsockopt(m_sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP,(char *)&group, sizeof(group));
		if ( res != 0 ) {
			perror("Error (setting up IP_ADD_MEMBERSHIP failed)");
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
			perror("Error (sending message failed)");
			close(m_sockfd);
			m_sockfd = -1;
			return false;
		}
	}
	else return false;

	return true;
}

bool Sock::polling() {

	char databuf[80];
	struct pollfd fds[2];
	int numfds = 1;
	int timeout = -1;

	memset(fds, 0, sizeof(fds));
	fds[0].fd = m_sockfd;
	fds[0].events = POLLIN;

	while (true) {

		if ( poll(fds, numfds, timeout) > 0 ) {

			if ( m_sockfd != -1 ) {
				int res = read(m_sockfd, databuf, sizeof(databuf));
				printf("received datagram: %s\n",databuf);
				if ( res < 0 ) {
					perror("Error (reading message failed)");
					close(m_sockfd);
					m_sockfd = -1;
					return false;
				}
			}
		}
		else {
			perror("No poll events");
			close(m_sockfd);
			m_sockfd = -1;
			break;
		}
	}
	return true;
}

bool Sock::polling(Sock &dst) {

	char databuf[80];
	struct pollfd fds[2];
	int numfds = 1;
	int timeout = -1;

	memset(fds, 0, sizeof(fds));
	fds[0].fd = m_sockfd;
	fds[0].events = POLLIN;

	while (true) {

		if ( poll(fds, numfds, timeout) > 0 ) {

			int res;
			if ( m_sockfd != -1 ) {
				res = read(m_sockfd, databuf, sizeof(databuf));
				printf("received datagram: %s\n",databuf);
				if ( res < 0 ) {
					perror("Error (reading message failed)");
					close(m_sockfd);
					m_sockfd = -1;
					return false;
				}
			}
			if ( dst.get_sockfd() != -1) {
				sockaddr_in tmp = dst.get_addr();
				res = sendto(dst.get_sockfd(), databuf, sizeof(databuf), 0,(sockaddr*)&tmp,sizeof(tmp));
				printf("re-sent datagram: %s\n\n",databuf);
				if ( res < 0 ) {
					perror("Error (re-sending message failed)");
					close(dst.get_sockfd());
					return false;
				}
			}
		}
		else {
			perror("No poll events");
			close(m_sockfd);
			m_sockfd = -1;
			break;
		}
	}
	return true;
}
