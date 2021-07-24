/*
    Group:
    - Tomás Jiménez
    - Kevin Herrera
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wiringPi.h>
#include "ethernet/ethernet.h"
#include "protocol/protocol.h"
#include "slip/slip.h"
#include "menu/menu.h"
#include "ring/ring.h"
#include "helpers/helpers.h"
#include <unistd.h>
#include <poll.h>

#define MAX_TRANSFER_SIZE 300
#define BYTE unsigned char



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

char macDestiny[18];

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
char macOrigin[18];

int clockPin;
int txPin;
int rxPin;
int txPin2;
int rxPin2;

int transmissionPort;

void startTransmission();
void cb(void);
void processBit(bool level);
void processBit2(bool level);
void loop();

char macBroadcast[18] = "ff:ff:ff:ff:ff:ff";

Node *routeTable;
bool initialization = true;

int main(int argc, char *args[])
{
    // Check for ARGS: macOrigin clockPin txPin rxPin txPin2 rxPin2
    if (argc >= 7)
    {
        memcpy(macOrigin, args[1], sizeof(macOrigin));
        clockPin = atoi(args[2]);
        txPin = atoi(args[3]);
        rxPin = atoi(args[4]);
        txPin2 = atoi(args[5]);
        rxPin2 = atoi(args[6]);
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

    return 0;
}

void loop()
{
    int option = 0;
    while (true)
    {
        printMenu();
        struct pollfd mypoll = { STDIN_FILENO, POLLIN | POLLPRI };
        if (poll(&mypoll, 1, 3000)) {
            scanf("%d", &option);
        } else {
            option = 0;
        }
        printf("opcion: %d\n", option);
        delay(5000);
    }
}

void cb(void)
{
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

void processBit(bool level)
{

    //Insert a bit in actual byte
    BYTE pos = nbitsReceived;
    if (nbitsReceived > 7)
    {
        pos = 7;
        bytesReceived[nbytesReceived] = bytesReceived[nbytesReceived] >> 1;
        bytesReceived[nbytesReceived] &= 0x7f;
    }
    bytesReceived[nbytesReceived] |= level << pos;

    //Verify if transmission has started
    if (!transmissionStartedReceive && bytesReceived[nbytesReceived] == 0xC0)
    {
        transmissionStartedReceive = true;
        nbitsReceived = 0;
        nbytesReceived++;
        return;
    }

    // update counters and flags
    nbitsReceived++;
    if (transmissionStartedReceive)
    {
        if (nbitsReceived == 8)
        {
            nbitsReceived = 0;
            if (bytesReceived[nbytesReceived] == 0xC0 && nbytesReceived > 0)
            {
                transmissionStartedReceive = false;
                memcpy((void *)slipArrayReceived, (void *)bytesReceived, nbytesReceived + 1);
                memset(bytesReceived, 0, sizeof(bytesReceived));
                nbytesReceived = 0;
                boolReceivedFrame = true;
                return;
            }
            nbytesReceived++;
        }
    }
}

void processBit2(bool level)
{
    //Insert a bit in actual byte
    BYTE pos = nbitsReceived2;
    if (nbitsReceived2 > 7)
    {
        pos = 7;
        bytesReceived2[nbytesReceived2] = bytesReceived2[nbytesReceived2] >> 1;
        bytesReceived2[nbytesReceived2] &= 0x7f;
    }
    bytesReceived2[nbytesReceived2] |= level << pos;

    //Verify if transmission has started
    if (!transmissionStartedReceive2 && bytesReceived2[nbytesReceived2] == 0xC0)
    {
        transmissionStartedReceive2 = true;
        nbitsReceived2 = 0;
        nbytesReceived2++;
        return;
    }

    // update counters and flags
    nbitsReceived2++;
    if (transmissionStartedReceive2)
    {
        if (nbitsReceived2 == 8)
        {
            nbitsReceived2 = 0;
            if (bytesReceived2[nbytesReceived2] == 0xC0 && nbytesReceived2 > 0)
            {
                transmissionStartedReceive2 = false;
                memcpy((void *)slipArrayReceived2, (void *)bytesReceived2, nbytesReceived2 + 1);
                memset(bytesReceived2, 0, sizeof(bytesReceived2));
                nbytesReceived2 = 0;
                boolReceivedFrame2 = true;
                return;
            }
            nbytesReceived2++;
        }
    }
}

void startTransmission()
{
    transmissionStartedSend = true;
}
