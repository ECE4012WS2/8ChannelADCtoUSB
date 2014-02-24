/*
 * 8 Channel Simutaneous ADC Sampling
 *
 * ECE4012: Senior Design, Group: WS2
 * Members: Yao Lu
 * Feb 7, 2014
 *
 * ft232h.cpp
 *
 * Function definitions
 *
 */

#include "ft232h.h"

using namespace std;

/* Table and function used to flip a byte, defined for convenient
 * debugging purposes */
uint8_t lut[16] = {
    0x0, 0x8, 0x4, 0xC,
    0x2, 0xA, 0x6, 0xE,
    0x1, 0x9, 0x5, 0xD,
    0x3, 0xB, 0x7, 0xF };
uint8_t FT232H::flip( uint8_t n )
{
   return (lut[n&0x0F] << 4) | lut[n>>4];
}

/* Constructor, initializes ACBUS pins and initial variables */
FT232H::FT232H()
  : ftStatus(0), ftHandle(0), RxBytes(0),BytesReceived(0), SSn_led1(this, 2), CL_led2(this, 3), CLK7_5(this, 1), CS5368_reset(this, 0)
{
    /*
     * Upper nibble of CBUS_STATE defines direction.
     * Lower nibble defines the output with this mapping:
     *      bit 3: ACBUS 9 (flip flop clear)
     *      bit 2: ACBUS 8 (FT1248 slave select)
     *      bit 1: ACBUS 6 (7.5 Mhz clock)
     *      bit 0: ACBUS 5 (CS5368 reset)
     *
     */
    CBUS_STATE = 0xF0;          // all pins are outputs, default low

    // Initialize buffers
    for(uint32_t i = 0; i < 1024; i++) RxBuffer[i] = 0;
    dataBuffer.setSize(RAW_BUFFER_SIZE);
    for(uint32_t i = 0; i < 8; i ++){
        channelBuffer[i].setSize(CHANNEL_BUFFER_SIZE);
    }
}

void FT232H::open(uint16_t port){
    ftStatus = FT_Open(port, &ftHandle);
    errCheck("FT_Open failed");
}

void FT232H::reset(){
    ftStatus = FT_SetBitMode(ftHandle, 0x00, FT_BITMODE_RESET);
    errCheck("Reset failed");

    // Sets a read timeout of 5 sec and write timeout of 1 sec
    ftStatus = FT_SetTimeouts(ftHandle, 5000, 1000);
    errCheck("FT_SetTimeouts");
}

void FT232H::purge(){

    // Clear both receive and send buffers on FT232H
    ftStatus = FT_Purge(ftHandle, FT_PURGE_RX | FT_PURGE_TX);
    errCheck("FT_Purge failed");
}

void FT232H::close(){
    ftStatus = FT_Close(ftHandle);
    errCheck("FT_Close failed");
}

void FT232H::read(){

    // Get the number bytes currently in FT232H's buffer
    ftStatus = FT_GetQueueStatus(ftHandle, &RxBytes);
    errCheck("FT_GetQueueStatus failed");

    if(RxBytes > 0){

        // Read in the current amount. This function should not block
        // because it is requests the number of bytes from FT_GetQueueStatus
        ftStatus = FT_Read(ftHandle, RxBuffer, RxBytes, &BytesReceived);
        errCheck("FT_Read failed");

        if(RxBytes != BytesReceived){
            cout << "Bytes read mis-match" << endl;
            exit(0);
        }
    }

    dataBuffer.addN(RxBuffer, (uint32_t) BytesReceived);
}

DWORD FT232H::blockingRead(DWORD bytes, DWORD timeout){
    ftStatus = FT_SetTimeouts(ftHandle, timeout, 1000);
    errCheck("FT_SetTimeouts");
    ftStatus = FT_Read(ftHandle, RxBuffer, bytes, &BytesReceived);
    errCheck("FT_Read");
    if(BytesReceived < bytes){
        cout << "Timed out in blockingRead" << endl;
        exit(0);
    }

/******************************************************************************************
 * MSIO0 is MSB apparently, temporary flip in software, re-wire hardware in final design
 ******************************************************************************************/
for(DWORD i = 0; i < BytesReceived; i++){
    RxBuffer[i] = flip(RxBuffer[i]);
}
/******************************************************************************************/

    dataBuffer.addN(RxBuffer, (uint32_t) BytesReceived);
    return BytesReceived;
}

