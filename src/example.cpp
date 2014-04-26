/*
 * 8 Channel Simutaneous ADC Sampling
 *
 * ECE4012: Senior Design, Group: WS2
 * Members: Yao Lu, Eric Patterson, Austin Ward, Fujun Xie, Mohan Yang
 * Feb 7, 2014
 *
 * example.cpp
 *
 * Reads in sampled data from ADC using the FT1248 interface
 * on the FT232H
 *
 */

#include "simuladc.h"
#include <unistd.h>
#include <sstream>
using namespace std;

int main()
{
    bool first_run = false;
    bool local = false;

    // Create the adc object
    SimulADC adc;

    // Program the usb chip (ft232h) if this is the first run
    if(first_run) adc.programEEPROM();

    // Configuration parameters that can be set before initialization
    adc.setChannelNum(8);
    adc.setCrystalFreq(24576000);

    // Connect to destination
    if(!local) adc.connect("127.0.0.1",3000);

    // Initialize the ADC
    adc.init_ADC();

    // Configuration parameters that must be set after initialization
    adc.setSamplingRate(96000);
    //adc.setHighPassFilter(false);             // on by default

    // Must clear buffers before reading or sending data
    adc.clear();

    // Buffer samples locally or send them to a remote address
    if(local) adc.buffer(5000);
    else      while(1) adc.sendSamples(500000);

    // Example to read samples locally
    int* channel1;
    if(local){
        channel1 = new int[200];
        adc.read(channel1, 200, 1);
    }

    return 0;
}

