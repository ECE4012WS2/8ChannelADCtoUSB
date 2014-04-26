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

#include "time.h"
#include <string.h>

//#define TIME_LOG

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
  : SSn_RST(this, 0),
    CDIN(this, 1),
    CCLK(this, 2),
    CSn_CL(this, 3)
{
    ftStatus = 0;
    ftHandle = 0;
    crystal_freq = 27450000;
    socket_type = 1;
    channel_num = 8;
    RxBytes = 0;
    BytesReceived = 0;
    ip = "127.0.0.1";
    port = 0;
    socket = NULL;
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
    dataBuffer.setSize(265000);

    // Set register address map for ADC
    CHIP_ADDRESS = 0x9E;            // ADC chip address with write bit
    adc_regs.GCTL_addr = 0x01;
    adc_regs.OVFM_addr = 0x03;
    adc_regs.HPF_addr = 0x04;
    adc_regs.PDN_addr = 0x06;
    adc_regs.MUTE_addr = 0x08;
    adc_regs.SDEN_addr = 0x0A;

    // Open device and reset it
    open(0);
    reset();
}

FT232H::~FT232H()
{
    if(socket) delete socket;
    close();
}

void FT232H::setSamplingRate(int rate)
{
    double r = (double) rate;
    double mclk = crystal_freq;
    double max_rates[5];

    if( r > 108000){
        adc_regs.GCTL.bit.MODE = 0x2;           // quadruple-speed mode
    }else if(r > 54000){
        adc_regs.GCTL.bit.MODE = 0x1;           // double-speed mode
        mclk /= 2;
    }else{
        adc_regs.GCTL.bit.MODE = 0x0;           // single-speed mode
        mclk /= 4;
    }

    // Calculate avaliable rates derived from clock freq
    max_rates[0] = (double) mclk / 64;
    max_rates[1] = max_rates[0] / 1.5;
    max_rates[2] = max_rates[0] / 2;
    max_rates[3] = max_rates[0] / 3;
    max_rates[4] = max_rates[0] / 4;

    // Set clock dividers appropriately
    if(r < max_rates[0]+5 && r > max_rates[0]-5){
        adc_regs.GCTL.bit.CLKMODE = 0x0;
        adc_regs.GCTL.bit.MDIV = 0x1;
    }else if(r < max_rates[1]+5 && r > max_rates[1]-5){
        adc_regs.GCTL.bit.CLKMODE = 0x1;
        adc_regs.GCTL.bit.MDIV = 0x0;
    }else if(r < max_rates[2]+5 && r > max_rates[2]-5){
        adc_regs.GCTL.bit.CLKMODE = 0x0;
        adc_regs.GCTL.bit.MDIV = 0x1;
    }else if(r < max_rates[3]+5 && r > max_rates[3]-5){
        adc_regs.GCTL.bit.CLKMODE = 0x1;
        adc_regs.GCTL.bit.MDIV = 0x1;
    }else if(r < max_rates[4]+5 && r > max_rates[4]-5){
        adc_regs.GCTL.bit.CLKMODE = 0x0;
        adc_regs.GCTL.bit.MDIV = 0x2;
    }else{
        std::cout << "Unknown sampling rate. "
                  << "Valid rates with current crystal are:" << std::endl;
        double dividers[] = {1, 1.5, 2, 3, 4};
        for(int i = 1; i <= 4; i*=2){
            mclk = crystal_freq / 64 / i;
            for(int j = 0; j < 5; j++){
                r = mclk / dividers[j];
                if(i == 1 && r <= 216000 && r >= 108000){
                    cout << "quadruple-speed mode: " << (int)r << " Hz" << endl;
                }else if(i == 2 && r < 108000 && r >= 54000){
                    cout << "double-speed mode:    " << (int)r << " Hz" << endl;
                }else if(i == 4 && r >= 2000 && r < 54000){
                    cout << "single-speed mode:    " << (int)r << " Hz" << endl;
                }
            }
        }
        exit(1);
    }

#ifdef DEBUG_PRINT
    cout << "Sampling speed of " << r << " requested" << endl;
    cout << "MODE is set to: " << (int)adc_regs.GCTL.bit.MODE << endl;
    cout << "CLKMODE is set to: " << (int)adc_regs.GCTL.bit.CLKMODE << endl;
    cout << "MDIV is set to: " << (int)adc_regs.GCTL.bit.MDIV << endl;
#endif

    // Write global control register
    write_SPI(&adc_regs.GCTL.all);
}

