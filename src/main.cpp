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


void sendOverSocket();

int main()
{
  sendOverSocket();
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
    //ft.setHighPassFilter(false);
    ft.clear();

    ft.buffer(5000);

/*
    // Copy samples for channel 1 to array
    int* channel1 = new int[200];
    ft.read(channel1, 200, 1);
*/
    ft.writeBuf2File();
    sendOverSocket();
    return 0;
}

void sendOverSocket(){


  int numFiles = 0;
  int size;
  int bytesRead;
  char* buf;
  std::stringstream strstream;
  string filename;
  while(++numFiles < 9){
    TCPSocket s("192.168.1.6",3000,0,true);

    strstream << "channel" << numFiles << ".csv";
    filename = strstream.str();
    FILE *fp;
    cout << "Opening: " << filename << endl;
    fp = fopen(filename.c_str(),"r");
    fseek(fp,0L,SEEK_END);
    size = ftell(fp);
    fseek(fp,0L,SEEK_SET);

    buf = (char*) malloc(size);

    bytesRead = fread(buf,1,size,fp);

    if(bytesRead == 0) exit(1);
    if(bytesRead <0){
      perror("Read():");
      exit(1);
    }

    s.send(buf,size);
    s.close();
    strstream.str("");
    free(buf);


}

}
