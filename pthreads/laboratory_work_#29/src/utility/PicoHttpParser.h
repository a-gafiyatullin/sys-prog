#pragma once

#include "../../lib/picohttpparser/picohttpparser.h"
#include <cstdio>
#include <string>

class PicoHttpParser {
protected:
  static const size_t HEADERS_NUM = 100;

  size_t headers_num;
  size_t total_data_len;
  int minor_version;

  char data[BUFSIZ];
  phr_header headers[HEADERS_NUM];

  const phr_header *getHeader(const std::string &header) const;

public:
  PicoHttpParser();

  static const std::string NONE;

  inline int getMinorVersion() const { return minor_version; }

  inline std::string dataToString() const {
    return std::string(data, total_data_len);
  }

  inline std::pair<char *, size_t> getDataBuffer() const {
    return std::make_pair((char *)(data + total_data_len),
                          BUFSIZ - total_data_len);
  }

  static bool isNone(const std::string &str) { return NONE == str; }

  virtual int parseData(const size_t &curr_buff_len) = 0;

  virtual bool isRequest() const = 0;

  virtual ~PicoHttpParser() {};
};