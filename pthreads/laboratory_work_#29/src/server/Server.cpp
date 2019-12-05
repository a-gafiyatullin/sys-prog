#include "Server.h"

std::shared_ptr<Server> Server::instance = nullptr;

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
  if (bind(socket, (sockaddr *)&address, sizeof(sockaddr_in)) < 0) {
    throw std::system_error(std::error_code(errno, std::system_category()),
                            "Server::cannot bind port!");
  }
  if (::listen(socket, SOMAXCONN) < 0) {
    throw std::system_error(std::error_code(errno, std::system_category()),
                            "Server::cannot listen socket!");
  }
  if (fcntl(socket, F_SETFL, O_NONBLOCK) < 0) {
    throw std::system_error(
        std::error_code(errno, std::system_category()),
        "Server::cannot set socket in to the non-blocking mode!");
  }
}

std::shared_ptr<Server> &Server::getInstance(const in_port_t &port) {
  if (instance == nullptr) {
    instance.reset(new Server(port));
  }

  return instance;
}

int Server::accept() {
  sockaddr_in remote{};
  socklen_t sockaddr_in_len = sizeof(sockaddr_in);
  int client_socket;
  if ((client_socket =
           ::accept(socket, (sockaddr *)&remote, &sockaddr_in_len)) < 0) {
    return client_socket;
  }

  clients.insert(
      std::make_pair(client_socket, std::make_shared<Client>(client_socket)));
  client_sockets.push_back(client_socket);

  return client_socket;
}

fd_set Server::getFdSet() const {
  fd_set sockets;
  FD_ZERO(&sockets);
  for (auto client_socket : getClientSockets()) {
    FD_SET(client_socket, &sockets);
  }
  for (auto resource_socket : getClientResourceSockets()) {
    FD_SET(resource_socket, &sockets);
  }

  return sockets;
}

void Server::deleteClientSockets(const int &socket) {
  auto client = clients.find(socket);
  int client_socket = 0;
  int resource_socket = 0;
  if (client != clients.end()) {
    clients.erase(client);
    if (client->second->getSocket() == socket) {
      client_socket = socket;
      resource_socket = client->second->getResourceSocket();
      auto resource = clients.find(resource_socket);
      if (resource != clients.end()) {
        clients.erase(resource);
      }
    } else {
      resource_socket = socket;
      client_socket = client->second->getSocket();
      auto main = clients.find(client_socket);
      if (main != clients.end()) {
        clients.erase(main);
      }
    }
  } else {
    return;
  }
  auto client_socket_iter =
      find(client_sockets.begin(), client_sockets.end(), client_socket);
  if (client_socket_iter != client_sockets.end()) {
    client_sockets.erase(client_socket_iter);
  }
  auto resource_socket_iter =
      find(resource_sockets.begin(), resource_sockets.end(), resource_socket);
  if (resource_socket_iter != resource_sockets.end()) {
    resource_sockets.erase(resource_socket_iter);
  }
}

int Server::getMaxClientSocket() const {
  auto client_socket =
      std::max_element(client_sockets.begin(), client_sockets.end());
  auto resource_socket =
      std::max_element(resource_sockets.begin(), resource_sockets.end());
  if (client_socket == client_sockets.end() &&
      resource_socket == resource_sockets.end()) {
    return 0;
  } else if (client_socket != client_sockets.end() &&
             resource_socket == resource_sockets.end()) {
    return *client_socket;
  } else if (client_socket == client_sockets.end() &&
             resource_socket != resource_sockets.end()) {
    return *resource_socket;
  } else {
    return std::max(*client_socket, *resource_socket);
  }
}

void Server::addClientResourceSocket(const std::shared_ptr<Client> &client) {
  clients.insert(std::make_pair(client->getResourceSocket(), client));
  resource_sockets.push_back(client->getResourceSocket());
}
