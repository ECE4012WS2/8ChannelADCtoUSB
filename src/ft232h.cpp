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
#include <unistd.h>             // usleep

using namespace std;

/* Table and function used to flip a byte, defined for convenient
 * debugging purposes */
uint8_t lut[16] = {
    0x0, 0x8, 0x4, 0xC,
    0x2, 0xA, 0x6, 0xE,
    0x1, 0x9, 0x5, 0xD,
    0x3, 0xB, 0x7, 0xF };
uint8_t flip( uint8_t n )
{
   return (lut[n&0x0F] << 4) | lut[n>>4];
}

/* Constructor, initializes ACBUS pins and initial variables */
FT232H::FT232H()
  : ftStatus(0),
    ftHandle(0),
    RxBytes(0),
    BytesReceived(0),
    SSn_RST(this, 0),
    CDIN(this, 1),
    CCLK(this, 2),
    CSn_CL(this, 3)
{
    /*
     * Upper nibble of CBUS_STATE defines direction.
     * Lower nibble defines the output with this mapping:
     *      bit 0: ACBUS 5
     *      bit 1: ACBUS 6
     *      bit 2: ACBUS 8
     *      bit 3: ACBUS 9
     *
     */
    CBUS_STATE = 0xF0;          // all pins are outputs

    // Initialize buffers
    for(uint32_t i = 0; i < 1024; i++) RxBuffer[i] = 0;
    dataBuffer.setSize(RAW_BUFFER_SIZE);
    for(uint32_t i = 0; i < 8; i ++){
        channelBuffer[i].setSize(CHANNEL_BUFFER_SIZE);
    }

    // Set register address map for ADC
    CHIP_ADDRESS = 0x9E;            // ADC chip address with write bit
    adc_regs.GCTL_addr = 0x01;
    adc_regs.OVFM_addr = 0x03;
    adc_regs.HPF_addr = 0x04;
    adc_regs.PDN_addr = 0x06;
    adc_regs.MUTE_addr = 0x08;
    adc_regs.SDEN_addr = 0x0A;
}

void FT232H::initCS5368()
{

#ifdef DEBUG_PRINT
    cout << "Preparing output pins" << endl;
#endif

    // Prepare chips and interfaces
    SSn_RST.set(0);         // hold ADC in reset and don't select FT1248
    CDIN.set(0);            // zero data line
    CCLK.set(0);            // zero clock line
    CSn_CL = 0;             // clear flipflop

    usleep(20000);

#ifdef DEBUG_PRINT
    cout << "Initializing CS5368" << endl;
#endif

    SSn_RST = 1;            // release ADC from reset and select FT1248

    usleep(20000);

#ifdef DEBUG_PRINT
    cout << "Writing to CS5368 Control Register" << endl;
#endif

    // Set Global Control Register and write to ADC with SPI
    adc_regs.GCTL.bit.CP_EN = 0x1;          // enable control port mode
    adc_regs.GCTL.bit.CLKMODE = 0x0;        // no divide by 1.5
    adc_regs.GCTL.bit.MDIV = 0x1;           // divide by 2
    adc_regs.GCTL.bit.DIF = 0x0;            // left justified mode
    adc_regs.GCTL.bit.MODE = 0x0;           // quadruple-speed mode
    write_SPI(&adc_regs.GCTL.all);          // write to ADC

    // Disable high pass filter
    //adc_regs.HPF = 0xFF;
    //write_SPI(&adc_regs.HPF.all);
}

void FT232H::open()
{
	long locIdBuf[16];
	for(int i = 0; i < 16; i++) locIdBuf[i] = -1;
	 int numDevs;
	ftStatus = FT_ListDevices(locIdBuf,&numDevs,FT_LIST_ALL|FT_OPEN_BY_LOCATION);
	errCheck("FT_ListDevices cant find a device");

	if (ftStatus == FT_OK) {
		for(int i = 0; i < 16;++i){
			std::cout << std::endl << "Location[" << i << "] = " << locIdBuf[i] << std::endl;
		}
		this->open(locIdBuf[0]);
	}
}

void FT232H::open(uint16_t port)
{

#ifdef DEBUG_PRINT
    cout << "Opening FT232H Device" << endl;
#endif

    ftStatus = FT_Open(port, &ftHandle);
    errCheck("FT_Open failed");
}

