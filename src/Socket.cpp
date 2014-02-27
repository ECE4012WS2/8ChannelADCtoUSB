/*
 * Socket.cpp
 *
 *  Created on: Feb 26, 2014
 *      Author: austin
 */

#include "Socket.h"


Socket::Socket(const std::string& hostname, const int32_t& newPort,
		const size_t& buffSize) {
}

Socket::~Socket() {
}

bool Socket::isOpen() const {
}

char* Socket::getData(bool prepare) {
}

int32_t Socket::getSock() const {
}

size_t Socket::size() const {
}

void Socket::setSize(size_t size) {
}

int32_t Socket::getPort() const {
}

std::string Socket::getHostname() const {
}

bool Socket::isBlocking() const {
}

void Socket::setOpen(bool value) {
}
