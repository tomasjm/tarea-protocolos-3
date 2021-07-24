/*
    Group:
    - Tomás Jiménez
    - Kevin Herrera
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ethernet/ethernet.h"
#include "protocol/protocol.h"
#include "slip/slip.h"
#include "menu/menu.h"
#include "ring/ring.h"
#include "helpers/helpers.h"
#include <unistd.h>
#include <poll.h>
#include <time.h>

#include <wiringPi.h>

#define MAX_TRANSFER_SIZE 300
#define MAX_CONNECTED_NODES 2
#define BYTE unsigned char

int transmissionPort;

void startTransmission();
void cb(void);
void processBit(bool level);
void processBit2(bool level);
void loop();
void broadcast();
void checkReceivedTransmission();

Node routeTable[MAX_CONNECTED_NODES];

int timeToBroadcast = 0;
int currentTime = 0;

char macOrigin[18] = "00:00:00:00:00:00";
char macBroadcast[18] = "ff:ff:ff:ff:ff:ff";
BYTE byteMacBroadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
BYTE byteMacOrigin[6];

// GLOBAL VARS FOR SENDING PURPOSES
volatile int nbitsSend = 0;
BYTE slipArrayToSend[MAX_TRANSFER_SIZE];
volatile int nbytesSend = 0;
BYTE len = 10;
int nones = 0;
bool transmissionStartedSend = false;
int endCount = 0;
Ethernet ethernet;
Frame frame;

// GLOBAL VARS FOR RECEIVING PURPOSES
volatile int nbitsReceived, nbitsReceived2 = 0;
volatile int nbytesReceived, nbytesReceived2 = 0;
bool transmissionStartedReceive, transmissionStartedReceive2 = false;
bool boolReceivedFrame, boolReceivedFrame2 = false;
BYTE bytesReceived[MAX_TRANSFER_SIZE], bytesReceived2[MAX_TRANSFER_SIZE];
BYTE slipArrayReceived[MAX_TRANSFER_SIZE], slipArrayReceived2[MAX_TRANSFER_SIZE];
bool error, error2 = false;
Frame receivedFrame, receivedFrame2;
Ethernet receivedEthernet, receivedEthernet2;

int clockPin;
int txPin;
int rxPin;
int txPin2;
int rxPin2;

int main(int argc, char *args[]) {

    // Check for ARGS: strMacSource clockPin txPin rxPin txPin2 rxPin2
    if (argc >= 7)
    {
        memcpy(macOrigin, args[1], sizeof(macOrigin));
        clockPin = atoi(args[2]);
        txPin = atoi(args[3]);
        rxPin = atoi(args[4]);
        txPin2 = atoi(args[5]);
        rxPin2 = atoi(args[6]);
        convertMacAddressToByteArray(macOrigin, byteMacOrigin);
    }
    else
    {
        printf("You must execute this program providing arguments\n");
        printf("OriginMacAddress ClockPin TxPin RxPin TxPin2 RxPin2\n");
        exit(1);
    }
    // Setup wiring pi
    if (wiringPiSetup() == -1)
    {
        printf("Error initializing wiring pi\n");
        exit(1);
    }
    //pin config
    pinMode(rxPin, INPUT);
    pinMode(rxPin2, INPUT);
    pinMode(txPin, OUTPUT);
    pinMode(txPin2, OUTPUT);
    printf("Pins: clock: %d | tx: %d | rx: %d\n", clockPin, txPin, rxPin);
    printf("Pins2: clock: %d | tx: %d | rx: %d\n", clockPin, txPin2, rxPin2);
    delay(5000);

    // CONFIGURE INTERRUPT FOR SENDING AND RECEIVING DATA
    if (wiringPiISR(clockPin, INT_EDGE_BOTH, &cb) < 0)
    {
        printf("Unable to start interrupt function\n");
    }
    while(true) {
        printf("waiting callback\n");
        delay(1000);
    }
    return 0;
}

void cb(void)
{
    printf("CALLBACKKKKK\n");
    bool level = digitalRead(rxPin);
    bool level2 = digitalRead(rxPin2);
    processBit(level);
    processBit2(level2);
    if (transmissionStartedSend)
    {
        if (endCount == 0 && slipArrayToSend[nbytesSend] != 0xC0)
        {
            nbytesSend++;
            return;
        }

        // Writes on TX Pin
        digitalWrite(transmissionPort, (slipArrayToSend[nbytesSend] >> nbitsSend) & 0x01);

        // Update bit counter
        nbitsSend++;

        // Update byte counter
        if (nbitsSend == 8)
        {
            nbitsSend = 0;
            endCount += slipArrayToSend[nbytesSend] == 0xC0;
            // Finish transmission
            if (slipArrayToSend[nbytesSend] == 0xC0 && endCount > 1)
            {
                endCount = 0;
                nbytesSend = 0;
                transmissionStartedSend = false;
                return;
            }
            nbytesSend++;
        }
    }
    else
    {
        // Channel idle
        digitalWrite(txPin, 1);
        digitalWrite(txPin2, 1);
    }
}