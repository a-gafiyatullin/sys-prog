#include "PicoHttpRequest.h"

const std::string PicoHttpRequest::HOST_HEADER = "Host";
const std::vector<std::string> PicoHttpRequest::user_headers_names = {
    "DNT", "User-Agent", "Accept", "Accept-Encoding", "Accept-Language"};

int PicoHttpRequest::parseData(const size_t &d_req_len) {
  headers_num = HEADERS_NUM;

  int error = phr_parse_request(data, total_data_len + d_req_len,
                                (const char **)(&method), &method_len,
                                (const char **)&path, &path_len, &minor_version,
                                headers, &headers_num, total_data_len);
  total_data_len += d_req_len;

  return error;
}

PicoHttpRequest::PicoHttpRequest()
    : method_len(0), path_len(0), method(nullptr), path(nullptr),
      headers_num(HEADERS_NUM), total_data_len(0), minor_version(-1) {}

std::optional<std::string> PicoHttpRequest::getMethod() const {
  if (method == nullptr) {
    return {};
  } else {
    return std::string(method, method_len);
  }
}

std::optional<std::string> PicoHttpRequest::getResource() const {
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

std::optional<std::string> PicoHttpRequest::getURL() const {
  if (path == nullptr) {
    return {};
  } else {
    return std::string(path, path_len);
  }
}

std::optional<std::string> PicoHttpRequest::getHostName() const {
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

std::optional<std::string> PicoHttpRequest::getUserHeaders() const {
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

const phr_header *PicoHttpRequest::getHeader(const std::string &header) const {
  for (int i = 0; i < headers_num; i++) {
    if (std::string(headers[i].name, headers[i].name_len) == header) {
      return headers + i;
    }
  }

  return nullptr;
}