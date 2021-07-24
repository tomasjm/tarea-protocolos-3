#include <time.h>
#include <stdio.h>
#include "helpers.h"
#include "../ethernet/ethernet.h"
#include "../protocol/protocol.h"
#include "../slip/slip.h"

#define BYTE unsigned char

void cDelay(int milliSeconds) {
  clock_t startTime = clock()/CLOCKS_PER_SEC;
  while (clock()/CLOCKS_PER_SEC < startTime + milliSeconds/1000);
}

void readSensorData(int q, int valuesArr[], int timesArr[]) {
  FILE *fp;
  int rawTemp;

  fp = popen("cat /sys/bus/w1/devices/28-0113126a6baa/w1_slave | grep -i -o \"t=[0-9]*\" | grep -o \"[0-9]*\"", "r");
  fscanf(fp, "%d", &rawTemp);
  valuesArr[0] = rawTemp+10000;
  timesArr[0] = time(NULL);
  pclose(fp);
  
}

void getByteArrayOfInteger(int v, BYTE arr[]) {
  int l = sizeof(v);
  for (int i = 0; i<l; i++) {
    arr[i] = (v >> (8*i)) & 0xFF;
  }
}

void getIntegerOfByteArray(BYTE arr[], int *vPtr) {
  for (int i = 0; i< sizeof(int); i++) {
    *vPtr |= (arr[i] << (i*8));
  }
}

// This function will package all required variables to get the final SlipArray to use in the transmission
void prepareTransmissionOfTemperature(BYTE slipArray[], BYTE byteMacSource[6], BYTE byteMacDestiny[6], Ethernet &ethf, Frame &f) {
  int tempValue[1];
  int timeValue[1];
  readSensorData(1, tempValue, timeValue);
  BYTE byteTempValue[4];
  BYTE byteTimeValue[4];
  BYTE dataFrameToSend[8];
  getByteArrayOfInteger(tempValue[0], byteTempValue);
  getByteArrayOfInteger(timeValue[0], byteTimeValue);
  for (int i = 0; i<4; i++) {
    dataFrameToSend[i] = byteTempValue[i];
    dataFrameToSend[i+4] = byteTimeValue[i];
  }
  generateRawFrame(f, 1, 0, 8, dataFrameToSend); // Gets frame to send Telemetry
  generateRawEthernet(ethf, f, byteMacSource, byteMacDestiny); // Gets Ethernet to send Frame of Telemetry
  packSlip(slipArray, ethf.frame, 28); // Gets SlipArray of the Ethernet to send Frame of Telemetry
}

// This function is similar to prepareTransmissionOfTemperature but for sending a TextMessage of 30 characters
void prepareTransmissionOfTextMessage(BYTE slipArray[], BYTE byteMacSource[6], BYTE byteMacDestiny[6], Ethernet &ethf, Frame &f) {
  BYTE msg[30];
  printf("Enter message of 30 characters to send: \n");
  getTextMessage((char*)msg, 30);
  generateRawFrame(f, 2, 0, 30, msg);
  generateRawEthernet(ethf, f, byteMacSource, byteMacDestiny);
  packSlip(slipArray, ethf.frame, 50);
}

void prepareBroadcast(BYTE slipArray[], BYTE byteMacSource[6], BYTE byteMacDestiny[6], Ethernet &ethf, Frame &f, int ttl) {
  BYTE data[1] = {0};
  generateRawFrame(f, 5, ttl, 0, data);
  generateRawEthernet(ethf, f, byteMacSource, byteMacDestiny);
  packSlip(slipArray, ethf.frame, 20);
}

// Function to get a TextMessage with blank spaces
void getTextMessage(char msg[], int length) {
  char c;
  while ((c = getchar()) != '\n' && c != EOF); //clears buffer to use fgets()
  fgets(msg, length, stdin);
}

// If we have a temperature frame (identified by it cmd) we can retrieve its values with the function
void getValuesFromTemperatureFrame(Frame &f, int *vTemp, int *vTimestamp) {
  BYTE temp[4];
  BYTE timestamp[4];
  for (int i = 0; i<4; i++) {
    temp[i] = f.data[i];
    timestamp[i] = f.data[i+4];
  }
  getIntegerOfByteArray(temp, vTemp);
  getIntegerOfByteArray(timestamp, vTimestamp);
}
// Same as getValuesFromTemperatureFrame but for textmessage
void getMessageFromTextMessageFrame(Frame &f, char msg[]) {
  for (int i = 0; i<30; i++) {
    msg[i] = f.data[i];
  }
}

// Generates EthernetFrame from the custom Frame and some Mac Addresses
void generateRawEthernet(Ethernet &ethf, Frame &f, BYTE byteMacSource[6], BYTE byteMacDestiny[6]) {
  //BYTE macSource[6], macDestiny[6];
  //convertMacAddressToByteArray(strMacSource, macSource);
  //convertMacAddressToByteArray(strMacDestiny, macDestiny);
  memcpy(ethf.source, byteMacSource, sizeof(ethf.source));
  memcpy(ethf.destiny, byteMacDestiny, sizeof(ethf.destiny));
  ethf.length = 2+f.length;
  memcpy(ethf.data, f.frame, sizeof(ethf.data));
  packEthernet(ethf);
}
// Generates Custom Frame from a data array of bytes
void generateRawFrame(Frame &f, int cmd, int ttl, int length, BYTE data[]) {
  f.cmd = cmd;
  f.ttl = ttl;
  f.length = length;
  memcpy(f.data, data, length);
  generateFrameToSend(f);
}

// It gets final Custom Frame from a received SlipArray and returns a bool if it detects a FCS Error
bool getFrameFromTransmission(BYTE slipArray[], Frame &f, Ethernet &ef) {
  int n = unpackSlip(ef.frame, slipArray);
  bool error = unpackEthernet(ef);
  if (error) {
    return true;
  }
  memcpy(f.frame, ef.data, sizeof(f.frame));
  generateReceivedFrame(f);
  return false;
}

void printByteArray(BYTE* arr, int len){
  for(int i = 0; i<len; i++){
    printf("0x%x ", arr[i]);
  }
  printf("\n");
}