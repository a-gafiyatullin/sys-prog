#include "PicoHttpRequest.h"

const std::string PicoHttpRequest::HOST_HEADER = "Host";

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
    : method_len(0), path_len(0), method(NULL), path(NULL) {
  user_headers_names.push_back("DNT");
  user_headers_names.push_back("User-Agent");
  user_headers_names.push_back("Accept");
  user_headers_names.push_back("Accept-Encoding");
  user_headers_names.push_back("Accept-Language");
}

std::string PicoHttpRequest::getMethod() const {
  if (method == NULL) {
    return NONE;
  } else {
    return std::string(method, method_len);
  }
}

std::string PicoHttpRequest::getResource() const {
  std::string url = getURL();
  std::string host = getHostName();
  if (url != NONE && host != NONE) {
    size_t host_name_idx = url.find(host);
    if (host_name_idx != std::string::npos) {
      return url.substr(host_name_idx + host.length());
    } else {
      return NONE;
    }
  } else {
    return NONE;
  }
}

std::string PicoHttpRequest::getURL() const {
  if (path == NULL) {
    return NONE;
  } else {
    return std::string(path, path_len);
  }
}

std::string PicoHttpRequest::getHostName() const {
  if (path == NULL || method == NULL) {
    return NONE;
  } else {
    const phr_header *header = getHeader(HOST_HEADER);
    if (header == NULL) {
      return NONE;
    } else {
      return std::string(header->value, header->value_len);
    }
  }
}

std::string PicoHttpRequest::getUserHeaders() const {
  std::string result_headers;
  for (size_t i = 0; i < user_headers_names.size(); i++) {
    const phr_header *phr = getHeader(user_headers_names[i]);
    if (phr != NULL) {
      result_headers += std::string(phr->name, phr->name_len) + ": " +
                        std::string(phr->value, phr->value_len) + "\r\n";
    }
  }
  if (result_headers.empty()) {
    return NONE;
  } else {
    return result_headers;
  }
}