#include "ring.h"
#include "../ethernet/ethernet.h"
#define BYTE unsigned char


bool macExistsOnRouteTable(Node routeTable[], int maxLen, BYTE mac[6]) {
  for (int i = 0; i<maxLen; i++) {
    if (routeTable[i].init == false) continue;
    if (compareMacAddress(routeTable[i].mac, mac)) return true;
  }
  return false;
}

int getPosOnRouteTable(Node routeTable[], int maxLen, BYTE mac[6]) {
  for (int i = 0; i<maxLen; i++) {
    if (routeTable[i].init == false) continue;
    if (compareMacAddress(routeTable[i].mac, mac)) return i;
  }
  return -1;
}