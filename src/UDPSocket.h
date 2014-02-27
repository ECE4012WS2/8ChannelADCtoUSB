/*
 * UDPSocket.h
 *
 *  Created on: Feb 26, 2014
 *      Author: austin
 */

#ifndef UDPSOCKET_H_
#define UDPSOCKET_H_


#include "Socket.h"

class UDPSocket: public Socket {
public:
	UDPSocket(const std::string& hostname, const int32_t& port, const size_t& buffSize, const bool bind = true);
	virtual ~UDPSocket();
	int32_t send(const void* sendbuff, size_t length);
	int32_t recv();
	void dontBlock();
	void enableBlock();
private:
	int32_t open();
	virtual int32_t prepareData(char** data,const size_t length){return 0;}
};

#endif /* UDPSOCKET_H_ */
