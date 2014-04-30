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
		TCPSocket(const std::string& newHostname, const int32_t& newPort, const bool client = true);
		virtual ~TCPSocket();
		int32_t send(const void* sendbuff, size_t length);
		int32_t recv();
		void dontBlock();
		void enableBlock();
	private:
		int32_t open();
		socklen_t m_clilen;
		struct sockaddr_in m_cliaddr;
		int32_t m_listenfd;
		bool m_isClient;
		struct addrinfo *m_res;
	};

#endif /* TCPSOCKET_H_ */
