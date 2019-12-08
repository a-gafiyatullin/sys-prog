#pragma once

#include "PicoHttpParser.h"
#include <vector>

class PicoHttpRequest : public PicoHttpParser {
private:
  size_t method_len;
  size_t path_len;
  char *method, *path;

  std::vector<std::string> user_headers_names;

  static const std::string HOST_HEADER;

public:
  PicoHttpRequest();

  std::string getMethod() const;

  std::string getResource() const;

  std::string getHostName() const;

  int parseData(const size_t &curr_buff_len);

  bool isRequest() const { return true; }

  std::string getURL() const;

  std::string getUserHeaders() const;
};