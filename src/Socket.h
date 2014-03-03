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
#include <vector>

class Socket
	{
	public:
		Socket(const std::string& hostname, const int32_t& port,
				const size_t &numberSamples);
		virtual ~Socket();
		virtual int32_t send(const void* sendbuff, size_t length) = 0;
		int32_t send(const std::vector<Sample> elements);
		virtual int32_t recv() = 0;
		bool isOpen() const;
		Sample getData();
		int32_t getSock() const;
		size_t size() const;
		void setSize(size_t size);
		int32_t getPort() const;
		std::string getHostname() const;
		virtual void dontBlock() = 0;
		virtual void enableBlock() = 0;
		bool isBlocking() const;
		virtual std::ostream& toString(std::ostream& o) const;
        std::string toString() const;
	protected:
		std::string m_hostname;
		int32_t m_port;
		int32_t m_sockfd;
		struct sockaddr_in m_servaddr;
		int m_type;
		std::deque<Sample> m_samples;
		bool m_block;
		void setOpen(bool value);
		virtual int32_t open() = 0;
		ssize_t rc;
		bool noError(const std::string& msg, int code) const;
		bool noError(const char* msg, int code) const;
		bool noError(int code) const;
		bool noError(const std::string& msg) const;
		bool noError(const char* msg) const;
		bool noError() const;
	private:
		bool m_sockOpen;

	};

std::ostream& operator<<(std::ostream& o, const Socket& s);

#endif /* SOCKET_H_ */
