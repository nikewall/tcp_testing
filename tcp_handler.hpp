#include <iostream>
#include <cstring>
#include <system_error>
#include <unistd.h> //close

#include <sys/types.h> //size_t
#include <netinet/in.h> //sockaddr_in
#include <sys/socket.h>
#include <sys/epoll.h>

void tcp_handler(const std::string& ip_addr) {
  // specifies ipv4 (INET) and TCP (STREAM)
  int sockfd = socket(AF_INET, SOCK_STREAM, 0); 
  if (sockfd < 0) {
     throw new std::system_error(errno, std::generic_category(), "Failed to create socket");
  }

  // IPv4 setup
  struct sockaddr_in server_addr = {};
  server_addr.sin_family = AF_INET; // ipv4
  server_addr.sin_port = htons(8080); // port num

  // bind socket to addr
  if (bind(sockfd, reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
    throw new std::system_error(errno, std::generic_category(), "Failed to bind sock");
  }

  // listen on socket (wait for conn)
  if (listen(sockfd, 1) < 0) {
    throw new std::system_error(errno, std::generic_category(), "Failed to listen on socket");
  }
  std::cout << "Waiting for connection..." << std::endl;


  // we finally get connection, accept it 
  struct sockaddr client_addr;
  socklen_t client_len = sizeof(client_addr);
  int client_sockfd = accept(sockfd, &client_addr, &client_len);
  if (client_sockfd < 0) {
    throw new std::system_error(errno, std::generic_category(), "Failed to accept connection");
  }
  std::cout << "Client connected!" << std::endl;


  int epollfd = epoll_create1(0);
  if (epollfd == -1)
    throw new std::system_error(errno, std::generic_category(), "Failed to create epoll inst");
  
  struct epoll_event ev = {};
  ev.events = EPOLLIN;
  ev.data.fd = client_sockfd;
  // tell epoll to monitor client
  if (epoll_ctl(epollfd, EPOLL_CTL_ADD, client_sockfd, &ev) == -1 ) { 
    throw new std::system_error(errno, std::generic_category(), "Failed to add client sock to epoll");
  }

  bool done = false;
  while (!done) {
    struct epoll_event events[1];
    // wait for event
    // epoll_wait(fd, event list, num events, timeout_ms)
    int nfds = epoll_wait(epollfd, events, 1, -1); 
    if (nfds == -1) {
      if (errno == EINTR) 
        continue;
      throw new std::system_error(errno, std::generic_category(), "epoll_wait error");
    }

    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    ssize_t len = recv(client_sockfd, &buffer, sizeof(buffer), 0); // get data
    if (len <= 0) {
      if (len == 0)
        std::cout << "conn closed by client" << std::endl;
      else
        std::cerr << "recv error " << std::endl;
      done = true;
    } else {
      std::cout << "client said: '" << std::string(buffer, len-1) << "'" << std::endl;
    }
  }

  close(client_sockfd);
  close(epollfd);
  close(sockfd);
}
