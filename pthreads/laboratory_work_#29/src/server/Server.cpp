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
  request_sockets.push_back(client_socket);

  return client_socket;
}

fd_set Server::getFdSet() const {
  fd_set sockets;
  FD_ZERO(&sockets);
  for (auto request_socket : getClientRequestSockets()) {
    FD_SET(request_socket, &sockets);
  }
  for (auto resource_socket : getClientResourceSockets()) {
    FD_SET(resource_socket, &sockets);
  }

  return sockets;
}

void Server::deleteClient(const int &socket) {
  auto client = clients.find(socket);
  int request_socket = 0;
  int resource_socket = 0;
  if (client != clients.end()) {
    clients.erase(client);
    if (client->second->getSocket() == socket) {
      request_socket = socket;
      resource_socket = client->second->getResourceSocket();
      auto resource = clients.find(resource_socket);
      if (resource != clients.end()) {
        clients.erase(resource);
      }
    } else {
      resource_socket = socket;
      request_socket = client->second->getSocket();
      auto main = clients.find(request_socket);
      if (main != clients.end()) {
        clients.erase(main);
      }
    }
  } else {
    return;
  }
  auto request_socket_iter =
      find(request_sockets.begin(), request_sockets.end(), request_socket);
  if (request_socket_iter != request_sockets.end()) {
    request_sockets.erase(request_socket_iter);
  }
  auto resource_socket_iter =
      find(resource_sockets.begin(), resource_sockets.end(), resource_socket);
  if (resource_socket_iter != resource_sockets.end()) {
    resource_sockets.erase(resource_socket_iter);
  }
}

int Server::getMaxClientSocket() const {
  auto request_socket =
      std::max_element(request_sockets.begin(), request_sockets.end());
  auto resource_socket =
      std::max_element(resource_sockets.begin(), resource_sockets.end());

  int max_request_socket =
      (request_socket == request_sockets.end() ? 0 : *request_socket);
  int max_resource_socket =
      (resource_socket == resource_sockets.end() ? 0 : *resource_socket);

  return std::max(max_resource_socket, max_request_socket);
}

void Server::addClientResourceSocket(const std::shared_ptr<Client> &client) {
  clients.insert(std::make_pair(client->getResourceSocket(), client));
  resource_sockets.push_back(client->getResourceSocket());
}

std::shared_ptr<Data> Server::getCachedResource(const std::string &url) const {
  auto resource = cache.find(url);
  if (resource == cache.end()) {
    return nullptr;
  }

  return resource->second;
}

void Server::sendCachedDataToClients() const {
  for (const auto &data : cache) {
    data.second->sendDataToClients();
  }
}
