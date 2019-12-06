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
  std::map<int, std::shared_ptr<Client>> clients;
  std::vector<int> request_sockets;
  std::vector<int> resource_sockets;

  std::map<std::string, std::shared_ptr<Data>> cache;

  explicit Server(const in_port_t &port);

public:
  static std::shared_ptr<Server> &getInstance(const in_port_t &port);

  int accept();

  [[nodiscard]] inline int getServerSocket() const { return socket; }

  [[nodiscard]] inline std::vector<int> getClientRequestSockets() const {
    return request_sockets;
  }

  [[nodiscard]] inline std::vector<int> getClientResourceSockets() const {
    return resource_sockets;
  }

  [[nodiscard]] inline std::shared_ptr<Client> &getClient(const int &socket) {
    return clients[socket];
  }

  void deleteClient(const int &socket);

  void addClientResourceSocket(const std::shared_ptr<Client> &client);

  [[nodiscard]] int getMaxClientSocket() const;

  [[nodiscard]] fd_set getFdSet() const;

  [[nodiscard]] std::shared_ptr<Data>
  getCachedResource(const std::string &url) const;

  inline void addCacheElement(const std::string &url,
                              const std::shared_ptr<Data> &data) {
    cache.insert(std::make_pair(url, data));
  }

  void sendCachedDataToClients() const;

  ~Server() { close(socket); }
};