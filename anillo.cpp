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

// Prototypes

void startTransmission(char *msg);
void cb(void);
void processBit(bool level);
void processBit2(bool level);
void loop();
void broadcast();
void checkReceivedTransmission();
void setupTransmissionPort(int port);
void setTypeTransmission(char *msg);

// General purpose global vars

int transmissionPort;
int idlePort;

int clockPin;
int txPin;
int rxPin;
int txPin2;
int rxPin2;

Node routeTable[MAX_CONNECTED_NODES];

char macOrigin[18];
char macBroadcast[18] = "ff:ff:ff:ff:ff:ff";
BYTE byteMacBroadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
BYTE byteMacOrigin[6];

// Broadcast control global vars
int timeToBroadcast = 0;
int currentTime = 0;

// GLOBAL VARS FOR SENDING PURPOSES
volatile int nbitsSend = 0;
BYTE slipArrayToSend[MAX_TRANSFER_SIZE];
volatile int nbytesSend = 0;
int nones = 0;
bool transmissionStartedSend = false;
int endCount = 0;
Ethernet ethernet;
Frame frame;
char sendTypeTransmission[30];
bool sendingTelemetry = false;

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

int main(int argc, char *args[])
{

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

    loop(); // Main LOOP of the program
    return 0;
}

void loop()
{
    int option = 0;
    while (true)
    {
        printMenu();
        char line[20];
        struct pollfd mypoll = {STDIN_FILENO, POLLIN | POLLPRI};
        if (poll(&mypoll, 1, 3000))
        {
            fgets(line, sizeof(line), stdin);
            sscanf(line, "%d", &option);
        }
        else
        {
            printf("Timeout...\n");
            delay(500);
        }
        clearScreen();

        // Check if input option is registered
        if (option == 1)
        {
            int almostOneNode = 0;
            for (int i = 0; i < MAX_CONNECTED_NODES; i++)
            {
                if (routeTable[i].init == true)
                {
                    almostOneNode++;
                    continue;
                }
            }
            if (almostOneNode == 0)
            {
                printf("Have no detected nodes connected to the network, please wait a moment...\n");
                delay(2000);
            }
            else
            {
                printf("Select a Node to send a Telemetry\n");
                printRouteTable(routeTable, MAX_CONNECTED_NODES);
                int selecNode = -1;
                char line2[20];
                struct pollfd mypoll = {STDIN_FILENO, POLLIN | POLLPRI};
                if (poll(&mypoll, 1, 6000))
                {
                    fgets(line2, sizeof(line2), stdin);
                    sscanf(line2, "%d", &selecNode);
                    if (selecNode >= 0 || selecNode < MAX_CONNECTED_NODES)
                    {
                        int p = selecNode;
                        prepareTransmissionOfTemperature(slipArrayToSend, byteMacOrigin, routeTable[p].mac, ethernet, frame, 2);
                        setupTransmissionPort(routeTable[p].port);
                        startTransmission((char *)"SENDING_TELEMETRY");
                    }
                }
                else
                {
                    printf("Timeout...\n");
                    delay(500);
                }
            }
        }
        else if (option == 2)
        {
            exit(0); // program exit
        }
        if (transmissionStartedSend && sendingTelemetry)
        {
            while (transmissionStartedSend)
            {
                clearScreen();
                printf("Sending data... %d bytes\n", nbytesSend);
                printf("Transmission type: %s\n", sendTypeTransmission);
                delay(1000);
            }
            // RESET ARRAY TO SEND
            memset(slipArrayToSend, 0, sizeof(slipArrayToSend));
        }

        checkReceivedTransmission(); // check bytes received process

        broadcast(); // broadcast process

        fflush(stdin);
        option = 0;

        delay(100);
    }
}

