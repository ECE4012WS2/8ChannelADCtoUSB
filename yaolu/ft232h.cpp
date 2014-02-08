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

FT232H::FT232H()
  : SSn_led1(this, 2), CL_led2(this, 3), CS5368_reset(this, 0)
{
	CBUS_STATE = 0xF0;          // all pins are outputs, default low
}

void FT232H::open(uint16_t port){
    ftStatus = FT_Open(port, &ftHandle);
    errCheck("FT_Open failed");
}

void FT232H::reset(){
    ftStatus = FT_SetBitMode(ftHandle, 0x00, FT_BITMODE_RESET);
    errCheck("Reset failed");
}

void FT232H::purge(){
	ftStatus = FT_Purge(ftHandle, FT_PURGE_RX | FT_PURGE_TX);
	errCheck("FT_Purge failed");
}

void FT232H::close(){
	ftStatus = FT_Close(ftHandle);
    errCheck("FT_Close failed");
}

void FT232H::programEEPROM()
{
	static FT_PROGRAM_DATA Data;			// static for initial zeros
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

    Data.MaxPower = 90;
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
    Data.Cbus5H = FT_232H_CBUS_IOMODE;      // FT1248 SS_n
    Data.Cbus6H = FT_232H_CBUS_IOMODE;		// flip flop clear
    Data.Cbus8H = FT_232H_CBUS_IOMODE;      // UM232H onboard led1
    Data.Cbus9H = FT_232H_CBUS_IOMODE;      // UM232H onboard led2

	// FT1248 Settings
    Data.IsFT1248H = 1;						// FT1248 enable
    Data.FT1248CpolH = 1;					// clock idle high
    Data.FT1248LsbH = 0;					// MSB first
    Data.FT1248FlowControlH = 0;			// flow control disabled

	Data.IsVCPH = 0;						// disable VCP drivers

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

void ACBUS_out::operator=(uint8_t out){
	value = out;
	if(value == 0) ft->CBUS_STATE &= mask_low;
	else		   ft->CBUS_STATE |= mask_high;
	ft->ftStatus = FT_SetBitMode(ft->ftHandle, ft->CBUS_STATE,
							 FT_BITMODE_CBUS_BITBANG);
}

uint8_t ACBUS_out::operator!() const{
	return !value;
}
