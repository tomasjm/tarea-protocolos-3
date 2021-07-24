#ifndef HELPERS_H
#define HELPERS_H

#include "../ethernet/ethernet.h"
#include "../protocol/protocol.h"
#include "../slip/slip.h"

#define BYTE unsigned char

void cDelay(int milliseconds);

void readSensorData(int quantity, int valuesArr[], int timesArr[]);

void getByteArrayOfInteger(int v, BYTE arr[]); 

void getIntegerOfByteArray(BYTE arr[], int *v_ptr);

void prepareTransmissionOfTemperature(BYTE slipArray[], BYTE byteMacSource[6], BYTE byteMacDestiny[6], Ethernet &ethf, Frame &f);

void prepareTransmissionOfTextMessage(BYTE slipArray[], BYTE byteMacSource[6], BYTE byteMacDestiny[6], Ethernet &ethf, Frame &f);

void prepareBroadcast(BYTE slipArray[], BYTE byteMacSource[6], BYTE byteMacDestiny[6], Ethernet &ethf, Frame &f, int ttl);

void getTextMessage(char msg[], int length);

void getValuesFromTemperatureFrame(Frame &f, int *vTemp, int *vTimestamp);

void getMessageFromTextMessageFrame(Frame &f, char msg[]);

void generateRawEthernet(Ethernet &ethf, Frame &f, BYTE byteMacSource[6], BYTE byteMacDestiny[6]);
void generateRawFrame(Frame &f, int cmd, int sa, int length, BYTE data[]);

bool getFrameFromTransmission(BYTE slipArray[], Frame &f, Ethernet &ef);

void printByteArray(BYTE* arr, int len);
#endif