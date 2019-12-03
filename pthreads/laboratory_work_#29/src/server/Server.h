#pragma once

#include "../client/Client.h"
#include <algorithm>
#include <fcntl.h>
#include <map>
#include <memory>
#include <netinet/in.h>
#include <optional>
#include <stdexcept>
#include <system_error>
#include <unistd.h>

class Server {
private:
  static std::shared_ptr<Server> instance;
  int socket;
  sockaddr_in address{};
  static const size_t SOCKADDR_IN_LEN = sizeof(address);
  std::map<int, std::shared_ptr<Client>> clients;
  std::vector<int> client_sockets;
  std::vector<int> resource_sockets;

  explicit Server(const in_port_t &port);

public:
  static std::shared_ptr<Server> &getInstance(const in_port_t &port);

  int accept();

  [[nodiscard]] inline int getSocket() const { return socket; }

  [[nodiscard]] inline std::shared_ptr<Client> &getClient(const int &socket) {
    return clients[socket];
  }

  void deleteClient(const int &socket);

  void deleteClientResourceSocket(const int &socket);

  [[nodiscard]] inline std::vector<int> getClientSockets() const {
    return client_sockets;
  }

  void addClientResourceSocket(const std::shared_ptr<Client> &client);

  [[nodiscard]] inline std::vector<int> getClientResourceSockets() const {
    return resource_sockets;
  }

  [[nodiscard]] int getMaxClientSocket() const;

  [[nodiscard]] fd_set getFdSet() const;

  ~Server() { close(socket); }
};