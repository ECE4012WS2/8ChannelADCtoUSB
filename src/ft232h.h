/*
 * 8 Channel Simutaneous ADC Sampling
 *
 * ECE4012: Senior Design, Group: WS2
 * Members: Yao Lu
 * Feb 7, 2014
 *
 * ft232h.h
 *
 * Header file for helper objects
 *
 */

#ifndef __FT232H_H_
#define __FT232H_H_
/*** Standard library headers ***/
#include <stdlib.h>             // for exit
#include <iostream>             // std out
#include <fstream>              // file stream
#include <sstream>              // string stream
#include <stdint.h>             // int types
#include <bitset>

/*** FTDI library header ****/
#include "ftd2xx.h"

/*** Other supporting headers ***/
#include "buffer.h"             // managing buffer


/*** Global constants ***/
const uint32_t RAW_BUFFER_SIZE = 12800;
const uint32_t CHANNEL_BUFFER_SIZE = 200;

class FT232H;                   // declare class existance

/*
 * Helper class for convenient usage of output IO pins
 *
 * Usage:
 *
 *   ACBUSpin = 0;              // drive low
 *   ACBUSpin = 1;              // drive high
 *   ACBUSpin = !ACBUSpin;      // toggle
 *
 */
class ACBUS_out {
  private:
    FT232H* ft;                 // associated device
    uint8_t mask_high;          // used for driving high
    uint8_t mask_low;           // used for driving low
    uint8_t value;              // save current state
  public:
    ACBUS_out(FT232H* ft, uint8_t index);
    void operator=(uint8_t out);
    uint8_t operator!() const;
};

/*
 * Main class of abstraction of the D2XX API for supporting
 * the specific interface to the CS5368 ADC chip
 *
 */
class FT232H {
  friend class ACBUS_out;           // allow to status variables

  private:
    FT_STATUS ftStatus;             // stores status on each API call
    FT_HANDLE ftHandle;             // FT232H device handle
    uint8_t CBUS_STATE;             // maintains current CBUS IO pin state

    // Buffer and variables for storing results of each read
    uint8_t RxBuffer[1024];         // buffer size on FT232H is 1k bytes
    DWORD RxBytes;
    DWORD BytesReceived;

    // Buffer containing all recently read raw data
    CircularBuffer<uint8_t> dataBuffer;

    // Buffers for formatted data for each channel
    CircularBuffer<uint32_t> channelBuffer[8];

  public:

/*** CBUS bit-bang IO mode pins ***/

    // FT1248 active low slave select, also tied to led1 on UM232H (ACBUS8)
    ACBUS_out SSn_led1;
    // D flip-flop active low clear, also tied to led2 on UM232H (ACBUS9)
    ACBUS_out CL_led2;
    // Clock, 7.5Mhz
    ACBUS_out CLK7_5;
    // CS5368 active low reset (ACBUS5)
    ACBUS_out CS5368_reset;
    // Remaining IO pin, unused for now (ACBUS6)
    //ACBUS_out gpio;

    FT232H();

/*** Simplified error wrapping functions of FT API calls ***/

    /* Opens the device at the given port */
    void open(uint16_t port);
    /* Reset the device */
    void reset();
    /* Purge transmit and receive buffers */
    void purge();
    /* Close the handle on the device */
    void close();

/*** Functions managing receive buffer ***/

    /* Reads in available data and adds it to the dataBuffer */
    void read();

    /* Reads in the requested amount of data within the time frame,
     * or else times out */
    DWORD blockingRead(DWORD bytes, DWORD timeout);

    /// NOT TESTED ///
    /* Formats a single sample for 4 channels from the global raw data
     * buffer and stores it into its respective buffer */
    bool formatSample();

    void alignToNextLRCK(uint8_t LRCK);

/*** Supporting functions ***/

    /*
     * Called to program desired values into the EEPROM,
     * should be called only once on the first run for each device.
     *
     * FT1248 mode and settings is set along with ACBUS pins.
     *
     */
    void programEEPROM();

    /* 
     * Used by member functions to check error.
     * Error type and passed in string is printed out and
     * program terminates.
     *
     */
    void errCheck(std::string errString);

/*** Debugging functions ***/

    uint8_t flip( uint8_t n );
    void printBuffer(uint32_t count){
        for(uint32_t i = 0; i < count; i++){
            std::cout << "0b" << std::bitset<8>(dataBuffer[i]) << std::endl;
        }
        /*
        for(uint32_t i = 0; i < count; i++){
            std::cout << std::hex << "Channel 1: " << (unsigned int) channelBuffer[0][i] << std::endl;
            std::cout << std::hex << "Channel 2: " << (unsigned int) channelBuffer[1][i] << std::endl;
            std::cout << std::hex << "Channel 3: " << (unsigned int) channelBuffer[2][i] << std::endl;
            std::cout << std::hex << "Channel 4: " << (unsigned int) channelBuffer[3][i] << std::endl;
            std::cout << std::hex << "Channel 5: " << (unsigned int) channelBuffer[4][i] << std::endl;
            std::cout << std::hex << "Channel 6: " << (unsigned int) channelBuffer[5][i] << std::endl;
            std::cout << std::hex << "Channel 7: " << (unsigned int) channelBuffer[6][i] << std::endl;
            std::cout << std::hex << "Channel 8: " << (unsigned int) channelBuffer[7][i] << std::endl;
        }
        */
    }

    void printChannels(){
        uint32_t entry;
        for(int i = 0; i < channelBuffer[0].getEntries(); i++){
            channelBuffer[0].pop(&entry);
            std::cout << "Channel1: " << entry << std::endl;
        }
        exit(0);
    }

    /* Writes each of the channel buffers out to a separate
     * file in csv format
     */
    void writeBuf2File(){
        std::ofstream file;
        uint32_t entry;
        uint32_t n;
        for(int i = 0; i < 8; i++){
            std::stringstream filename;
            filename << "channel" << (i+1) << ".csv";
            file.open(filename.str().c_str());
            channelBuffer[i].reset();
            n = 0;
            while(channelBuffer[i].getNext(entry)){
                file << n << "," << entry << std::endl;
                n++;
            }
            file.close();
        }
    }
};

#endif
