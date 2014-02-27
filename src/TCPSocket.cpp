/*
 * TCPSocket.cpp
 *
 *  Created on: Feb 26, 2014
 *      Author: austin
 */

#include "TCPSocket.h"


TCPSocket::TCPSocket(const std::string& hostname, const int32_t& port,
		const size_t& buffSize, const bool client):Socket(hostname, port, buffSize) {
}

TCPSocket::~TCPSocket() {
}

int32_t TCPSocket::send(const void* sendbuff, size_t length) {
}

int32_t TCPSocket::recv() {
}

void TCPSocket::dontBlock() {
}

void TCPSocket::enableBlock() {
}

int32_t TCPSocket::open() {
}
