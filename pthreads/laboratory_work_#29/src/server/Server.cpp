#include "Server.h"

Server *Server::instance = NULL;

Server::Server(const in_port_t &port)
    : server_socket(socket(AF_INET, SOCK_STREAM, 0)) {
  if (server_socket < 0) {
    throw std::out_of_range("Server::Server : socket < 0!");
  }
  if (port < 1024) {
    throw std::out_of_range("Server::Server : port < 1024!");
  }

  address.sin_family = AF_INET;
  address.sin_port = htons(port);
  address.sin_addr.s_addr = INADDR_ANY;
  if (bind(server_socket, (sockaddr *)&address, sizeof(sockaddr_in)) < 0) {
    throw std::domain_error("Server::Server : socket cannot bind port!");
  }
  if (::listen(server_socket, SOMAXCONN) < 0) {
    throw std::domain_error("Server::Server : socket cannot listen socket!");
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

  clients.insert(std::make_pair(Socket(client_socket, GET_REQUEST),
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
  std::map<Socket, Client *>::iterator client = clients.find(Socket(socket));
  if (client == clients.end()) {
    return false;
  }
  clients.insert(std::make_pair(
      Socket(client->second->getResourceSocket(), CONNECT), client->second));

  return true;
}

Data *Server::getCachedResource(const std::string &url) const {
  std::map<std::string, Data *>::const_iterator resource = cache.find(url);
  if (resource == cache.end()) {
    return NULL;
  }

  return resource->second;
}

int Server::getSocketsTasks(fd_set &w, fd_set &r, fd_set &e) {
  FD_ZERO(&w);
  FD_ZERO(&r);
  FD_ZERO(&e);
  int max_socket = 0;

  for (std::map<Socket, Client *>::const_iterator client = clients.begin();
       client != clients.end(); client++) {
    switch (client->first.type) {
    case GET_REQUEST:
    case GET_RESOURCE:
    case GET_RESPONSE:
      FD_SET(client->first.socket, &r);
      break;
    case CONNECT:
    case SEND_REQUEST:
    case SEND_RESOURCE:
      FD_SET(client->first.socket, &w);
      break;
    case NONE:
      break;
    }
    if (client->first.socket > max_socket) {
      max_socket = client->first.socket;
    }
  }
  FD_SET(server_socket, &r);
  if (server_socket > max_socket) {
    max_socket = server_socket;
  }

  return max_socket;
}

int Server::execClientAction(const int &socket) {
  if (socket == server_socket) {
#ifdef DEBUG
    std::cerr << "New connection!" << std::endl;
#endif
    return accept();
  }
  std::map<Socket, Client *>::iterator client = clients.find(Socket(socket));
  switch (client->first.type) {
  case GET_REQUEST: {
    int status = client->second->readHttpHeader();
    if (status == -1) {
#ifdef DEBUG
      std::cerr << "GET_REQUEST: Connection is rejected!" << std::endl;
#endif
      delete deleteClient(socket);
      return -1;
    } else if (status == 0) {
      std::string resource = client->second->getRequestedResource();
      if (PicoHttpRequest::isNone(resource)) {
#ifdef DEBUG
        std::cerr << "Nothing to download!" << std::endl;
#endif
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
        changeSocketType(socket, SEND_RESOURCE);
        return 0;
      } else {
        changeSocketType(socket, NONE);
        addClientResourceSocket(socket);
      }
    }
  }

  case CONNECT: {
    int error = client->second->connectResourceServer();
    if (error == -1) {
#ifdef DEBUG
      std::cerr << "CONNECT: Connection is rejected!" << std::endl;
#endif
      deleteClient(client->second->getSocket());
      delete deleteClient(socket);
    } else if (error == 0) {
#ifdef DEBUG
      std::cerr << "CONNECT: Connection is successful!" << std::endl;
#endif
      changeSocketType(socket, SEND_REQUEST);
    }
    return error;
  }

  case SEND_REQUEST: {
    int error = client->second->sendRequest();
    if (error == -1) {
#ifdef DEBUG
      std::cerr << "SEND_REQUEST: Connection is rejected!" << std::endl;
#endif
      deleteClient(client->second->getSocket());
      delete deleteClient(socket);
    } else if (error == 0) {
      changeSocketType(socket, GET_RESPONSE);
    }
    return error;
  }

  case GET_RESPONSE: {
    int error = client->second->readHttpHeader();
    if (error == -1) {
#ifdef DEBUG
      std::cerr << "GET_RESPONSE: Connection is rejected!" << std::endl;
#endif
      deleteClient(client->second->getSocket());
      delete deleteClient(socket);
    } else if (error == 0) {
      std::pair<Data *, ssize_t> status = client->second->parseResponse();
      if (status.second == 0) {
        cache.insert(std::make_pair(client->second->getRequestedResource(),
                                    status.first));
      }
#ifdef DEBUG
      std::cerr << "Start to download resource "
                << client->second->getRequestedResource() << std::endl;
#endif
      changeSocketType(client->second->getSocket(), SEND_RESOURCE);
      changeSocketType(socket, GET_RESOURCE);
    }
    return error;
  }

  case GET_RESOURCE: {
    int error = client->second->getResource();
    if (error == -1) {
#ifdef DEBUG
      std::cerr << "Resource " << client->second->getRequestedResource()
                << " is downloaded!" << std::endl;
#endif
      client->second->closeResourceSocket();
      deleteClient(socket);
      if (client->second->getDeleteRequest()) {
        delete client->second;
      } else {
        client->second->setDeleteRequest(true);
      }
    }
    return error;
  }

  case SEND_RESOURCE: {
    int error = client->second->sendResource();
    if (error == -1) {
#ifdef DEBUG
      std::cerr << "Resource " << client->second->getRequestedResource()
                << " is sent!" << std::endl;
#endif
      client->second->closeSocket();
      deleteClient(socket);
      if (client->second->getDeleteRequest()) {
        delete client->second;
      } else {
        client->second->setDeleteRequest(true);
      }
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

  close(server_socket);
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
    cache.erase(to_delete[i]);
  }
}

bool Server::changeSocketType(const int &socket, const SocketType &new_type) {
  Client *client = deleteClient(socket);
  if (client == NULL) {
    return false;
  }
  clients.insert(std::make_pair(Socket(socket, new_type), client));

  return true;
}