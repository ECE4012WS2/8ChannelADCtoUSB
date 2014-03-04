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
Socket::Socket(const std::string& hostname, const int32_t& port,
		const size_t &numberSamples = 0)
		: m_hostname(hostname), m_port(port), m_sockfd(0), m_type(-1), m_block(
				true), m_sockOpen(false)
	{
		//Resize the deque to reserve data, if we have a number
		if (numberSamples)
			{
				m_samples.resize(numberSamples);
			}

		bzero(&m_servaddr, sizeof(sockaddr_in));

	}

Socket::~Socket()
	{
	}

bool Socket::isOpen() const
	{
		return m_sockOpen;
	}

Sample Socket::getData()
	{
		Sample returnValue = m_samples[0];
		m_samples.pop_front();
		return returnValue;
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

int32_t Socket::send(const std::vector<Sample> elements){
    int32_t code = 0;
    for(size_t i = 0; i < elements.size(); ++i){
        Sample curr = elements[i];
        code +=send((void*) curr.name.c_str(),curr.name.length());
    }
    return code;
}

int32_t Socket::getSock() const
	{
		return m_sockfd;
	}

size_t Socket::size() const
	{
		return m_samples.size();
	}

void Socket::setSize(size_t size)
	{
		m_samples.resize(size);
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


