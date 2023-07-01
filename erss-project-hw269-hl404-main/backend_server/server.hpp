#ifndef _SERVER_H__
#define _SERVER_H__

#include "Common.hpp"

#define BACKLOG 10


class Server {

private:
  int sockfd;  
    
public:
  Server(char * port);
  int to_accept();    
};
#endif