void checkReceivedTransmission()
{
    // MESSAGE RECEIVED FROM PINOUT_1
    if (boolReceivedFrame)
    {
        error = getFrameFromTransmission(slipArrayReceived, receivedFrame, receivedEthernet);
        if (error)
        {
            printf("----- AN ERROR WAS DETECTED WITH FCS ----- \n");
            printf("-----    IGNORING COMPLETE MESSAGE   ----- \n");
            delay(5000);
        }

        bool isForMe = compareMacAddress(receivedEthernet.destiny, byteMacOrigin);
        bool isBroadcast = compareMacAddress(receivedEthernet.destiny, byteMacBroadcast);

        // check if message is a broadcast and has the special cmd = 5
        if (isBroadcast && receivedFrame.cmd == 5)
        {
            // check if mac is already registered, returns -1 if not
            int p = getPosOnRouteTable(routeTable, MAX_CONNECTED_NODES, receivedEthernet.source);
            if (p == -1)
            {
                // We add node to RouteTable
                Node node;
                memcpy(node.mac, receivedEthernet.source, 6);
                node.ttl = receivedFrame.ttl;
                if (receivedFrame.ttl == 2)
                {
                    // if ttl = 2, it means that broadcast was sent by node at PINOUT_1
                    node.port = 1;
                }
                else
                {
                    // if ttl = 1, it means that broadcast was sent by node at PINOUT_2
                    node.port = 2;
                }
                addNodeToRouteTable(routeTable, MAX_CONNECTED_NODES, node);
            }
            // if broadcast has ttl=2 means we need to re-send it to another node packaging frame with ttl-1
            if (receivedFrame.ttl > 1)
            {
                prepareBroadcast(slipArrayToSend, receivedEthernet.source, receivedEthernet.destiny, ethernet, frame, receivedFrame.ttl - 1);
                setupTransmissionPort(2);
                startTransmission((char *)"RE-BROADCAST-1");
            }
            return;
        }

        if (isForMe)
        {
            if (receivedFrame.cmd == 1)
            {
                int temp = 0;
                int timestamp = 0;
                // Gets telemetry data from a frame with cmd = 1
                getValuesFromTemperatureFrame(receivedFrame, &temp, &timestamp);
                printf("----- RECEIVED TELEMETRY ----- \n");
                printf("Temperature: %.2f \n", ((float)temp - 10000) / 1000);
                printf("Timestamp: %d \n", timestamp);
                delay(5000);
            }
            else
            {
                // if cmd has another value, probably is an undetected error
                printf("Received an unknown cmd, probably an error...\n");
                delay(5000);
            }
        }
        else
        {
            // Programs are designed to work with only messages of Telemetry or Broadcast
            // So check if ttl is > 1 to re-send Telemetry to another node
            if (receivedFrame.ttl > 1)
            {
                memcpy(&ethernet, &receivedEthernet, sizeof(ethernet));
                memcpy(&frame, &receivedFrame, sizeof(frame));
                prepareReTransmissionOfTemperature(slipArrayToSend, ethernet, frame, frame.ttl - 1);
                setupTransmissionPort(2);
                startTransmission((char *)"RE-TRANSMISSION-TEMP-1");
            }
        }
        boolReceivedFrame = false;
    }
    // MESSAGE RECEIVED FROM PINOUT_2
    if (boolReceivedFrame2)
    {
        error2 = getFrameFromTransmission(slipArrayReceived2, receivedFrame2, receivedEthernet2);
        if (error2)
        {
            printf("----- AN ERROR WAS DETECTED WITH FCS ----- \n");
            printf("-----    IGNORING COMPLETE MESSAGE   ----- \n");
            delay(5000);
        }

        bool isForMe = compareMacAddress(receivedEthernet2.destiny, byteMacOrigin);
        bool isBroadcast = compareMacAddress(receivedEthernet2.destiny, byteMacBroadcast);

        if (isBroadcast && receivedFrame2.cmd == 5)
        {
            int p = getPosOnRouteTable(routeTable, MAX_CONNECTED_NODES, receivedEthernet2.source);
            if (p == -1)
            {
                // We add node to RouteTable
                Node node;
                memcpy(node.mac, receivedEthernet2.source, 6);
                node.ttl = receivedFrame2.ttl;
                if (receivedFrame2.ttl == 2)
                {
                    // if ttl = 2, it means that broadcast was sent by node at PINOUT_2
                    node.port = 2;
                }
                else
                {
                    // if ttl = 1, it means that broadcast was sent by node at PINOUT_1
                    node.port = 1;
                }
                addNodeToRouteTable(routeTable, MAX_CONNECTED_NODES, node);
            }
            if (receivedFrame2.ttl > 1)
            {
                prepareBroadcast(slipArrayToSend, receivedEthernet2.source, receivedEthernet2.destiny, ethernet, frame, receivedFrame2.ttl - 1);
                setupTransmissionPort(1);
                startTransmission((char *)"RE-BROADCAST-2");
            }
        }
        return;

        if (isForMe)
        {
            if (receivedFrame2.cmd == 1)
            {
                int temp = 0;
                int timestamp = 0;
                // Gets telemetry data from a frame with cmd = 1
                getValuesFromTemperatureFrame(receivedFrame2, &temp, &timestamp);
                printf("----- RECEIVED TELEMETRY ----- \n");
                printf("Temperature: %.2f \n", ((float)temp - 10000) / 1000);
                printf("Timestamp: %d \n", timestamp);
                delay(5000);
            }
            else
            {
                // if cmd has another value, probably is an undetected error
                printf("Received an unknown cmd, probably an error...\n");
                delay(5000);
            }
        }
        else
        {
            if (receivedFrame2.ttl > 1)
            {
                memcpy(&ethernet, &receivedEthernet2, sizeof(ethernet));
                memcpy(&frame, &receivedFrame2, sizeof(frame));
                prepareReTransmissionOfTemperature(slipArrayToSend, ethernet, frame, frame.ttl - 1);
                setupTransmissionPort(1);
                startTransmission((char *)"RE-TRANSMISSION-TEMP-2");
            }
        }
        boolReceivedFrame2 = false;
    }
    delay(100);
    // for 2
}

void broadcast()
{
    if (transmissionStartedSend)
        return;
    currentTime = time(NULL); // returns unix timestamp in seconds
    //printf("current time %d\n", currentTime);
    //printf("broadcast time %d\n", timeToBroadcast);
    //printf("diff %d\n", currentTime - timeToBroadcast);
    if (currentTime - timeToBroadcast > 0)
    {
        timeToBroadcast = currentTime + 120; // we add 2 minutes to timestamp

        prepareBroadcast(slipArrayToSend, byteMacOrigin, byteMacBroadcast, ethernet, frame, 2);
        setupTransmissionPort(1);
        startTransmission((char *)"BROADCAST");
    }
}

void cb(void)
{
    //printf("run callback\n");
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
                memset(slipArrayToSend, 0, sizeof(slipArrayToSend));
                return;
            }
            nbytesSend++;
        }
    }
    else
    {
        // Channel idle
        digitalWrite(idlePort, 1);
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

void startTransmission(char *msg)
{
    transmissionStartedSend = true;
    setTypeTransmission(msg);
}
void setupTransmissionPort(int port)
{
    if (port == 1)
    {
        transmissionPort = txPin;
        idlePort = txPin2;
    }
    else if (port == 2)
    {
        transmissionPort = txPin2;
        idlePort = txPin;
    }
}
// debug purpose
void setTypeTransmission(char *msg)
{
    memcpy(sendTypeTransmission, msg, sizeof(sendTypeTransmission));
}