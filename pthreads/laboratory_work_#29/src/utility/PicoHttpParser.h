#pragma once

#include "../../lib/picohttpparser/picohttpparser.h"
#include <optional>
#include <string>
#include <vector>

class HttpRequestInfo {
private:
  static const size_t HEADERS_NUM = 100;

  size_t method_len;
  size_t path_len;
  size_t headers_num;
  size_t total_req_len;

  char request[BUFSIZ];
  int minor_version;
  phr_header headers[HEADERS_NUM];
  char *method, *path;

public:
  HttpRequestInfo();

  [[nodiscard]] std::optional<std::string> getMethod() const;

  [[nodiscard]] std::optional<std::string> getPath() const;

  [[nodiscard]] inline int getMinorVersion() const { return minor_version; }

  [[nodiscard]] inline std::pair<phr_header *, size_t> getHeaders() const {
    return std::make_pair((phr_header *)headers, headers_num);
  }

  [[nodiscard]] inline std::pair<void *, size_t> getRequestBuffer() const {
    return std::make_pair((void *)(request + total_req_len), BUFSIZ - total_req_len);
  }

  int parseRequest(const size_t &curr_buff_len);
};
