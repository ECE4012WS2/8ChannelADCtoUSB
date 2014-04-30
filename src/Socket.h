// The MIT License (MIT)
//
// Copyright (c) 2014 Austin Ward, Yao Lu, Fujun Xie, Eric Patterson, Mohan Yang
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


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
#include <deque>
#include <vector>

class Socket
	{
	public:
		Socket(const std::string& hostname, const int32_t& port);
		virtual ~Socket();
		virtual int32_t send(const void* sendbuff, size_t length) = 0;
        virtual int32_t close();
		virtual int32_t recv() = 0;
		bool isOpen() const;
		int32_t getSock() const;
		size_t size() const;
		void setSize(size_t size);
		int32_t getPort() const;
		std::string getHostname() const;
		virtual void dontBlock() = 0;
		virtual void enableBlock() = 0;
		bool isBlocking() const;
        virtual std::string toString() const;
	protected:
		std::string m_hostname;
		int32_t m_port;
		int32_t m_sockfd;
		struct sockaddr_in m_servaddr;
		int m_type;
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


#endif /* SOCKET_H_ */
