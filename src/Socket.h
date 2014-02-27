/*
 * Socket.h
 *
 *  Created on: Feb 26, 2014
 *      Author: austin
 */

#ifndef SOCKET_H_
#define SOCKET_H_

#include <netinet/in.h>
#include <sys/socket.h>
#include <string>
#include "Sample.h"
#include <deque>

class Socket {
public:
	Socket(const std::string& hostname,const int32_t& newPort, const size_t& buffSize);
	virtual ~Socket();
	virtual int32_t send(const void* sendbuff,size_t length) = 0;
	virtual int32_t recv() = 0;
	bool isOpen() const;
	char* getData(bool prepare = true);
	int32_t getSock() const;
	size_t size() const;
	void setSize(size_t size);
	int32_t getPort() const;
	std::string getHostname() const;
	virtual void dontBlock() = 0;
	virtual void enableBlock() = 0;
	bool isBlocking() const;

protected:
	std::string hostname;
	int32_t port;
	int32_t sockfd;
	struct sockaddr_in servaddr;
	size_t buffSize;
	int type;
	std::deque<Sample> samples;
	bool block;
	void setOpen(bool value);
	virtual int32_t prepareData(char** data,const size_t length) = 0;
	virtual int32_t open() = 0;
private:
	bool sockOpen;
};

#endif /* SOCKET_H_ */
