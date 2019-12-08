#pragma once

#include "PicoHttpParser.h"

class PicoHttpResponse : public PicoHttpParser {
private:
  int status;
  char *msg;
  size_t msg_len;

  static const std::string CONTENT_LENGTH;

public:
  PicoHttpResponse();

  int parseData(const size_t &curr_buff_len);

  bool isRequest() const { return false; }

  std::string getContentLength() const;

  int getStatus() const { return status; }
};