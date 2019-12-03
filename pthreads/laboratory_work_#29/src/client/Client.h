#pragma once

#include "../utility/PicoHttpParser.h"
#include <memory>
#include <netinet/in.h>
#include <sys/socket.h>
#include <system_error>
#include <unistd.h>

class Client {
private:
  int socket;
  int resource_socket;
  std::shared_ptr<HttpRequestInfo> curr_req;

public:
  explicit Client(const int &socket);

  [[nodiscard]] std::pair<std::shared_ptr<HttpRequestInfo>, int> readRequest();

  bool sendRequest(const std::shared_ptr<HttpRequestInfo> &request);

  [[nodiscard]] inline int getResourceSocket() const { return resource_socket; }

  ~Client();
};