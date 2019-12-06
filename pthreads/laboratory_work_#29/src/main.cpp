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
    server->accept();
    auto sockets = server->getFdSet();
    if (select(server->getMaxClientSocket() + 1, &sockets, nullptr, nullptr,
               &timevl) > 0) {
      for (auto request_socket :
           server->getClientRequestSockets()) { // get requests from clients
        if (FD_ISSET(request_socket, &sockets)) {
          auto curr_client = server->getClient(request_socket);
          auto request = curr_client->readRequest();
          if (request.second == -1) {
            server->deleteClient(request_socket);
            continue;
          }
#ifdef DEBUG
          std::cerr << "Access to resource: "
                    << request.first->getResource().value_or(
                           "Getting full request...")
                    << std::endl;
#endif
          if (request.second != 0) {
            continue;
          }
          if (!request.first->getURL().has_value()) {
            server->deleteClient(request_socket);
            continue;
          }
          auto data =
              server->getCachedResource(request.first->getURL().value());
          if (data == nullptr) {
            auto cache_element = curr_client->sendRequest(request.first);
            if (cache_element != nullptr) {
              server->addCacheElement(request.first->getURL().value(),
                                      cache_element);
              server->addClientResourceSocket(curr_client);
#ifdef DEBUG
              std::cerr << "Client's resource socket is added to reading!"
                        << std::endl;
#endif
            } else {
              server->deleteClient(request_socket);
            }
          } else {
#ifdef DEBUG
            std::cerr << "Add client to receiving cached data" << std::endl;
#endif
            data->addClient(curr_client);
            server->deleteClient(request_socket);
          }
        }
      }
      for (auto resource_socket :
           server->getClientResourceSockets()) { // get data from servers
        if (FD_ISSET(resource_socket, &sockets)) {
          auto curr_client = server->getClient(resource_socket);
          if (!curr_client->proxyingData()) {
            server->deleteClient(resource_socket);
          }
        }
      }
    }
    server->sendCachedDataToClients();
  }
}