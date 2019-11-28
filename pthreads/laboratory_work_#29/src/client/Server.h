#pragma once

#include <fcntl.h>
#include <memory>
#include <netinet/in.h>
#include <optional>
#include <stdexcept>
#include <system_error>

class Server {
private:
  static Server *instance;
  int socket;
  sockaddr_in address{};
  static const size_t SOCKADDR_IN_LEN = sizeof(address);

  explicit Server(const in_port_t &port);

public:
  static Server getInstance(const in_port_t &port);

  std::optional<sockaddr_in> listen();
};

Server *Server::instance = nullptr;