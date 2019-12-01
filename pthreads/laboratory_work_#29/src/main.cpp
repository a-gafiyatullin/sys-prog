#include "server/Server.h"
#include <iostream>
#include <sys/select.h>

int main() {
  in_port_t port = 8888;

  auto server = Server::getInstance(8888);
  timeval timevl;
  timevl.tv_sec = 0;
  timevl.tv_usec = 0;
  while (true) {
    int client;
    if ((client = server->accept()) < 0) {
      continue;
    }
    fd_set sockets;
    FD_ZERO(&sockets);
    for (auto client_socket : server->getClientSockets()) {
      FD_SET(client_socket, &sockets);
    }
    if (select(server->getMaxClientSocket(), &sockets, nullptr, nullptr,
               &timevl) > 0) {
      for (auto client_socket : server->getClientSockets()) {
        if (FD_ISSET(client_socket, &sockets)) {
          auto req = server->getClient(client_socket)->readMsg();
          if (req.second == 0) {
            std::cout << req.first->getMinorVersion() << std::endl;
            std::cout << req.first->getMethod().value_or(" ") << std::endl;
            std::cout << req.first->getPath().value_or(" ") << std::endl;
            for (size_t i = 0; i != req.first->getHeaders().second; ++i) {
              printf("%.*s: %.*s\n",
                     (int)req.first->getHeaders().first[i].name_len,
                     req.first->getHeaders().first[i].name,
                     (int)req.first->getHeaders().first[i].value_len,
                     req.first->getHeaders().first[i].value);
            }
          }
        }
      }
    }
  }
}