bool FT232H::formatSample(){
    if(dataBuffer.getEntries() < 100) return false;

    uint8_t entry;
    // Peek into first entry for the LRCK bit, which is high in LJ mode
    // when the channel is even
    dataBuffer.getN(&entry, 1);
    uint8_t LRCK = entry & 1;

    // Add a new entry for the even or odd channels, depending on LRCK 
    uint32_t i0 = channelBuffer[1-LRCK].add(0);
    uint32_t i1 = channelBuffer[3-LRCK].add(0);
    uint32_t i2 = channelBuffer[5-LRCK].add(0);
    uint32_t i3 = channelBuffer[7-LRCK].add(0);

    // For each of the next 24 bits, store the bit into the correct
    // position of each channel sample buffer
    for(int i = 0; i < 24; i++){
        dataBuffer.getN(&entry, 1);                 // Get one entry
        if(LRCK != (entry&1)) return true;
        channelBuffer[1-LRCK][i0] |= ((uint32_t) ((entry&2)>>1) << (23-i));
        channelBuffer[3-LRCK][i1] |= ((uint32_t) ((entry&4)>>2) << (23-i));
        channelBuffer[5-LRCK][i2] |= ((uint32_t) ((entry&8)>>3) << (23-i));
        channelBuffer[7-LRCK][i3] |= ((uint32_t) ((entry&16)>>4) << (23-i));
        dataBuffer.clearN(1);                       // Remove one entry
    }

    // Skip extra bits to the next LRCK
    alignToNextLRCK(LRCK);

    return true;
}

/* Aligns buffer to the next set of channels different from the given LRCK.
 * If no valid LRCK (0 or 1) is given, the LRCK of the first entry in the
 * buffer is used */
void FT232H::alignToNextLRCK(uint8_t LRCK){
    uint8_t entry;
    int i = 0;
    dataBuffer.getN(&entry, 1);
    //if(LRCK > 1) LRCK = entry & 1;
    while((entry&1) == LRCK){               // Check if LRCK is still the same
        //dataBuffer.pop(&entry);
        dataBuffer.clearN(1);               // Get rid of it
        dataBuffer.getN(&entry, 1);         // Grab the next entry
        i++;
        if(i > 8){
            //cout << "Error: Over 32 clock cycles seen for one sample!" << endl;
            //exit(0);
        }
    }
}

void FT232H::programEEPROM()
{
    static FT_PROGRAM_DATA Data;            // static for initial zeros
    char manufacturer[] = "FTDI";
    char manufacturerID[] = "FT";
    char description[] = "UM232H";
    char serialNumber[] = "FTVSHU8R";

    Data.Signature1 = 0x00000000;
    Data.Signature2 = 0xffffffff;
    Data.Version = 5;
    Data.VendorId = 0x0403;
    Data.ProductId = 0x6014;
    Data.Manufacturer =  manufacturer;
    Data.ManufacturerId = manufacturerID;
    Data.Description = description;
    Data.SerialNumber = serialNumber;

    Data.MaxPower = 500;
    Data.PnP = 1;
    Data.SelfPowered = 0;
    Data.RemoteWakeup = 1;
    Data.Rev5 = 1;
    Data.IsoIn = 0;
    Data.IsoOut = 0;
    Data.PullDownEnable = 1;
    Data.SerNumEnable = 1;
    Data.USBVersionEnable = 0;
    Data.USBVersion = 0x0200;

    Data.SerNumEnableH = 1;

//  Data.ACDriveCurrentH = 8;
//  Data.ADDriveCurrentH = 8;

    // Cbus mux settings
    Data.Cbus4H = FT_232H_CBUS_TXRXLED;
    Data.Cbus5H = FT_232H_CBUS_IOMODE;      // CS5368 reset
    Data.Cbus6H = FT_232H_CBUS_CLK7_5;      // CLK 7.5Mhz
    Data.Cbus8H = FT_232H_CBUS_IOMODE;      // FT1248 SS_n / led1
    Data.Cbus9H = FT_232H_CBUS_IOMODE;      // flip flop clear / led2

    // FT1248 Settings
    Data.IsFT1248H = 1;                     // FT1248 enable
    Data.FT1248CpolH = 1;                   // clock idle high
    Data.FT1248LsbH = 0;                    // MSB first
    Data.FT1248FlowControlH = 0;            // flow control disabled

    Data.IsVCPH = 0;                        // disable VCP drivers

    ftStatus = FT_EE_Program(ftHandle, &Data);
    errCheck("programEE");
}

