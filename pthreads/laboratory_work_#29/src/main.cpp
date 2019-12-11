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

  // use SIGINT to finish program in the good way
  struct sigaction int_act;
  int_act.sa_handler = sig_stop;
  sigemptyset(&int_act.sa_mask);
  int_act.sa_flags = ~SA_RESTART;
  sigaction(SIGINT, &int_act, NULL);
  // ignore SIGPIPE from sockets
  struct sigaction pipe_act;
  pipe_act.sa_handler = SIG_IGN;
  sigemptyset(&pipe_act.sa_mask);
  sigaction(SIGPIPE, &pipe_act, NULL);

  Server *server = Server::getInstance(port);

  std::cout << "http-proxy started at port " << port << std::endl;
  int server_socket = server->getServerSocket();
  while (!stop) {
    fd_set r, w, e;
    int max_socket =
        server->getSocketsTasks(w, r, e); // get commands for sockets
    if (select(max_socket + 1, &r, &w, &e, NULL) <= 0) {
      continue;
    }
    if (FD_ISSET(server_socket, &r)) {
      server->execClientAction(server_socket);
    }

    std::map<Socket, Client *, compare> sockets = server->getClients();
    for (std::map<Socket, Client *>::iterator iter = sockets.begin();
         iter != sockets.end(); iter++) {
      if ((FD_ISSET(iter->first.socket, &r) &&
           (iter->first.type == GET_REQUEST ||
            iter->first.type == GET_RESPONSE ||
            iter->first.type == GET_RESOURCE)) ||
          (FD_ISSET(iter->first.socket, &w) &&
           (iter->first.type == CONNECT || iter->first.type == SEND_RESOURCE ||
            iter->first.type == SEND_REQUEST))) {
        server->execClientAction(iter->first.socket);
      }
    }
    server->updateCache();
  }

  std::cout << "Finishing server..." << std::endl;
  delete server; // free server structures
  std::cout << "Server is finished" << std::endl;

  return 0;
}