#include <stdio.h>

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

int addNodeToRouteTable(Node routeTable[], int maxLen, Node node) {
  for (int i = 0; i<maxLen; i++) {
    if (routeTable[i].init == false) {
      routeTable[i].init = true;
      memcpy(routeTable[i].mac, node.mac, 6);
      routeTable[i].port = node.port;
      routeTable[i].ttl = node.ttl;
      return i;
    }
  }
  return -1;
}
void printRouteTable(Node routeTable[], int maxLen) {
  printf("id |    mac address    | port\n");
  for (int i = 0; i < maxLen; i++) {
    if (routeTable[i].init == false) continue;
    printf("%d  | ", i);
    printMacAddress(routeTable[i].mac);
    printf(" | %d\n", routeTable[i].port);
  }
}