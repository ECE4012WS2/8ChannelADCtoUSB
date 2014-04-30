// The MIT License (MIT)
//
// Copyright (c) 2014 Austin Ward, Yao Lu, Fujun Xie, Eric Patterson, Mohan Yang
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

/* Network API Interface proposal
 *
 * 8 channel simultanously sampled ADC USB device
 *
 *
 * Terminology:
 *
 * Client:
 *     PC where the ADC is connected through USB
 *
 * Server:
 *     PC where the sampled data is sent to
 *
 *
 * If both the client and server are initiated in the same program,
 * data will be copied straight from local buffers
 *
 *
 * Example usage retriving 500 samples of 8 channels on same PC:
 *
 *     SimulADC adc(8, 216000, string(""), 10000, 10000);
 *     adc.initServer();
 *     adc.initClient();
 *     adc.startClient();
 *
 *     int* buffer = new int[500*8];
 *     while(adc.getBufferCount() < 500) {}
 *     adc.readSamples(buffer, 0, 500);
 *
 *     adc.stopClient();
 *     adc.stopServer();
 *
 */

class SimulADC {
  public:
    SimulADC();
    SimulADC(int channels, int rate, string destIP, int clientBufSize, int serverBufsize);
    ~SimulADC();

/********************************************************************************
 * ADC Configuration Functions
 *******************************************************************************/

    /* Set the sampling rate of ADC (MCLK/64, MCLK/(64*2), or MCLK/(64*4) */
    void setSamplingRate(int rate);

    /* Set the number of ADC channels to use (1-8) */
    void setChannels(int n);

/********************************************************************************
 * Client Functions
 *******************************************************************************/

    /* Set IP of server, where sampled data are sent, blank to send to self */
    void setDestIP(string ip);

    /* Set the number of samples per channel to store before overflow error */
    void setClientBufferSize(int size);

    /* Called to initialize the client and connect to the server */
    void initClient();

    /* Start sampling data and send to server */
    void startClient();

    /* Stop sampling, but send remaining queued data in buffer to server */
    void stopClient();

    /* Clears client buffer and resets all hardware */
    void resetClient();

/********************************************************************************
 * Server Functions
 *******************************************************************************/

    /* Set the number of samples per channel to store at server */
    void setServerBufferSize(int size);

    /* Called to initialize the server and start listening for client connection */
    void startServer();

    /* Stops server */
    void stopServer();

    /* Returns the number of samples in the server buffer */
    int getBufferCount();

    /* Given the channel and number of samples, data is copied into the provided
     * buffer and removed from the server buffer. If channel is 0, all channels
     * will be put into provided buffer in order, one sample at a time */
    void readSamples(int* buffer, int channel, int num);

    /* Clears the buffer of server */
    void clearBuffer();

/********************************************************************************
 * Common Server and Client Functions
 *******************************************************************************/

    /* Port number used to send and receive */
    void setPort(int port);

  private:
    FT232H ft;
};
