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
    //ft.setCrystalFreq(24576000);
    ft.setCrystalFreq(27460000);
    ft.setSocketType("TCP");

    // Initialize ADC and to start sampling
    ft.init_ADC();

    // Setting sampling rate, which must be followed by buffer clear
    //ft.setSamplingRate(96000);
    ft.clear();
    
    // Read 200 samples into buffer
    //ft.buffer(1000);

    
    int buff[1000];
    for(int i = 0; i < 100; i++){
        ft.buffer(1000);
        for(int j = 1; j <= 8; j++){
            ft.read(buff, 1000, j);
        }
    }
    
    //for(int i = 0; i < 1000; i++) cout << buff[i] << endl;
        ft.buffer(1000);

/*
    // Copy samples for channel 1 to array
    int* channel1 = new int[200];
    ft.read(channel1, 200, 1);
*/
    ft.writeBuf2File();

    return 0;
}

