#ifndef RING_H
#define RING_H
#define BYTE unsigned char

struct Node {
  BYTE mac[6];
  int port; // 1 or 2
  int ttl;
  bool init = false;
};

// check if mac is already registered
bool macExistsOnRouteTable(Node routeTable[], int maxLen, BYTE mac[6]);

// it returns array relative position (from 0-inf) of a mac address in route table
int getPosOnRouteTable(Node routeTable[], int maxLen, BYTE mac[6]);

// add a node to the route table
int addNodeToRouteTable(Node routeTable[], int maxLen, Node node);

// it print all information of registered nodes in a route table
void printRouteTable(Node routeTable[], int maxLen);
#endif