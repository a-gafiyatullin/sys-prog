#include "Server.h"

Server::Server(const in_port_t &port)
    : socket(::socket(AF_INET, SOCK_STREAM, 0)) {
  if (socket < 0) {
    throw std::out_of_range("Server::socket < 0!");
  }
  if (port < 1024) {
    throw std::out_of_range("Server::port < 1024!");
  }

  address.sin_family = AF_INET;
  address.sin_port = htons(port);
  address.sin_addr.s_addr = INADDR_ANY;
  if (bind(socket, (sockaddr *)&address, SOCKADDR_IN_LEN) < 0) {
    throw std::system_error(std::error_code(errno, std::system_category()),
                            "Server: cannot bind port!");
  }
  if (::listen(socket, SOMAXCONN) < 0) {
    throw std::system_error(std::error_code(errno, std::system_category()),
                            "Server: cannot listen socket!");
  }
  if (fcntl(socket, F_SETFL, O_NONBLOCK) < 0) {
    throw std::system_error(
        std::error_code(errno, std::system_category()),
        "Server: cannot set socket in to the non-blocking mode!");
  }
}

Server Server::getInstance(const in_port_t &port) {
  return (instance == nullptr ? *(instance = new Server(port)) : *instance);
}

std::optional<sockaddr_in> Server::listen() {
  sockaddr_in remote{};
  socklen_t sockaddr_in_len = SOCKADDR_IN_LEN;
  if(accept(socket, (sockaddr *)&remote, &sockaddr_in_len) < 0) {
    return {};
  }

  return remote;
}
