#include "client.hpp"

Client::Client (const char * addr, const char * port) {
  struct addrinfo hints;
  
  // 首先，用 getaddrinfo() 载入 address structs：
  
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  
  getaddrinfo(addr, port, &hints, &res);
  
  // 建立一个 socket：
 
  sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  
}

int Client::to_connect() {
  return connect(sockfd, res->ai_addr, res->ai_addrlen);
}

int Client::get_sockfd() {
  return sockfd;
  
}
Client::~Client() {
  freeaddrinfo(res);
}