void FT232H::setHighPassFilter(bool on)
{
    if(on) adc_regs.HPF = 0x00;
    else   adc_regs.HPF = 0xFF;

#ifdef DEBUG_PRINT
    if(on) cout << "Turning ON High Pass Filter" << endl;
    else   cout << "Turning OFF High Pass Filter" << endl;
#endif

    write_SPI(&adc_regs.HPF);
}

void FT232H::setChannelNum(int n)
{
    channel_num = (uint32_t) n;
}

void FT232H::setCrystalFreq(int freq)
{
    crystal_freq = (uint32_t) freq;
}

void FT232H::connect(string ip, int port)
{
    if(socket){
        cout << "Error: connection already exists." << endl;
        exit(1);
    }
    if(socket_type == 1){
        socket = new TCPSocket(ip, port, true);
    }else if(socket_type == 0){
      cout <<"Error: UDP NOT IMPLEMENTED" << endl;
      exit(1);
    }
}

void FT232H::disconnect()
{
    delete[] socket;
    socket = NULL;
}

void FT232H::send(){
      header_t header;
      uint8_t currentChannel = 0;
      uint32_t numEntries;

      if(socket){
        //Socket is not null
        //Send 8 bytes with channelnum,size
        do{
          header.currentChannel = htonl(currentChannel+1);
          numEntries = channelBuffer[currentChannel].getEntries();
          header.size = htonl(numEntries);
          socket->send(&header,sizeof(header_t));

          channelBuffer[currentChannel].getN(socketBuffer,numEntries,true);

          socket->send(socketBuffer,numEntries * sizeof(uint32_t));

        }
        while(++currentChannel < channel_num);


      }
}

void FT232H::clear()
{
    // Clear raw data buffer and channel buffers
    dataBuffer.clearN(dataBuffer.getEntries());
    for(int i = 0; i < 8; i++){
        channelBuffer[i].setSize(0);
    }

    // Clear buffers on ft232h
    purge();
}

void FT232H::sendSamples(int sample_count)
{
    // Clear data buffer and allocate space for channel buffers
    dataBuffer.clearN(dataBuffer.getEntries());
    for(uint32_t i = 0; i < channel_num; i++){
        channelBuffer[i].setSize(4500);
    }

#ifdef DEBUG_PRINT
    cout << "Buffering " << sample_count << " samples...";
#endif

    int samples_sent = 0;

    // Align data on the first buffer read as odd channels for each
    // sample is clocked out first
    blockingRead(BYTES_TO_BUFFER);
    alignToNextLRCK(1, 32);
    alignToNextLRCK(0, 32);
    while(formatSample()) {}
    this->send();
    // Continue to read data in specified chunks and process them
    // into samples until the desired sample count is reached
    while(samples_sent < sample_count){
        blockingRead(BYTES_TO_BUFFER);

        //Format each sample, one at a time, until no more are left
        while(formatSample()) {}
        this->send();
        samples_sent += channelBuffer[0].getEntries();
        for(uint32_t i = 0; i < channel_num; ++i){
          //channelBuffer[i].clearN(channelBuffer[i].getEntries());
          channelBuffer[i].clearAll();
        }
    }


#ifdef DEBUG_PRINT
    cout << "Done" << endl;
#endif
}

