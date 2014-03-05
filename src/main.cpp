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
#include "TCPSocket.h"
#include "UDPSocket.h"

using namespace std;

void testSockets(){
    
    std::string hostname("127.0.0.1");
    UDPSocket s(hostname,8080,4,true);
    
    std::cout << "Sending" << std::endl;
    
    char send[] = "Hello!\n";
    std::cout << "char buffer: " << send << std::endl;
    s.send( send, sizeof(send));
    
    

    while(true){
//    std::cin >> mybuff;
//        size_t length = strlen(mybuff);
//        s.send(mybuff,length);
//        bzero(mybuff,sizeof(mybuff));
        s.recv();
    }
    
    getchar();
    
}


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

    // Read in 20,000 bytes of data
    // buffer (which will overflow)
    cout << "Reading data from FT232H... ";
    for(int i = 0; i < 20; i++){
        ft.blockingRead(1000, 5000);
    }
    cout << "Done." << endl;

    // Format 200 times (so there will be ~100 samples in each channel)
    for(int i = 0; i < 200; i++){
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

