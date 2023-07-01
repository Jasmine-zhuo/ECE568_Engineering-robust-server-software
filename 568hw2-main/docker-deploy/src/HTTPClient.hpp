#ifndef __HTTP_CLIENT__
#define __HTTP_CLIENT__
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <functional>
#include <iostream>
#include <unordered_map>

#include "HTTPProtocol.hpp"
#include "Utils.hpp"

using namespace std;

class HTTPClient {
 private:
  int sockfd;
  string hostName;
  int port;

 public:
  // contructor with HTTPRequest, store the hostName and port Number;
  HTTPClient(HTTPRequest request);
  HTTPResponse send(HTTPRequest request);
  ~HTTPClient();
  // close the socket
  void closeClient();
  bool isClosed();
  int getSocketFD();
};

#endif
