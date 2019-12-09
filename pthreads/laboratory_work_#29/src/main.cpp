#include "server/Server.h"
#include <csignal>
#include <getopt.h>
#include <iostream>

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
  while ((c = getopt(argc, argv, "p:")) != EOF) { // get server port number
    switch (c) {
    case 'p':
      port = atoi(optarg);
      break;
    default:
      std::cerr << error_msg << std::endl;
      return -1;
    }
  }

  signal(SIGINT, sig_stop); // use SIGINT to finish program in the good way
  struct sigaction act;
  act.sa_handler = SIG_IGN;
  sigemptyset(&act.sa_mask);
  sigaction(SIGPIPE, &act, NULL); // ignore SIGPIPE from sockets

  Server *server = Server::getInstance(port);

  std::cout << "http-proxy started at port " << port << std::endl;
  while (!stop) {
    std::pair<pollfd *, size_t> socket_set =
        server->getSocketsTasks(); // get commands for sockets
    if (poll(socket_set.first, socket_set.second, -1) < 0) {
      continue;
    }
    for (size_t i = 0; i < socket_set.second; i++) {
      if (socket_set.first[i].revents == 0) {
        continue;
      }
      if (socket_set.first[i].revents != 0) {
        server->execClientAction(socket_set.first[i].fd);
      }
    }
    server->updateCache();
    delete socket_set.first; // delete previous commands array
  }

  delete server; // free server structures

  return 0;
}