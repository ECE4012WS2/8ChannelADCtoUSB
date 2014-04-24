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
#include <sstream>
using namespace std;

int main()
{
    bool first_run = false;             // get this as a cmd arg later
    bool local = false;
    // testSocket();
    FT232H ft;
    // Configuration parameters
    if(first_run) ft.programEEPROM();
    ft.setChannelNum(8);
    ft.setCrystalFreq(24576000);
    //ft.setCrystalFreq(27460000);
    //ft.setSocketType("TCP");
    

    //if(!local) ft.connect("192.168.1.101",3000);
    if(!local) ft.connect("127.0.0.1",3000);

    // Initialize ADC and to start sampling
    ft.init_ADC();

    // Setting sampling rate, which must be followed by buffer clear
    ft.setSamplingRate(96000);
    //ft.setSamplingRate(214530);
    //ft.setHighPassFilter(false);
    ft.clear();

    // This will buffer at least the number of samples requested into
    // channel buffers. Memory is allocated dynamically and previous
    // data in the buffers will be cleared.
    if(local) ft.buffer(5000);
    else      ft.sendSamples(50000000);

/*
    // Copy samples for channel 1 to array
    int* channel1 = new int[200];
    ft.read(channel1, 200, 1);
*/
    if(local){
        cout << "Writing buffer to file" << endl;
        ft.writeBuf2File();
    }

    cout << "Clearing channels" << endl;

    ft.clear();     // clear to channel buffers to free up memory

    return 0;
}

