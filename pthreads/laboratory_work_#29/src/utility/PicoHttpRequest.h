#pragma once

#include "../../lib/picohttpparser/picohttpparser.h"
#include <optional>
#include <string>
#include <vector>

class PicoHttpRequest {
private:
  static const size_t HEADERS_NUM = 100;

  size_t method_len;
  size_t path_len;
  size_t headers_num;
  size_t total_data_len;
  int minor_version;
  char *method, *path;

  char data[BUFSIZ];
  phr_header headers[HEADERS_NUM];
  static const std::vector<std::string> user_headers_names;

  static const std::string HOST_HEADER;

  [[nodiscard]] const phr_header *getHeader(const std::string &header) const;

public:
  PicoHttpRequest();

  [[nodiscard]] inline int getMinorVersion() const { return minor_version; }

  [[nodiscard]] inline std::optional<std::string> dataToString() const {
    return std::string(data, total_data_len);
  }

  [[nodiscard]] inline std::pair<void *, size_t> getDataBuffer() const {
    return std::make_pair((void *)(data + total_data_len),
                          BUFSIZ - total_data_len);
  }

  [[nodiscard]] std::optional<std::string> getMethod() const;

  [[nodiscard]] std::optional<std::string> getResource() const;

  [[nodiscard]] std::optional<std::string> getHostName() const;

  int parseData(const size_t &curr_buff_len);

  [[nodiscard]] std::optional<std::string> getURL() const;

  [[nodiscard]] std::optional<std::string> getUserHeaders() const;
};