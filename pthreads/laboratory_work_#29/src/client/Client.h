#pragma once

#include "../utility/PicoHttpRequest.h"
#include <arpa/inet.h>
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <memory>
#include <netdb.h>
#include <netinet/in.h>
#include <regex>
#include <sys/socket.h>
#include <system_error>
#include <unistd.h>

class Client;

class Data {
private:
  std::vector<std::pair<std::shared_ptr<char>, size_t>> data;
  std::map<std::shared_ptr<Client>, size_t> clients;
  bool coherence;

public:
  Data() : coherence(false){};

  inline void setCoherence(const bool &status) { coherence = true; }

  inline void pushData(const std::pair<std::shared_ptr<char>, size_t> data) {
    this->data.push_back(data);
  }

  bool sendDataToClients();

  inline void addClient(const std::shared_ptr<Client> &client) {
    clients.insert(std::make_pair(client, 0));
  }
};

class Client {
private:
  int socket;
  int resource_socket;
  std::shared_ptr<PicoHttpRequest> request;
  sockaddr_in resourceServerAddress{};
  std::shared_ptr<Data> data;

  bool connectResourceServer(const std::string &address, const in_port_t &port,
                             const bool &isIp);

  [[nodiscard]] static std::pair<std::string, in_port_t>
  parseHostname(const std::string &host);

  static bool isDNSHostname(const std::string &host);

public:
  explicit Client(const int &socket);

  [[nodiscard]] std::pair<std::shared_ptr<PicoHttpRequest>, int> readRequest();

  [[nodiscard]] bool proxyingData() const;

  bool sendData(const std::pair<std::shared_ptr<char>, size_t> &data) const;

  std::shared_ptr<Data>
  sendRequest(const std::shared_ptr<PicoHttpRequest> &request);

  [[nodiscard]] inline int getResourceSocket() const { return resource_socket; }

  [[nodiscard]] inline int getSocket() const { return socket; }

  ~Client();
};