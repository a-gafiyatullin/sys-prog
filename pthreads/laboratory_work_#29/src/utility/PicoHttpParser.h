#pragma once

#include "../../lib/picohttpparser/picohttpparser.h"
#include <optional>
#include <string>
#include <vector>

class HttpRequestInfo {
private:
  std::string method;
  std::string path;
  int minor_version;
  std::vector<phr_header> headers;

public:
  HttpRequestInfo(const char *method, size_t method_len, const char *path,
                  size_t path_len, const int &version,
                  const phr_header *headers, size_t headers_len);
  [[nodiscard]] inline std::string getMethod() const { return method; }
  [[nodiscard]] inline std::string getPath() const { return path; }
  [[nodiscard]] inline int getMinorVersion() const { return minor_version; }
  [[nodiscard]] inline std::vector<phr_header> getHeaders() const { return headers; }
};

class PicoHttpParser {
private:
  static const size_t HEADERS_NUM = 1000;

public:
  static std::optional<HttpRequestInfo>
  parseRequest(char *request, size_t bufflen, size_t prevbufflen);
};
