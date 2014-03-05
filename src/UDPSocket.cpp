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
		: Socket(hostname, port, buffSize), m_bind(bind)
	{
        m_servaddr.sin_family = AF_INET;
        m_servaddr.sin_addr.s_addr = inet_addr(m_hostname.c_str());
        m_servaddr.sin_port = htons(m_port);
        
        m_type = SOCK_DGRAM;
        
        m_sockfd = socket(AF_INET,m_type,0);
        
        if(m_sockfd < 0){
            perror("UDPSocket: socket()");
        }
        
        int oval = 1;
        
        //Set reuse address so you dont have to wait to bind
        
        rc = setsockopt(m_sockfd,SOL_SOCKET,SO_REUSEADDR,&oval, sizeof(int));
        
        noError("UDPSocket: setsockopt()");
        
        if(bind){
            open();
        }
        
        std::cout <<"Created new UDP Socket: " << toString() << std::endl;
        
	}

UDPSocket::~UDPSocket()
	{
	}

int32_t UDPSocket::send(const void* sendbuff, size_t length)
	{
        int numBytes = 0;
        while((size_t)numBytes < length){
        numBytes += sendto(m_sockfd, (char*)sendbuff + numBytes, length - numBytes, 0, (struct sockaddr *) &m_servaddr,sizeof(m_servaddr));
            
        }
        return 0;
	}

int32_t UDPSocket::recv()
	{
        if(!m_bind){
            std::cerr <<"Recv called on UDP socket not bound. Reconstruct with bind = true!" << std::endl;
            return -1;
        }
        //Until we figure out our sample size this will be used.
        size_t buffSize = 100;
        char buff[buffSize];
        bzero(buff, buffSize);
        
        int32_t bytesRecv = 0;
        if(m_block){
            bytesRecv = (int32_t)recvfrom(m_sockfd,(void*) buff, buffSize,0,NULL,NULL);
        }
        else{
            bytesRecv = (int32_t)recvfrom(m_sockfd,(void*)buff, buffSize,MSG_DONTWAIT,NULL,NULL);
        }
        std::cout << "Received: " << buff << std::endl;
        return bytesRecv;
	}

void UDPSocket::dontBlock()
	{
        m_block = false;
	}

void UDPSocket::enableBlock()
	{
        m_block = true;
	}

int32_t UDPSocket::open()
	{
        struct sockaddr_in bindaddr;
        bindaddr.sin_family = AF_INET;
        bindaddr.sin_addr.s_addr = inet_addr("0.0.0.0");
        bindaddr.sin_port = htons(m_port);
        
        rc = bind(m_sockfd,(struct sockaddr*) &bindaddr,sizeof(struct sockaddr_in));
        noError("UDPSocket: bind()");
        this->setOpen(true);
        
        
        return 0;
	}
