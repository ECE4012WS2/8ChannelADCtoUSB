/*
 * 8 Channel Simutaneous ADC Sampling
 *
 * ECE4012: Senior Design, Group: WS2
 * Members: Yao Lu
 * Feb 7, 2014
 *
 * main.cpp
 *
 * Reads in sampled data from ADC using the FT1248 interface
 * on the FT232H
 *
 */

#include "ft232h.h"
#include <unistd.h>

using namespace std;

int main()
{
    bool first_run = false;             // get this as a cmd arg later
    FT232H ft;

    // Configuration parameters
    if(first_run) ft.programEEPROM();
    ft.setChannelNum(8);
    ft.setCrystalFreq(27000000);
    ft.setSocketType("TCP");

    // Initialize ADC and to start sampling
    ft.init_ADC();

    // Setting sampling rate, which must be followed by buffer clear
    ft.setSamplingRate(210000);
    ft.clear();
    
    // Read 200 samples into buffer
    ft.buffer(200);

    // Copy samples for channel 1 to array
    int* channel1 = new int[200];
    ft.read(channel1, 200, 1);

    return 0;
}

