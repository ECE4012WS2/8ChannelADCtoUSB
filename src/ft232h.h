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


/*** CS5368 register abstraction layer ***/
struct GCTL_BITS
{
    uint8_t MODE:2;         // 0:1 sample rate range
    uint8_t DIF:2;          // 2:3 data format of serial audio
    uint8_t MDIV:2;         // 4:5 either divides bt 2, both by 4
    uint8_t CLKMODE:1;      // 6   enable divide by 1.5
    uint8_t CP_EN:1;        // 7   enabled Control Port Mode
};
union GCTL_REG
{
    uint8_t all;
    struct GCTL_BITS bit;
};
struct PDN_BITS
{
    uint8_t PDN21:1;        // power down channel pair 1 and 2
    uint8_t PDN43:1;
    uint8_t PDN65:1;
    uint8_t PDN87:1;
    uint8_t PDN_OSC:1;      // disable internal oscillator core
    uint8_t PDN_BG:1;       // powers down bandgap reference
    uint8_t RSVD:2;
};
union PDN_REG
{
    uint8_t all;
    struct PDN_BITS;
};
struct CS5368_REGS
{
    uint8_t GCTL_addr;          // global control register
    uint8_t OVFM_addr;          // disable overflow interrupts
    uint8_t HPF_addr;           // disable high-pass filters
    uint8_t PDN_addr;           // power down register
    uint8_t MUTE_addr;          // mute individual channels
    uint8_t SDEN_addr;          // SDOUT enable control register
    union GCTL_REG GCTL;
    union PDN_REG PDN;
};

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
class ACBUS_out
{
  private:
    FT232H* ft;                 // associated device
    uint8_t mask_high;          // used for driving high
    uint8_t mask_low;           // used for driving low
    uint8_t value;              // save current state (for toggle)
  public:
    ACBUS_out(FT232H* ft, uint8_t index);
    /* Sets value, but does not write out to device */
    void set(uint8_t out);
    /* Writes out all CBUS state updates since last write to device */
    void write();
    /* Sets value and immediately writes out */
    void operator=(uint8_t out);
    /* Returns opposite of current value */
    uint8_t operator!() const;
};

/*** Handles SPI protocol on 3 GPIO pins ***/
class ADC_SPI
{
  private:
    FT232H* ft;                 // associated device
    ACBUS_out* CDIN;
    ACBUS_out* CCLK;
    ACBUS_out* CSn_CL;
    uint8_t CHIP_ADDRESS;       // CS5368 SPI chip address
  public:
    ADC_SPI(FT232H* ft, ACBUS_out* CDIN, ACBUS_out* CCLK, ACBUS_out* CSn_CL);

    /* Writes data to the register map (addr) */
    void write(uint8_t map, uint8_t data);
};

/*
 * Main class of abstraction of the D2XX API for supporting
 * the specific interface to the CS5368 ADC chip
 *
 */
class FT232H
{
  friend class ACBUS_out;       // allow access to status variables
  friend class ADC_SPI;

  private:
    FT_STATUS ftStatus;             // stores status on each API call
    FT_HANDLE ftHandle;             // FT232H device handle
    uint8_t CBUS_STATE;             // maintains current CBUS IO pin state
    CS5368_REGS adc_regs;

    // Buffer and variables for storing results of each read
    uint8_t RxBuffer[1024];         // buffer size on FT232H is 1k bytes
    DWORD RxBytes;
    DWORD BytesReceived;

    // Buffer containing all recently read raw data
    CircularBuffer<uint8_t> dataBuffer;

    // Buffers for formatted data for each channel
    CircularBuffer<uint32_t> channelBuffer[8];

  public:

    // FT232H GPIO output pins
    ACBUS_out SSn_RST;          // ACBUS5
    ACBUS_out CDIN;             // ACBUS6
    ACBUS_out CCLK;             // ACBUS8
    ACBUS_out CSn_CL;           // ACBUS9

    // Handles SPI protocol on 3 GPIO pins
    ADC_SPI SPI;

    FT232H();

    /* Initializes ADC with default parameters */
    void initCS5368();

/*** Simplified error wrapping functions of FT API calls ***/
    /*Tries to guess the device to open*/
    void open();
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

    /* Formats a single sample for 4 channels from the global raw data
     * buffer and stores it into its respective buffer */
    bool formatSample();

    /* Throw away all samples until the next change in LRCK */
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

    void printBuffer(uint32_t count){
        for(uint32_t i = 0; i < count; i++){
            std::cout << "0b" << std::bitset<8>(dataBuffer[i]) << std::endl;
        }
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
