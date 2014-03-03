/*
 * UDPSocket.cpp
 *
 *  Created on: Feb 26, 2014
 *      Author: austin
 */

#include "UDPSocket.h"
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <arpa/inet.h>

UDPSocket::UDPSocket(const std::string& hostname, const int32_t& port,
		const size_t& buffSize, const bool bind)
		: Socket(hostname, port, buffSize)
	{
	}

UDPSocket::~UDPSocket()
	{
	}

int32_t UDPSocket::send(const void* sendbuff, size_t length)
	{
        return 0;
	}

int32_t UDPSocket::recv()
	{
        return 0;
	}

void UDPSocket::dontBlock()
	{
	}

void UDPSocket::enableBlock()
	{
	}

int32_t UDPSocket::open()
	{
        return 0;
	}

std::ostream& UDPSocket::toString(std::ostream& o) const
	{
		return o << "UDPSocket( " << m_hostname << ":" << m_port << ", bind = " << m_bind << ")";
	}
