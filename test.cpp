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

char macOrigin[18] = "aa:aa:aa:aa:aa:aa";
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

int main(int argc, char *args[])
{
    printMenu();
    // Node node1, node2;
    // BYTE mac1[6] = {0xAA, 0xEE, 0x11, 0x11, 0x11, 0x11};
    // BYTE mac2[6] =  {0xCC, 0x22, 0x11, 0x11, 0x11, 0x11};
    // memcpy(routeTable[0].mac, mac1, 6);
    // routeTable[0].port = 1;
    // routeTable[0].init = true;
    // routeTable[0].ttl = 2;
    // memcpy(routeTable[1].mac, mac2, 6);
    // routeTable[1].port = 2;
    // routeTable[1].ttl = 1;
    // routeTable[1].init = true;
    // printRouteTable(routeTable, MAX_CONNECTED_NODES);
    return 0;
}
