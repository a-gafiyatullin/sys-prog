#pragma once

#include "../utility/PicoHttpParser.h"
#include <arpa/inet.h>
#include <iostream>
#include <memory>
#include <netdb.h>
#include <netinet/in.h>
#include <regex>
#include <sys/socket.h>
#include <system_error>
#include <unistd.h>

class Client {
private:
  int socket;
  int resource_socket;
  std::shared_ptr<HttpRequestInfo> curr_req;
  sockaddr_in resourceServerAddress{};

  bool connectResourceServer(const std::string &address, const in_port_t &port,
                             const bool &isIp);
  [[nodiscard]] static std::pair<std::string, in_port_t>
  parseHostname(const std::string &host);
  static bool isDNSHostname(const std::string &host);

public:
  explicit Client(const int &socket);

  [[nodiscard]] std::pair<std::shared_ptr<HttpRequestInfo>, int> readRequest();

  [[nodiscard]] bool proxyingData() const;

  bool sendRequest(const std::shared_ptr<HttpRequestInfo> &request);

  [[nodiscard]] inline int getResourceSocket() const { return resource_socket; }

  [[nodiscard]] inline int getSocket() const { return socket; }

  ~Client();
};