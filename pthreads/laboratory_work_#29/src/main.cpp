#include "server/Server.h"
#include <csignal>
#include <getopt.h>
#include <iostream>
#include <sys/select.h>

bool stop = false;

void sig_stop(int) { stop = true; }

int main(int argc, char *argv[]) {
  std::string error_msg = "usage: http-proxy [-p port-number]";
  if (argc < 2) {
    std::cerr << error_msg << std::endl;
    return -1;
  }

  in_port_t port = 0;
  char c;
  while ((c = getopt(argc, argv, "p:")) != EOF) {
    switch (c) {
    case 'p':
      port = atoi(optarg);
      break;
    default:
      std::cerr << error_msg << std::endl;
      return -1;
    }
  }

  signal(SIGINT, sig_stop);

  std::cout << "http-proxy started at port " << port << std::endl;
  auto server = Server::getInstance(port);
  timeval timevl;
  timevl.tv_sec = 0;
  timevl.tv_usec = 0;

  while (!stop) {
    if (server->accept() < 0) {
      continue;
    }
    auto sockets = server->getFdSet();
    if (select(server->getMaxClientSocket(), &sockets, nullptr, nullptr,
               &timevl) > 0) {
      for (auto client_socket : server->getClientSockets()) {
        if (FD_ISSET(client_socket, &sockets)) {
          auto req = server->getClient(client_socket)->readMsg();
#ifdef DEBUG
          if (req.second == 0) {
            std::cout << "Access to resource: "
                      << req.first->getPath().value_or("") << std::endl;
          } else {
            std::cout << "Getting full request..." << std::endl;
          }
#endif
        }
      }
    }
  }
}