void FT232H::errCheck(string errString)
{
    switch(ftStatus){
        case FT_OK:
            return;
        case FT_INVALID_HANDLE:
            cout << "FT_INVALID_HANDLE: " << errString << endl;
            break;
        case FT_DEVICE_NOT_FOUND:
            cout << "FT_DEVICE_NOT_FOUND: " << errString << endl;
            break;
        case FT_DEVICE_NOT_OPENED:
            cout << "FT_DEVICE_NOT_OPENED: " << errString << endl;
            break;
        case FT_IO_ERROR:
            cout << "FT_IO_ERROR: " << errString << endl;
            break;
        case FT_INSUFFICIENT_RESOURCES:
            cout << "FT_INSUFFICIENT_RESOURCES: " << errString << endl;
            break;
        case FT_INVALID_PARAMETER:
            cout << "FT_INVALID_PARAMETER: " << errString << endl;
            break;
        case FT_INVALID_BAUD_RATE:
            cout << "FT_INVALID_VAUD_RATE: " << errString << endl;
            break;
        case FT_DEVICE_NOT_OPENED_FOR_ERASE:
            cout << "FT_DEVICE_NOT_OPENED_FOR_ERASE: " << errString << endl;
            break;
        case FT_DEVICE_NOT_OPENED_FOR_WRITE:
            cout << "FT_DEVICE_NOT_OPENED_FOR_WRITE: " << errString << endl;
            break;
        case FT_EEPROM_READ_FAILED:
            cout << "FT_EEPROM_READ_FAILED: " << errString << endl;
            break;
        case FT_EEPROM_WRITE_FAILED:
            cout << "FT_EEPROM_WRITE_FAILED: " << errString << endl;
            break;
        case FT_EEPROM_NOT_PRESENT:
            cout << "FT_EEPROM_NOT_PRESENT: " << errString << endl;
            break;
        case FT_EEPROM_NOT_PROGRAMMED:
            cout << "FT_EEPROM_NOT_PROGRAMMED: " << errString << endl;
            break;
        case FT_INVALID_ARGS:
            cout << "FT_INVALID_ARGS: " << errString << endl;
            break;
        case FT_NOT_SUPPORTED:
            cout << "FT_NOT_SUPPORTED: " << errString << endl;
            break;
        case FT_OTHER_ERROR:
            cout << "FT_OTHER_ERROR: " << errString << endl;
            break;
        default:
            cout << "UNKNOWN_ERROR: " << errString << endl;
    }
    exit(0);
}

ACBUS_out::ACBUS_out(FT232H* ft, uint8_t index){
    this->ft = ft;                  // save associated device
    mask_high = 1 << index;         // set bit mask used for driving high
    mask_low = ~(1 << index);       // set bit mask used for driving low
    value = 0;                      // initial output
}

void ACBUS_out::operator=(uint8_t out){
    value = out;                    // save current state
    if(value == 0) ft->CBUS_STATE &= mask_low;      // drive low
    else           ft->CBUS_STATE |= mask_high;     // else drive high
    // Write out to device
    ft->ftStatus = FT_SetBitMode(ft->ftHandle, ft->CBUS_STATE,
                             FT_BITMODE_CBUS_BITBANG);
}

uint8_t ACBUS_out::operator!() const{
    return !value;
}