void FT232H::buffer(int sample_count)
{
    // Clear data buffer and allocate space for channel buffers
    dataBuffer.clearN(dataBuffer.getEntries());
    for(uint32_t i = 0; i < channel_num; i++){
        channelBuffer[i].setSize(sample_count + BYTES_TO_BUFFER/64 + 5);
    }

    dataBuffer.setSize((sample_count+100)*64);

#ifdef DEBUG_PRINT
    cout << "Buffering " << sample_count << " samples...";
#endif

    // Align data on the first buffer read as odd channels for each
    // sample is clocked out first
    blockingRead(BYTES_TO_BUFFER);
    alignToNextLRCK(1, 32);
    alignToNextLRCK(0, 32);

    // Continue to read data in specified chunks and process them
    // into samples until the desired sample count is reached
    while(dataBuffer.getEntries() < sample_count*64){
        blockingRead(BYTES_TO_BUFFER);
    }

#ifdef DEBUG_PRINT
    cout << "Done" << endl;
#endif

    //Format each sample, one at a time, until no more are left
    while(formatSample()) {}
}

void FT232H::read(int* buf, int samples, int channel)
{
    // Check that channel exists
    if((uint32_t)channel > channel_num && channel < 1){
        cout << "Error: Channel " << channel << " doesn't exist!" << endl;
        exit(1);
    }

    // Check that there is enough samples in buffer
    if(channelBuffer[channel-1].getEntries() < samples){
        cout << "Error: trying to read more than available samples" << endl;
        exit(1);
    }

    // Extract from buffer and clear it
    channelBuffer[channel-1].getN((uint32_t*)buf, samples);
    channelBuffer[channel-1].clearN(samples);
}

void FT232H::setSocketType(std::string type)
{
    if(!type.compare("UDP")){
        socket_type = 0;
    }else if(!type.compare("TCP")){
        socket_type = 1;
    }else{
        cout << "Error: socket type not recognized. Options are:"
             << endl << "TCP" << endl << "UDP" << endl;
        exit(1);
    }
}

void FT232H::init_ADC()
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
    adc_regs.GCTL.bit.MODE = 0x2;           // quadruple-speed mode
    write_SPI(&adc_regs.GCTL.all);          // write to ADC

    // Initialize other register values to default ADC values
    adc_regs.PDN.all = 0x00;                // everything is powered
    adc_regs.HPF = 0x00;                    // high pass filter enabled
    adc_regs.OVFM = 0xFF;                   // overflow interrupt enabled
    adc_regs.MUTE = 0x00;                   // no muted
    adc_regs.SDEN = 0x00;                   // SDOUT pins enabled

    CSn_CL = 1;             //
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

void FT232H::readBuffer()
{

#ifdef TIME_LOG
    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);
#endif

    // Even non blocking FTDI calls take too long, which will cause
    // the buffer to overflow at high sampling speeds!

    // Get the number bytes currently in FT232H's buffer
    ftStatus = FT_GetQueueStatus(ftHandle, &RxBytes);

#ifdef TIME_LOG
    clock_gettime(CLOCK_REALTIME, &end);
    cout << "readBuffer s: " << end.tv_sec-start.tv_sec << " ns: " << end.tv_nsec-start.tv_nsec << endl;
#endif

    errCheck("FT_GetQueueStatus failed");

    if(RxBytes > 0){

        // Read in the current amount. This function should not block
        // because it is requests the number of bytes from FT_GetQueueStatus
        ftStatus = FT_Read(ftHandle, RxBuffer, RxBytes, &BytesReceived);
        errCheck("FT_Read failed");

        if(RxBytes != BytesReceived){
            //cout << "Bytes read mis-match" << endl;
        }
        dataBuffer.addN(RxBuffer, (uint32_t) BytesReceived);
    }
}

DWORD FT232H::blockingRead(DWORD bytes)
{
#ifdef TIME_LOG
    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);
#endif

    ftStatus = FT_Read(ftHandle, RxBuffer, bytes, &BytesReceived);

#ifdef TIME_LOG
    clock_gettime(CLOCK_REALTIME, &end);
    cout << "blockingRead s: " << end.tv_sec-start.tv_sec << " ns: " << end.tv_nsec-start.tv_nsec << endl;
