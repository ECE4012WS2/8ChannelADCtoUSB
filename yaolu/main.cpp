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

using namespace std;

int main(int argc, char* argv[])
{
	bool first_run = false;				// get this as a cmd arg later

	cout << "Initializing..." << endl;
	FT232H ft;
	ft.open(0);
	ft.reset();
	cout << "Done." << endl;

	if(first_run){
		cout << "Programming EEPROM..." << endl;
		ft.programEEPROM();
		cout << "Done." << endl;
	}

	cout << "Settings up" << endl;
	ft.SSn_led1 = 1;					// not selecting FT1248
	ft.CL_led2 = 0;						// clear flip flop
	ft.CS5368_reset = 0;				// hold in reset

	usleep(500);

	cout << "Starting CS5368" << endl;
	ft.CS5368_reset = 1;				// release reset

	cout << "Enabling FT1248" << endl;
	ft.SSn_led1 = 0;					// enable FT1248

	usleep(20);

	// FT1248 should be seeings all zeros at this point, which sets
	// it up for 8-bit mode and storing received data into buffer

	cout << "Enabling flip flop to clock data from ADC" << endl;
	ft.CL_led2 = 1;						// release clear on flip flop

	cout << "Clearing buffers" << endl;
	ft.purge();							// clear buffers

	while(1){
		// receive data
	}

	ft.close();
	return 0;
}

