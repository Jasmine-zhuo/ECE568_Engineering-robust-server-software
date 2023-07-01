#include "HTTPClient.hpp"

// contructor with HTTPRequest
HTTPClient::HTTPClient(HTTPRequest request) {
  this->hostName = request.getHostName();
  this->port = request.getPort();
  //initiate socket
  this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
  //if(sockfd>0) {error("ERROR opening socket");}
  //get host name from HTTP request
  struct hostent * server;
  server = gethostbyname(this->hostName.c_str());
  //if (server == NULL) {
  //fprintf(stderr, "ERROR, no such host\n");
  //exit(0);
  //}
  //connect port
  struct sockaddr_in serv_addr;
  int n;
  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(this->port);
  //connect < 0 handle error
  connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  //{error("ERROR connecting");}
}

HTTPClient::~HTTPClient() {
  closeClient();
}
HTTPResponse HTTPClient::send(HTTPRequest request) {
  vector<char> rawData = request.getRawData();
  int n = write(sockfd, rawData.data(), rawData.size());
  //if (n < 0)  error("ERROR writing to socket");
  HTTPResponse response(this->sockfd);
  return response;
}
// close the socket
void HTTPClient::closeClient() {
  close(this->sockfd);
}

bool HTTPClient::isClosed() {
  int error = 0;
  socklen_t len = sizeof(error);
  int retval = getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len);
  return retval != 0;
}

int HTTPClient::getSocketFD() {
  return this->sockfd;
}
