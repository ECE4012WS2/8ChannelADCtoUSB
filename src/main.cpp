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
    bool first_run = false;             // get this as a cmd arg later
    bool local = false;
    SimulADC adc;
    // Configuration parameters
    if(first_run) adc.programEEPROM();
    adc.setChannelNum(8);
    adc.setCrystalFreq(24576000);

    //if(!local) adc.connect("192.168.1.101",3000);
    if(!local) adc.connect("127.0.0.1",3000);

    // Initialize ADC and to start sampling
    adc.init_ADC();

    // Setting sampling rate, which must be followed by buffer clear
    adc.setSamplingRate(96000);
    //adc.setHighPassFilter(false);
    adc.clear();

    // This will buffer at least the number of samples requested into
    // channel buffers. Memory is allocated dynamically and previous
    // data in the buffers will be cleared.
    if(local) adc.buffer(5000);
    else      while(1) adc.sendSamples(500000);

/*
    // Copy samples for channel 1 to array
    int* channel1 = new int[200];
    adc.read(channel1, 200, 1);
*/

    cout << "Clearing channels" << endl;
    adc.clear();     // clear to channel buffers to free up memory

    return 0;
}

