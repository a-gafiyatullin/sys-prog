#include "PicoHttpParser.h"

std::optional<HttpRequestInfo>
PicoHttpParser::parseRequest(char *request, size_t bufflen,
                             size_t prevbufflen) {
  size_t method_len = 0;
  size_t path_len = 0;
  int minor_version = 0;
  size_t num_headers = HEADERS_NUM;
  phr_header headers[HEADERS_NUM];
  char *method, *path;
  int error =
      phr_parse_request(request, bufflen, (const char **)(&method), &method_len,
                        (const char **)&path, &path_len, &minor_version,
                        headers, &num_headers, prevbufflen);
  if (error <= 0) {
    return {};
  }

  return HttpRequestInfo(method, method_len, path, path_len, minor_version,
                         headers, num_headers);
}

HttpRequestInfo::HttpRequestInfo(const char *method, const size_t method_len,
                                 const char *path, const size_t path_len,
                                 const int &version, const phr_header *headers,
                                 const size_t headers_len) {
  this->method = std::string(method, method_len);
  this->path = std::string(path, path_len);
  minor_version = version;
  for(ssize_t i = 0; i < headers_len; i++) {
    this->headers.push_back(headers[i]);
  }
}
