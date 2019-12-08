#include "PicoHttpResponse.h"

const std::string PicoHttpResponse::CONTENT_LENGTH = "Content-Length";

PicoHttpResponse::PicoHttpResponse() : msg(NULL), msg_len(0), status(0) {}

std::string PicoHttpResponse::getContentLength() const {
  const phr_header *length = getHeader(CONTENT_LENGTH);
  if (length == NULL) {
    return NONE;
  }
  return std::string(length->value, length->value_len);
}

int PicoHttpResponse::parseData(const size_t &curr_buff_len) {
  headers_num = HEADERS_NUM;

  int error = phr_parse_response(
      data, total_data_len + curr_buff_len, &minor_version, &status,
      (const char **)&msg, &msg_len, headers, &headers_num, total_data_len);
  total_data_len += curr_buff_len;

  return error;
}