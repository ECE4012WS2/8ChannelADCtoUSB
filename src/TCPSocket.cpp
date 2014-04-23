/*
 * TCPSocket.cpp
 *
 *  Created on: Feb 26, 2014
 *      Author: austin
 */

#include "TCPSocket.h"
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <netdb.h>
#include <cstdlib>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

TCPSocket::TCPSocket(const std::string& hostname, const int32_t& port, const bool client)
: Socket(hostname, port),m_clilen(-1),m_listenfd(-1), m_isClient(client)
{

    m_type = SOCK_STREAM;
    bzero(&m_cliaddr, sizeof(sockaddr_in));

    //Set up the server address
    m_servaddr.sin_family = AF_INET;
    m_servaddr.sin_addr.s_addr = INADDR_ANY;
    m_servaddr.sin_port = htons(m_port);

    struct addrinfo hints;
    char portstr[5];
    snprintf(portstr, 5, "%d", m_port);

    bzero(&hints, sizeof(struct addrinfo));

    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = PF_UNSPEC;
    rc = getaddrinfo(m_hostname.c_str(), portstr, &hints, &m_res);
    noError("TCPSocket: getaddrinfo");

    m_sockfd = socket(m_res->ai_family, m_res->ai_socktype,
                      m_res->ai_protocol);
    noError("TCPSocket: socket()");

    int optval = 1;
    rc = setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &optval,
                    sizeof(optval));
    noError("TCPSocket: setsockopt()");


    this->open();
    this->enableBlock();
    std::cout << "Created new TCPSocket " << m_hostname << ":" << m_port << std::endl;

}

TCPSocket::~TCPSocket()
{
}

int32_t TCPSocket::send(const void* sendbuff, size_t length)
{
    if (this->isOpen())
    {
        ssize_t sent = 0;
        ssize_t bytesSent = 0;
        while (sent < (ssize_t)length)
        {
            bytesSent = ::send(m_sockfd,
                               (char*) sendbuff + sent,
                               length - sent, 0);

            if(bytesSent < 0){
                perror("TCPSocket: Send");
                return (int32_t) bytesSent;
            }
            sent += bytesSent;
        }
        return (int32_t)sent;
    }

    std::cerr << "Socket not opened! " << toString() << std::endl;
    return -1;
}

int32_t TCPSocket::recv()
{
    if (this->isOpen())
    {

        fd_set recv_sock;
        struct timeval timeout;

        FD_ZERO(&recv_sock);
        FD_SET(m_sockfd, &recv_sock);
        if (!m_block)
        {
            timeout.tv_sec = 0;
            timeout.tv_usec = 1000;
            //Call select with the timeout
            std::cout <<"Calling \"nonblocking\" select" << std::endl;
            rc = select(m_sockfd + 1, &recv_sock, NULL, NULL, &timeout);
        }
        else
        {
            //infinite timeout
            std::cout <<"Calling \"blocking\" select" << std::endl;
            rc = select(m_sockfd +1 , &recv_sock, NULL, NULL, NULL);
        }

//        noError("TCPSocket: select()");

        //Check that we received data
        if (rc > 0)
        {
            size_t buffSize = 100;
            char buff[buffSize];
            bzero(buff, buffSize);
            if (!m_block)
            {
                rc = (int)::recv(m_sockfd, buff, buffSize, MSG_WAITALL & MSG_DONTWAIT);
            }
            else
            {
                rc = (int) ::recv(m_sockfd, buff, buffSize, 0);
            }

            //Specific error handling so we cant use noErrors()
            if (rc == 0)
            {
                ::close(m_sockfd);
                this->setOpen(false);
            }
            else if (rc < 0)
            {
                switch (errno)
                {
                    case (EAGAIN): //We need to run it again
                        break;
                    default:
                        perror("TCPSocket: recv");
                        break;
                }

            }
            std::cout << "Recieved: " << buff << std::endl;
            return (int32_t)rc;
        }

    }
    return -1;
}

void TCPSocket::dontBlock()
{
    m_block = false;
    int flags;
    flags = fcntl(m_sockfd, F_GETFL, 0);
    if (!noError("TCPSocket: fcntl: F_GETFL", flags)) return;

    rc = fcntl(m_sockfd, F_SETFL, flags | O_NONBLOCK);
    if (!noError("TCPSocket: fcntl: F_SETFL")) return;

}

void TCPSocket::enableBlock()
{
    m_block = true;
    int orig_flags;
    orig_flags = fcntl(m_sockfd, F_GETFL, 0);
    if (!noError("TCPSocket: fcntl: F_GETFL", orig_flags)) return;

    rc = fcntl(m_sockfd, F_SETFL, orig_flags | !O_NONBLOCK);
    if (!noError("TCPSocket: fcntl: F_SETFL")) return;
}

int32_t TCPSocket::open()
{
    int numTries = 0;
    if (m_isClient)
    {
        while(true){
            std::cout << "Trying to connect on " << toString() ;
            rc = connect(m_sockfd, m_res->ai_addr, m_res->ai_addrlen);
            std::cout << "  Returned " << rc << std::endl;
            if (rc != 0)
            {
                std::cout << "Waiting for a listener at: " <<  toString() << std::endl;
                if (++numTries > 10)
                {
                    std::cerr << "Could not connect " << toString() << " after 10 tries, exiting " << std::endl;
                    exit((int32_t)rc);
                }
                sleep(1);
            }else
            {
                freeaddrinfo(m_res);
                this->setOpen(true);
                return (int32_t)rc;
            }
        }
    }
    else
    {
        rc = bind(m_sockfd, m_res->ai_addr, m_res->ai_addrlen);
        noError("TCPSocket: Bind()");

        rc = listen(m_sockfd, 5);
        noError("TCPSocket: listen()");

        m_clilen = sizeof(m_cliaddr);

        fd_set socket_set;
        fd_set working_set;

        FD_ZERO(&socket_set);
        FD_SET(m_sockfd, &socket_set);

        timeval origtime;
        timeval modtime;
        bzero(&origtime, sizeof(timeval));
        bzero(&modtime, sizeof(timeval));
        origtime.tv_sec = 1;

        unsigned int numTimeouts = 0;
        while (1)
        {
            modtime.tv_sec = origtime.tv_sec; //reset the timer

            memcpy(&working_set, &socket_set, sizeof(fd_set)); //copy the socket set

            rc = select(m_sockfd + 1, &working_set, NULL, NULL,
                        &modtime);
            noError("TCPSocket: select()");
            if (rc <= 0)
            {
                std::cout
                << "Waiting for a client to connect on: "
                << toString() << std::endl;
                if (++numTimeouts >= 10)
                {
                    std::cerr
                    << "Client took too long to connect, exiting"
                    << std::endl;
                    return -1;
                }
            }
            else
            {
                m_listenfd = m_sockfd;
                m_sockfd = accept(m_listenfd,
                                  (struct sockaddr*) &m_cliaddr,
                                  &m_clilen);
                noError("TCPSocket: accept", m_sockfd);
                break;
            }
        }
        this->dontBlock();
    }
    freeaddrinfo(m_res);
    this->setOpen(true);

    return (int)rc;
}
