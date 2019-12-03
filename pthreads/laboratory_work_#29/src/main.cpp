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
  timeval timevl{};
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
          auto curr_client = server->getClient(client_socket);
          auto req = curr_client->readRequest();
          if (req.second == -1) {
            server->deleteClient(client_socket);
            continue;
          }
#ifdef DEBUG1
          std::cout << "Access to resource: "
                    << req.first->getResource().value_or(
                           "Getting full request...")
                    << std::endl;
#endif
#ifdef DEBUG2
          std::cout << req.first->requestToString().value_or(
                           "Getting full request...")
                    << std::endl;
#endif
          if (req.second == 0) {
            server->addClientResourceSocket(curr_client);
          }
        }
      }
    }
  }
}