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
    bool first_run = true;             // get this as a cmd arg later
    FT232H ft;

    // Configuration parameters
    if(first_run) ft.programEEPROM();
    ft.setChannelNum(8);
    ft.setCrystalFreq(24576000);
    ft.setSocketType("TCP");

    // Initialize ADC and to start sampling
    ft.init_ADC();

    // Setting sampling rate, which must be followed by buffer clear
    //ft.setSamplingRate(96000);
    ft.clear();
    
    // Read 200 samples into buffer
    while(true){
    ft.buffer(6000);
        ft.send(6000);
    }

/*
    // Copy samples for channel 1 to array
    int* channel1 = new int[200];
    ft.read(channel1, 200, 1);
*/
    ft.writeBuf2File();

    return 0;
}

