#pragma once

#include "../client/Client.h"
#include <algorithm>
#include <map>
#include <poll.h>
#include <stdexcept>

enum SocketType {
  GET_REQUEST,
  SEND_REQUEST,
  GET_RESOURCE,
  SEND_RESOURCE,
  GET_RESPONSE,
  CONNECT,
  NONE
};

struct Socket {
  int socket;
  SocketType type;
  Socket(const int &socket, const SocketType &type)
      : socket(socket), type(type) {}
  explicit Socket(const int &socket) : socket(socket), type(NONE) {}
};

struct compare {
  bool operator()(const Socket &l, const Socket &r) const {
    return l.socket < r.socket;
  }
};

class Server {
private:
  static Server *instance;
  int server_socket;
  sockaddr_in address;

  std::map<Socket, Client *, compare> clients;
  std::map<std::string, Data *> cache;

  explicit Server(const in_port_t &port);

  int accept();

  bool addClientResourceSocket(const int &socket);

  bool changeSocketType(const int &socket, const SocketType &new_type);

  Data *getCachedResource(const std::string &url) const;

  Client *deleteClient(const int &socket);

public:
  static Server *getInstance(const in_port_t &port);

  int getSocketsTasks(fd_set &w, fd_set &r,
                      fd_set &e); // sockets tasks for select

  inline const std::map<Socket, Client *, compare> getClients() const {
    return clients;
  }

  inline int getServerSocket() const { return server_socket; }

  int execClientAction(
      const int &socket); // execute client's action depend on socket type

  void updateCache(); // delete bad cache entries

  ~Server();
};