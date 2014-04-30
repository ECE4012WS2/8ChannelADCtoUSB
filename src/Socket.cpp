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
 * Socket.cpp
 *
 *  Created on: Feb 26, 2014
 *      Author: austin
 */

#include "Socket.h"
#include <cstring>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
Socket::Socket(const std::string& hostname, const int32_t& port)
		: m_hostname(hostname), m_port(port), m_sockfd(0), m_type(-1), m_block(
				true), m_sockOpen(false)
	{
		bzero(&m_servaddr, sizeof(sockaddr_in));

	}

Socket::~Socket()
	{
	}

bool Socket::isOpen() const
	{
		return m_sockOpen;
	}

int32_t Socket::close(){

    if(isOpen()){
        std::cout << "Closing socket: " << toString() << std::endl;
        this->setOpen(false);
        return ::close(m_sockfd);
    }
    std::cerr <<"Close called on already closed socket! " << toString() << std::endl;
    return -1;
}


int32_t Socket::getSock() const
	{
		return m_sockfd;
	}



int32_t Socket::getPort() const
	{
		return m_port;
	}

std::string Socket::getHostname() const
	{
		return m_hostname;
	}

bool Socket::isBlocking() const
	{
		return m_block;
	}

void Socket::setOpen(bool value)
	{
		m_sockOpen = value;
	}
bool Socket::noError(const std::string& msg) const
	{
		if (rc != 0)
			{
				perror(msg.c_str());
				return false;
			}
		return true;
	}

bool Socket::noError(const char* msg) const
	{
		if (rc != 0)
			{
				perror(msg);
				return false;
			}
		return true;
	}

bool Socket::noError() const
	{
		if (rc != 0)
			{
				perror("Error in unknown function");
				return false;
			}
		return true;
	}

bool Socket::noError(const std::string& msg, int code) const
	{
		if (code != 0)
			{
				perror(msg.c_str());
				return false;
			}
		return true;
	}

bool Socket::noError(const char* msg, int code) const
	{
		if (code != 0)
			{
				perror(msg);
				return false;
			}
		return true;
	}

bool Socket::noError(int code) const
	{
		if (code != 0)
			{
				perror("Error in unknown function");
				return false;
			}
		return true;
	}

std::string Socket::toString() const{

    std::stringstream s;

    s << m_hostname << ":" << m_port;

    return s.str();

}
