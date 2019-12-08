#include "Server.h"

Server *Server::instance = NULL;

Server::Server(const in_port_t &port)
    : server_socket(socket(AF_INET, SOCK_STREAM, 0)) {
  if (server_socket < 0) {
    throw std::out_of_range("Server::socket < 0!");
  }
  if (port < 1024) {
    throw std::out_of_range("Server::port < 1024!");
  }

  address.sin_family = AF_INET;
  address.sin_port = htons(port);
  address.sin_addr.s_addr = INADDR_ANY;
  if (bind(server_socket, (sockaddr *)&address, sizeof(sockaddr_in)) < 0) {
    throw std::domain_error("Server::socket cannot bind port!");
  }
  if (::listen(server_socket, SOMAXCONN) < 0) {
    throw std::domain_error("Server::socket cannot listen socket!");
  }
}

Server *Server::getInstance(const in_port_t &port) {
  if (instance == NULL) {
    instance = new Server(port);
  }

  return instance;
}

int Server::accept() {
  sockaddr_in remote;
  socklen_t sockaddr_in_len = sizeof(sockaddr_in);
  int client_socket;
  if ((client_socket = ::accept(server_socket, (sockaddr *)&remote,
                                &sockaddr_in_len)) < 0) {
    return client_socket;
  }

  clients.insert(std::make_pair(Socket(client_socket, REQUEST),
                                new Client(client_socket)));

  return client_socket;
}

Client *Server::deleteClient(const int &socket) {
  std::map<Socket, Client *>::iterator client = clients.find(Socket(socket));
  if (client == clients.end()) {
    return NULL;
  }

  clients.erase(client);
  return client->second;
}

bool Server::addClientResourceSocket(const int &socket) {
  std::map<Socket, Client *>::iterator client =
      clients.find(Socket(socket, REQUEST));
  if (client == clients.end()) {
    return false;
  }
  clients.insert(std::make_pair(
      Socket(client->second->getResourceSocket(), RESOURCE), client->second));

  return true;
}

Data *Server::getCachedResource(const std::string &url) const {
  std::map<std::string, Data *>::const_iterator resource = cache.find(url);
  if (resource == cache.end()) {
    return NULL;
  }

  return resource->second;
}

bool Server::moveRequestToResponseSocket(const int &socket) {
  Client *client = deleteClient(socket);
  if (client == NULL) {
    return false;
  }
  clients.insert(std::make_pair(Socket(socket, RESPONSE), client));

  return true;
}

std::pair<pollfd *, size_t> Server::getSocketsTasks() const {
  std::pair<pollfd *, size_t> result;
  result.first = new pollfd[clients.size() + 1];
  result.second = clients.size() + 1;
  size_t i = 0;
  for (std::map<Socket, Client *>::const_iterator client = clients.begin();
       client != clients.end(); client++) {
    result.first[i].fd = client->first.socket;
    switch (client->first.type) {
    case REQUEST:
    case RESOURCE:
      result.first[i].events = POLLIN;
      break;
    case RESPONSE:
      result.first[i].events = POLLOUT;
      break;
    case NONE:
      break;
    }
    i++;
  }
  result.first[i].fd = server_socket;
  result.first[i].events = POLLIN;

  return result;
}

int Server::execClientAction(const int &socket) {
  if (socket == server_socket) {
    return accept();
  }
  std::map<Socket, Client *>::iterator client = clients.find(Socket(socket));
  switch (client->first.type) {
  case REQUEST: {
    int status = client->second->readHttpHeader();
    if (status == -1) {
      delete deleteClient(socket);
    } else if (status == 0) {
      std::string resource = client->second->getRequestedResource();
      if (PicoHttpRequest::isNone(resource)) {
        delete deleteClient(socket);
        return -1;
      }
      Data *data = getCachedResource(resource);
      if (data != NULL) {
#ifdef DEBUG
        std::cerr << "Getting resource " << resource << " from cache."
                  << std::endl;
#endif
        client->second->setSendData(data);
        moveRequestToResponseSocket(socket);
      } else {
        Data *new_data = client->second->sendRequest();
        if (new_data == NULL) {
          delete deleteClient(socket);
        } else {
#ifdef DEBUG
          std::cerr << "Start to download resource " << resource << std::endl;
#endif
          cache.insert(std::make_pair(resource, new_data));
          addClientResourceSocket(socket);
          moveRequestToResponseSocket(socket);
        }
      }
    }
    return status;
  }
  case RESOURCE: {
    int error = client->second->proxyingData();
    if (error == -1) {
      client->second->closeResourceSocket();
      deleteClient(socket);
      delete client->second;
    }
    return error;
  }
  case RESPONSE: {
    int error = client->second->sendData();
    if (error == -1) {
      client->second->closeSocket();
      deleteClient(socket);
    }
    return error;
  }
  case NONE:
    break;
  }

  return 0;
}

Server::~Server() {
  for (std::map<Socket, Client *>::const_iterator client = clients.begin();
       client != clients.end(); client++) {
    delete client->second;
  }
  for (std::map<std::string, Data *>::iterator data = cache.begin();
       data != cache.end(); data++) {
    delete data->second;
  }
}

void Server::updateCache() {
  std::vector<std::map<std::string, Data *>::iterator> to_delete;
  for (std::map<std::string, Data *>::iterator data = cache.begin();
       data != cache.end(); data++) {
    if (data->second->getDeleteRequest()) {
      delete data->second;
      to_delete.push_back(data);
    }
  }
  for (size_t i = 0; i < to_delete.size(); i++) {
#ifdef DEBUG
    std::cerr << "Delete cache: " << to_delete[i]->second->getResourcePath()
              << std::endl;
#endif
    cache.erase(to_delete[i]);
  }
}
