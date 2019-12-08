#include "PicoHttpParser.h"

const std::string PicoHttpParser::NONE = "NONE";

const phr_header *PicoHttpParser::getHeader(const std::string &header) const {
  for (int i = 0; i < headers_num; i++) {
    if (std::string(headers[i].name, headers[i].name_len) == header) {
      return headers + i;
    }
  }

  return NULL;
}

PicoHttpParser::PicoHttpParser()
    : headers_num(HEADERS_NUM), total_data_len(0), minor_version(-1) {}
