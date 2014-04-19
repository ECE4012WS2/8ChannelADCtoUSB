/*
 * Sample.cpp
 *
 *  Created on: Feb 26, 2014
 *      Author: austin
 */

#include "Sample.h"
#include <iostream>             // std out
Sample::Sample() {
//		std::cout <<"Creating sample object" << std::endl;
    name = std::string("Sample");
    
}

Sample::~Sample() {
		std::cout <<"Destroying sample object" << std::endl;
}





