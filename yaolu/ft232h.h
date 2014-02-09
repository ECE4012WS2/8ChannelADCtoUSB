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

#include <stdlib.h>
#include <iostream>
#include <string>
#include <stdint.h>
//#include <cstdint>
#include "ftd2xx.h"

class FT232H;

const uint32_t BUFFER_SIZE = 2048;

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

    // Buffer and variables for each read
    uint8_t RxBuffer[1024];
    DWORD RxBytes;
    DWORD BytesReceived;

    // Global buffer containing recently received data and accomplaining
    // variables to maintain it as a circular buffer
    uint8_t dataBuffer[BUFFER_SIZE];
    uint32_t head;
    uint32_t entries;

  public:

/*** CBUS bit-bang IO mode pins ***/

    // FT1248 active low slave select, also tied to led1 on UM232H (ACBUS8)
    ACBUS_out SSn_led1;
    // D flip-flop active low clear, also tied to led2 on UM232H (ACBUS9)
    ACBUS_out CL_led2;
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

    /* Reads in available data and adds it to the buffer */
    void read();
    /* Get the current number of entries in buffer */
    int getEntries(){ return entries; }
    /* Clear buffer */
    void clearBuffer(){ head=entries=0; }

/*** Supporting functions ***/

    /* Adds received data into the buffer */
    void saveToBuffer(DWORD bytes);

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
        if(count > BUFFER_SIZE) count = BUFFER_SIZE;
        for(uint32_t i = 0; i < count; i++){
            std::cout << std::hex << (unsigned int) flip(dataBuffer[i]) << std::endl;
        }
    }
};

