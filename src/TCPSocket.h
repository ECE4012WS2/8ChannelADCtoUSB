/*
 * TCPSocket.h
 *
 *  Created on: Feb 26, 2014
 *      Author: austin
 */

#ifndef TCPSOCKET_H_
#define TCPSOCKET_H_

#include "Socket.h"

class TCPSocket: public Socket
	{
	public:
		TCPSocket(const std::string& newHostname, const int32_t& newPort,
				const size_t& buffSize, const bool client = true);
		virtual ~TCPSocket();
		int32_t send(const void* sendbuff, size_t length);
		int32_t recv();
		void dontBlock();
		void enableBlock();
		virtual std::ostream& toString(std::ostream& o) const;
	private:
		int32_t open();
		socklen_t m_clilen;
		struct sockaddr_in m_cliaddr;
		int32_t m_listenfd;
		bool m_isClient;
		struct addrinfo *m_res;
	};

#endif /* TCPSOCKET_H_ */
