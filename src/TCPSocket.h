/*
 * TCPSocket.h
 *
 *  Created on: Feb 26, 2014
 *      Author: austin
 */

#ifndef TCPSOCKET_H_
#define TCPSOCKET_H_

#include "Socket.h"

class TCPSocket: public Socket {
public:
	TCPSocket(const std::string& newHostname, const int32_t& newPort, const size_t& buffSize, const bool client = true);
	virtual ~TCPSocket();
	int32_t send(const void* sendbuff,size_t length);
	int32_t recv();
	void dontBlock();
	void enableBlock();
private:
	virtual int32_t prepareData(char** data,const size_t length){return 0;}
	int32_t open();
	socklen_t clilen;
	struct sockaddr_in cliaddr;
	int32_t listenfd;
	bool isCLient;
	struct addrinfo_t *res;
};

#endif /* TCPSOCKET_H_ */
