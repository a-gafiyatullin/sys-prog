#include "PicoHttpParser.h"

int HttpRequestInfo::parseRequest(const size_t &d_req_len) {
  headers_num = HEADERS_NUM;

  int error = phr_parse_request(request, total_req_len + d_req_len,
                                (const char **)(&method), &method_len,
                                (const char **)&path, &path_len, &minor_version,
                                headers, &headers_num, total_req_len);
  total_req_len += d_req_len;

  return error;
}

HttpRequestInfo::HttpRequestInfo()
    : method_len(0), path_len(0), minor_version(0), method(nullptr),
      path(nullptr), total_req_len(0), headers_num(HEADERS_NUM) {}

std::optional<std::string> HttpRequestInfo::getMethod() const {
  if (method == nullptr) {
    return {};
  } else {
    return std::string(method, method_len);
  }
}

std::optional<std::string> HttpRequestInfo::getPath() const {
  if (path == nullptr) {
    return {};
  } else {
    return std::string(path, path_len);
  }
}
