#pragma once

#include "../utility/PicoHttpParser.h"
#include <memory>
#include <sys/socket.h>

class Client {
private:
  int socket;
  std::shared_ptr<HttpRequestInfo> curr_req;

public:
  explicit Client(const int &socket);

  [[nodiscard]] std::pair<std::shared_ptr<HttpRequestInfo>, int> readMsg();
};