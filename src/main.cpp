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

    cout << "Initializing... ";
    FT232H ft;
    ft.open(0);
    ft.reset();
    cout << "Done." << endl;

    if(first_run){
        cout << "Programming EEPROM... ";
        ft.programEEPROM();
        cout << "Done." << endl;
    }

    cout << "Setting up." << endl;
    ft.initCS5368();

    sleep(1);

    // Clear buffers before reading anything
    cout << "Clearing buffers on FT232H... ";
    ft.purge();                         // clear buffers
    cout << "Cleared." << endl << endl;

    // Read in 20,000 bytes of data
    // buffer (which will overflow)
    cout << "Reading data from FT232H... ";
    for(int i = 0; i < 20; i++){
        ft.blockingRead(1000, 5000);
    }
    cout << "Done." << endl;

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

