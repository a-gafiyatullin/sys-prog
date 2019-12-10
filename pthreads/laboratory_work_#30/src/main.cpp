#include "server/Server.h"
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

  // use SIGINT to finish program in the good way
  struct sigaction act;
  act.sa_handler = sig_stop;
  sigemptyset(&act.sa_mask);
  act.sa_flags = ~SA_RESTART;
  sigaction(SIGINT, &act, NULL);
  // ignore SIGPIPE from sockets
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGPIPE);
  if (pthread_sigmask(SIG_BLOCK, &mask, NULL) < 0) {
    throw std::domain_error("Server::main: can't block SIGPIPE!");
  }

  Server *server = Server::getInstance(port);
  std::cout << "http-proxy started at port " << port << std::endl;

  while (!stop) {
    server->accept();
  }

  std::cout << "Finishing server..." << std::endl;
  delete server; // free server structures
  std::cout << "Server is finished" << std::endl;

  return 0;
}