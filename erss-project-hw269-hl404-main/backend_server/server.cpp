#include "server.hpp"

Server::Server(char * port) {
  struct addrinfo hints;
  struct addrinfo *servinfo;
  struct addrinfo *p;
  int status;
  int yes=1;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  if ((status = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    exit(1);
  }
   // 以循环找出全部的结果，并绑定（bind）到第一个能用的结果
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype,
      p->ai_protocol)) == -1) {
      perror("server: socket");
      continue;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
        sizeof(int)) == -1) {
      perror("setsockopt");
      exit(1);
    }

    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("server: bind");
      continue;
    }

    break;
  }

  if (p == NULL) {
    fprintf(stderr, "server: failed to bind\n");
  }
  listen(sockfd, BACKLOG);
  freeaddrinfo(servinfo); // 全部都用这个 structure
  
}

int Server::to_accept() {
  struct sockaddr_storage their_addr;
  int new_fd;
  socklen_t addr_size = sizeof(their_addr);
  new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
  return new_fd;
}