#endif

    errCheck("FT_Read");
    if(BytesReceived < bytes){
        cout << "Timed out in blockingRead" << endl;
        exit(0);
    }

    dataBuffer.addN(RxBuffer, (uint32_t) BytesReceived);
    return BytesReceived;
}

bool FT232H::formatSample()
{
    // Make sure enough data is in buffer
    if(dataBuffer.getEntries() < 50) return false;

// Save next 64 entries for debugging in case over 32 clocks seen for one sample
//dataBuffer.getN(history, 64);

    uint8_t entry;
    // Peek into first entry for the LRCK bit, which is high in LJ mode
    // when the channel is odd
    dataBuffer.getN(&entry, 1);
    uint8_t LRCK = entry & 1;

    uint32_t i0, i1, i2, i3;
    // Add a new entry for the even or odd channels, depending on LRCK
    // and how many channels needed
    if(channel_num >= (uint32_t)(2-LRCK)) i0 = channelBuffer[1-LRCK].add(0);
    if(channel_num >= (uint32_t)(4-LRCK)) i1 = channelBuffer[3-LRCK].add(0);
    if(channel_num >= (uint32_t)(6-LRCK)) i2 = channelBuffer[5-LRCK].add(0);
    if(channel_num >= (uint32_t)(8-LRCK)) i3 = channelBuffer[7-LRCK].add(0);

    // For each of the next 24 bits, store the bit into the correct
    // position of each channel sample buffer
    for(int i = 0; i < 24; i++){
        dataBuffer.getN(&entry, 1);                 // Get one entry
        if(LRCK != (entry&1)) return true;
        if(channel_num >= (uint32_t)(2-LRCK))
            channelBuffer[1-LRCK][i0] |= ((uint32_t) ((entry&2)>>1) << (23-i));
        if(channel_num >= (uint32_t)(4-LRCK))
            channelBuffer[3-LRCK][i1] |= ((uint32_t) ((entry&4)>>2) << (23-i));
        if(channel_num >= (uint32_t)(6-LRCK))
            channelBuffer[5-LRCK][i2] |= ((uint32_t) ((entry&8)>>3) << (23-i));
        if(channel_num >= (uint32_t)(8-LRCK))
            channelBuffer[7-LRCK][i3] |= ((uint32_t) ((entry&16)>>4) << (23-i));
        dataBuffer.clearN(1);                       // Remove one entry
    }

    // Skip extra bits to the next LRCK
    alignToNextLRCK(LRCK, 8);

    return true;
}

/* Aligns buffer to the next set of channels different from the given LRCK.
 * If no valid LRCK (0 or 1) is given, the LRCK of the first entry in the
 * buffer is used */
void FT232H::alignToNextLRCK(uint8_t LRCK, uint8_t limit)
{
    uint8_t entry;
    int i = 0;
    // Get next entry and check if LRCK is still the same
    while(dataBuffer.getN(&entry, 1) && (entry&1) == LRCK){
        //dataBuffer.pop(&entry);
        dataBuffer.clearN(1);               // Get rid of it
        i++;
        if(i > limit){
//            cout << "Warning: Over 32 clock cycles seen for one sample!" << endl;
        }
    }
}

void FT232H::write_SPI(uint8_t* reg)
{
    uint32_t clock_wait = 10000;        // clock change wait time
    uint8_t map = *(reg-1);             // get register address
    uint8_t data = *reg;                // copy data

    // Initialize select and clock lines high
    CSn_CL.set(1);
    CCLK = 1;

    usleep(clock_wait);

    // Send ship address
    uint8_t addr = flip(CHIP_ADDRESS);
    for(int i = 0; i < 16; i++){        // 16 clock changes to write 8 bits
        if(i == 0) CSn_CL.set(0);       // drop select when starting
        CCLK.set(!CCLK);                // flip clock every time

        // Update bit every time clock drops
        if(i%2 == 0){
            CDIN.set(addr & 0x01);
            addr >>= 1;
        }
        CDIN.write();                   // write all pin changes
        usleep(clock_wait);             // delay
    }

    // Send register address
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

    // Send data to write
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
    exit(1);
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
