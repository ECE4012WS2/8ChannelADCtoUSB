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
    cout << "Starting" << endl;
    bool first_run = false;             // get this as a cmd arg later

    FT232H ft;
    ft.open(0);
    ft.reset();

    if(first_run) ft.programEEPROM();

    ft.initCS5368();

    sleep(1);

    // Clear buffers before reading anything
    ft.purge();                         // clear buffers

    // Read in 20,000 bytes of data
    // buffer (which will overflow)
    for(int i = 0; i < 20; i++){
        ft.blockingRead(1000, 5000);
    }

    // Format 400 times (so there will be ~200 samples in each channel)
    for(int i = 0; i < 400; i++){
        ft.formatSample();
    }
    
    // Write contents of buffer out to files for easy graphing
    ft.writeBuf2File();

    //cout << "Buffer contains:" << endl;
    //ft.printBuffer(2048);    

    ft.close();

    cout << "Exiting Application" << endl;
    return 0;
}

