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
    ft.SSn_led1 = 1;                    // not selecting FT1248
    ft.CL_led2 = 0;                     // clear flip flop
    ft.CS5368_reset = 0;                // hold ADC in reset

    sleep(1);

    cout << "Starting CS5368." << endl;
    ft.CS5368_reset = 1;                // release reset

    sleep(1);

    cout << "Enabling FT1248 interface." << endl;
    ft.SSn_led1 = 0;                    // enable FT1248

    sleep(1);

    // FT1248 should be seeings all zeros at this point, which sets
    // it up for 8-bit mode and storing received data into buffer

    cout << "Enabling flip flop to clock data from ADC." << endl;
    ft.CL_led2 = 1;                     // release clear on flip flop

    cout << "Clearing buffers on FT232H... ";
    ft.purge();                         // clear buffers
    cout << "Cleared." << endl << endl;

    // Read in 20,000 bytes of data and format into the channel
    // buffer (which will overflow)
    cout << "Reading data from FT232H... ";
        ft.blockingRead(300, 5000);
//        ft.alignToNextLSR(2);
//    for(int i = 0; i < 20; i++){
//        ft.blockingRead(100, 5000);
//        while(ft.formatSample()){}
//    }
    cout << "Done." << endl;
    
    // Write contents of buffer out to files for easy graphing
    //ft.writeBuf2File();

    //cout << "Buffer contains:" << endl;
    ft.printBuffer(300);    

    ft.close();
    return 0;
}

