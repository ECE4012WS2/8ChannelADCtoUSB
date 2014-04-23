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


//void sendOverSocket();

int main()
{
    bool first_run = false;             // get this as a cmd arg later
     uint32_t buf[2048];
     for(int i = 0; i < 2048;++i){
       buf[i] = i;
     }
   TCPSocket s("192.168.1.101",3000,true);
    header_t header;
    header.currentChannel = 0;
    header.size = 2048;
    char mydata = 0xCC;
    for(int i = 0;i < 2000;++i)
      s.send(&mydata,1);
        // s.send(buf,2048 * sizeof(uint32_t));
        // sleep(100000);
        s.close();
    exit(1);
    FT232H ft;
    // Configuration parameters
    if(first_run) ft.programEEPROM();
    ft.setChannelNum(8);
    //ft.setCrystalFreq(24576000);
    ft.setCrystalFreq(27460000);
    ft.setSocketType("TCP");
    ft.connect("192.168.1.101",3000);

    // Initialize ADC and to start sampling
    ft.init_ADC();

    // Setting sampling rate, which must be followed by buffer clear
    ft.setSamplingRate(107265);
    //ft.setSamplingRate(214530);
    // ft.setHighPassFilter(false);
    ft.clear();

    // This will buffer at least the number of samples requested into
    // channel buffers. Memory is allocated dynamically and previous
    // data in the buffers will be cleared.
    ft.buffer(5000);

/*
    // Copy samples for channel 1 to array
    int* channel1 = new int[200];
    ft.read(channel1, 200, 1);
*/
    cout << "Writing buffer to file" << endl;
    ft.writeBuf2File();

    cout << "Clearing channels" << endl;

    ft.clear();     // clear to channel buffers to free up memory

    // sendOverSocket();
    return 0;
}

//void sendOverSocket(){
//
//
//  int numFiles = 0;
//  int size;
//  int bytesRead;
//  char* buf;
//  std::stringstream strstream;
//  string filename;
//  while(++numFiles < 9){
//    TCPSocket s("127.0.0.1",0,true);
//
//    strstream << "channel" << numFiles << ".csv";
//    filename = strstream.str();
//    FILE *fp;
//    cout << "Opening: " << filename << endl;
//    fp = fopen(filename.c_str(),"r");
//    fseek(fp,0L,SEEK_END);
//    size = ftell(fp);
//    fseek(fp,0L,SEEK_SET);
//
//    buf = (char*) malloc(size);
//
//    bytesRead = fread(buf,1,size,fp);
//
//    if(bytesRead == 0) exit(1);
//    if(bytesRead <0){
//      perror("Read():");
//      exit(1);
//    }
//
//    s.send(buf,size);
//    s.close();
//    strstream.str("");
//    free(buf);
//
//
//}
//
//}