void FT232H::reset()
{

#ifdef DEBUG_PRINT
    cout << "Reseting FT232H" << endl;
#endif

    ftStatus = FT_SetBitMode(ftHandle, 0x00, FT_BITMODE_RESET);
    errCheck("Reset failed");

    // Sets a read timeout of 5 sec and write timeout of 1 sec
    ftStatus = FT_SetTimeouts(ftHandle, 5000, 1000);
    errCheck("FT_SetTimeouts");
}

void FT232H::purge()
{

#ifdef DEBUG_PRINT
    cout << "Purging buffers" << endl;
#endif

    // Clear both receive and send buffers on FT232H
    ftStatus = FT_Purge(ftHandle, FT_PURGE_RX | FT_PURGE_TX);
    errCheck("FT_Purge failed");
}

void FT232H::close()
{

#ifdef DEBUG_PRINT
    cout << "Closing FT232H Device" << endl;
#endif

    ftStatus = FT_Close(ftHandle);
    errCheck("FT_Close failed");
}

void FT232H::read()
{
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

DWORD FT232H::blockingRead(DWORD bytes, DWORD timeout)
{

#ifdef DEBUG_PRINT
    cout << "Reading " << (int) bytes << " bytes of data (blocking)" << endl;
#endif

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

bool FT232H::formatSample()
{
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
void FT232H::alignToNextLRCK(uint8_t LRCK)
{
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

void FT232H::write_SPI(uint8_t* reg)
{
    uint32_t clock_wait = 10000;
    uint8_t map = *(reg-1);
    uint8_t data = *reg;

    CSn_CL.set(1);
    CCLK = 1;

    cout << "Press c to continue" << endl;
    int c;
    do {
         c=getchar();
         putchar (c);
    } while (c != 'c');
    cout << endl;

    usleep(clock_wait);

    uint8_t addr = flip(CHIP_ADDRESS);
    for(int i = 0; i < 16; i++){
        if(i == 0) CSn_CL.set(0);
        CCLK.set(!CCLK);
        if(i%2 == 0){
            CDIN.set(addr & 0x01);
            addr >>= 1;
        }
        CDIN.write();
        usleep(clock_wait);
    }
    map = flip(map);
    for(int i = 0; i < 16; i++){
        CCLK.set(!CCLK);
        if(!CCLK){
            CDIN.set(map & 0x01);
            map >>= 1;
        }
        CDIN.write();
        usleep(clock_wait);
    }
    data = flip(data);
    for(int i = 0; i < 18; i++){
        CCLK.set(!CCLK);
        if(!CCLK && i < 16){
            CDIN.set(data & 0x01);
            data >>= 1;
        }
        if(i == 16) CSn_CL.set(1);
    CDIN.write();
        usleep(clock_wait);
    }
}

void FT232H::programEEPROM()
{

#ifdef DEBUG_PRINT
    cout << "Programming EEPROM" << endl;
#endif

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
    Data.Cbus5H = FT_232H_CBUS_IOMODE;      // 
    Data.Cbus6H = FT_232H_CBUS_IOMODE;      // 
    Data.Cbus8H = FT_232H_CBUS_IOMODE;      // 
    Data.Cbus9H = FT_232H_CBUS_IOMODE;      // 

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

ACBUS_out::ACBUS_out(FT232H* ft, uint8_t index)
{
    this->ft = ft;                  // save associated device
    mask_high = 1 << index;         // set bit mask used for driving high
    mask_low = ~(1 << index);       // set bit mask used for driving low
    value = 0;                      // initial output
}

void ACBUS_out::set(uint8_t out)
{
    value = out;                    // save current state
    if(value == 0) ft->CBUS_STATE &= mask_low;      // drive low
    else           ft->CBUS_STATE |= mask_high;     // else drive high
}

void ACBUS_out::write()
{
    ft->ftStatus = FT_SetBitMode(ft->ftHandle, ft->CBUS_STATE,
                                 FT_BITMODE_CBUS_BITBANG);
}

void ACBUS_out::operator=(uint8_t out)
{
    set(out);
    write();
}

uint8_t ACBUS_out::operator!() const
{
    return !value;
}

