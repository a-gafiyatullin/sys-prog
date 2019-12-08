#include "Client.h"

Client::Client(const int &socket)
    : socket(socket), http_header(NULL), resource_socket(::socket(AF_INET, SOCK_STREAM, 0)) {
  if(resource_socket < 0) {
    throw std::domain_error("Client::resource_socket < 0!");
  }
  data.first = NULL;
  data.second = 0;
}

int Client::readHttpHeader(const char *ext_buffer, const size_t &ext_length) {
  if (http_header == NULL) {
    http_header = new PicoHttpRequest();
  }

  std::pair<char *, size_t> buffer = http_header->getDataBuffer();
  size_t length = 0;
  if (ext_buffer == NULL) {
    length = recv(socket, buffer.first, buffer.second, 0);
  } else {
    memcpy(buffer.first, ext_buffer, std::min(ext_length, buffer.second));
    length = ext_length;
  }
  if (length <= 0) {
    delete http_header;
    http_header = NULL;
    return -1;
  }

  int error = http_header->parseData(length);
  if (error == -1) {
    delete http_header;
    http_header = NULL;
    return -1;
  }

  if (error > 0) {
    return 0;
  }

  return error;
}

Data *Client::sendRequest() {
  PicoHttpRequest *request = dynamic_cast<PicoHttpRequest *>(http_header);
  if (PicoHttpRequest::isNone(request->getResource()) ||
      PicoHttpRequest::isNone(request->getHostName())) {
    return NULL;
  }
  std::string req_str = "GET " + request->getResource() + " HTTP/1.1\r\n";
  req_str += "Host: " + request->getHostName() + "\r\n";
  req_str += "Connection: close\r\n";
  if (!PicoHttpRequest::isNone(request->getUserHeaders())) {
    req_str += request->getUserHeaders();
  }
  req_str += "\r\n";

  std::pair<std::string, in_port_t> address =
      parseHostname(request->getHostName());
  int error = 0;
  if (connectResourceServer(address.first, address.second,
                            isDNSHostname(address.first))) {
    error = send(resource_socket, req_str.c_str(), req_str.size(), 0) > 0;
  }

  if (error <= 0) {
    return NULL;
  }

  data = std::make_pair(new Data(request->getURL()), 0);
  return data.first;
}

bool Client::connectResourceServer(const std::string &address,
                                   const in_port_t &port, const bool &isIp) {
  if (isIp) {
    resourceServerAddress.sin_family = AF_INET;
    resourceServerAddress.sin_port = htons(port);
    if (inet_pton(AF_INET, address.c_str(), &resourceServerAddress.sin_addr) <
        0) {
      return false;
    }
  } else {
    addrinfo *info;
    addrinfo req = {};
    req.ai_flags = AI_ADDRCONFIG;
    req.ai_family = AF_INET;
    req.ai_protocol = SOCK_STREAM;
    req.ai_socktype = 0;
    req.ai_addr = NULL;
    req.ai_addrlen = 0;
    req.ai_canonname = NULL;
    req.ai_next = NULL;
    if (getaddrinfo(address.c_str(), NULL, &req, &info) < 0) {
      return false;
    }
    resourceServerAddress = *(sockaddr_in *)info->ai_addr;
    resourceServerAddress.sin_port = htons(port);
    freeaddrinfo(info);
  }

  return connect(resource_socket, (sockaddr *)&resourceServerAddress,
                 sizeof(resourceServerAddress)) == 0;
}

std::pair<std::string, in_port_t>
Client::parseHostname(const std::string &host) {
  size_t delimiter = host.find(':');
  in_port_t port = 80;

  if (delimiter != std::string::npos) {
    port = atoi(host.substr(delimiter + 1).c_str());
  }

  return std::make_pair(host.substr(0, delimiter), port);
}

bool Client::isDNSHostname(const std::string &host) {
  size_t idx = host.find_first_not_of("0123456789.");
  return idx == std::string::npos;
}

int Client::proxyingData() {
  if (data.first == NULL) {
    return -1;
  }

  char buffer[BUFSIZ];
  ssize_t len = recv(resource_socket, buffer, BUFSIZ, 0);

  // check if download is canceled
  if (len <= 0) {
    if (!http_header->isRequest()) {
      if (data.first->getExpectedLength() > 0 && !data.first->isFull()) {
        data.first->setDeleteRequest(true);
#ifdef DEBUG
        std::cerr << "Cache coherence is broken!" << std::endl;
#endif
        return -1;
      }
    } else {
      data.first->setCoherence(true);
      return -1;
    }
  }

  // parse response header from resource server and get content length
  if (data.first->getExpectedLength() == 0) {
    if (http_header->isRequest()) {
      delete http_header;
      http_header = new PicoHttpResponse();
    }
    int error = readHttpHeader(buffer, len);
    if (error == -1) {
      return error;
    }
    if (error == 0) {
      std::string length =
          dynamic_cast<PicoHttpResponse *>(http_header)->getContentLength();
      if (PicoHttpParser::isNone(length)) {
        data.first->setExpectedLength(-1);
      } else {
        data.first->setExpectedLength(atoi(length.c_str()));
      }
    }
  }

  char *array = new char[len];
  memcpy(array, buffer, len);
  data.first->pushData(std::make_pair(array, len));

  return 0;
}

int Client::sendData() {
  if (data.first->getDeleteRequest()) {
    return -1;
  }
  if (data.second >= data.first->getDataPieceCounter()) {
    return (data.first->getCoherence() ? -1 : -2);
  }

  std::pair<char *, size_t> curr_data_piece = data.first->at(data.second);
  data.second++;

  return send(socket, curr_data_piece.first, curr_data_piece.second, 0);
}

std::string Client::getRequestedResource() const {
  if (http_header == NULL) {
    return PicoHttpParser::NONE;
  }
  if (http_header->isRequest()) {
    return dynamic_cast<PicoHttpRequest *>(http_header)->getURL();
  }
  return data.first->getResourcePath();
}

Data::~Data() {
  for (size_t i = 0; i < data.size(); i++) {
    delete[] data[i].first;
  }
}
