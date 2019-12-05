#include "PicoHttpParser.h"

const std::string HttpRequestInfo::HOST_HEADER = "Host";
const std::vector<std::string> HttpRequestInfo::user_headers_names = {
    "DNT", "User-Agent", "Accept", "Accept-Encoding", "Accept-Language"};

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

std::optional<std::string> HttpRequestInfo::getResource() const {
  auto url = getURL();
  auto host = getHostName();
  if (url.has_value() && host.has_value()) {
    auto host_name = url.value().find(host.value());
    if (host_name != std::string::npos) {
      return url.value().substr(host_name + host.value().length());
    } else {
      return {};
    }
  } else {
    return {};
  }
}

std::optional<std::string> HttpRequestInfo::getURL() const {
  if (path == nullptr) {
    return {};
  } else {
    return std::string(path, path_len);
  }
}

std::optional<std::string> HttpRequestInfo::getHostName() const {
  if (path == nullptr || method == nullptr) {
    return {};
  } else {
    auto header = getHeader(HOST_HEADER);
    if (header == nullptr) {
      return {};
    } else {
      return std::string(header->value, header->value_len);
    }
  }
}

const phr_header *HttpRequestInfo::getHeader(const std::string &header) const {
  for (int i = 0; i < headers_num; i++) {
    if (std::string(headers[i].name, headers[i].name_len) == header) {
      return headers + i;
    }
  }

  return nullptr;
}

std::optional<std::string> HttpRequestInfo::getUserHeaders() const {
  std::string result_headers;
  for (const auto &header : user_headers_names) {
    auto phr = getHeader(header);
    if (phr != nullptr) {
      result_headers += std::string(phr->name, phr->name_len) + ": " +
                        std::string(phr->value, phr->value_len) + "\r\n";
    }
  }
  if (result_headers.empty()) {
    return {};
  } else {
    return result_headers;
  }
}