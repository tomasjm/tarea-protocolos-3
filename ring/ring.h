struct Node {
  BYTE mac[6];
  int port; // 1 or 2
  int ttl;
  bool init = false;
};

bool macExistsOnRouteTable(Node routeTable[], int maxLen, BYTE mac[6]);

int getPosOnRouteTable(Node routeTable[], int maxLen, BYTE mac[6])