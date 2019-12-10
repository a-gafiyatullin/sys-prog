#include "Client.h"

Client::Client(const int &socket)
    : socket(socket), http_header(NULL), response_status(-1),
      requested_resource(PicoHttpParser::NONE), resourceServerAddress(NULL),
      resource_socket(::socket(AF_INET, SOCK_STREAM, 0)) {
  if (resource_socket < 0) {
    throw std::domain_error("Client::Client : resource_socket < 0!");
  }
  data.first = NULL;
  data.second = 0;
  if (pthread_mutex_init(&send_mutex, NULL) < 0) {
    throw std::domain_error("Client::Client : can't init mutex!");
  }
}

int Client::readHttpHeader() {
  if (http_header == NULL) {
    http_header = new PicoHttpRequest();
  }

  while (true) {
    std::pair<char *, ssize_t> buffer = http_header->getDataBuffer();
    ssize_t length = 0;
    if (http_header->isRequest()) {
      length = recv(socket, buffer.first, buffer.second, 0);
    } else {
      length = recv(resource_socket, buffer.first, buffer.second, 0);
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
#ifdef DEBUG
      std::cerr << "Successfully read a http header!" << std::endl;
#endif
      if (http_header->isRequest()) {
        requested_resource =
            dynamic_cast<PicoHttpRequest *>(http_header)->getURL();
      } else {
        response_status =
            dynamic_cast<PicoHttpResponse *>(http_header)->getStatus();
      }
      return 0;
    }

    return error;
  }
}

int Client::sendRequest() {
  PicoHttpRequest *request = dynamic_cast<PicoHttpRequest *>(http_header);
  if (PicoHttpRequest::isNone(request->getResource()) ||
      PicoHttpRequest::isNone(request->getHostName())) {
    return -1;
  }
  std::string req_str = "GET " + request->getResource() + " HTTP/1.1\r\n";
  req_str += "Host: " + request->getHostName() + "\r\n";
  req_str += "Connection: close\r\n";
  if (!PicoHttpRequest::isNone(request->getUserHeaders())) {
    req_str += request->getUserHeaders();
  }
  req_str += "\r\n";

  int req_len = req_str.size();
  int length = 0;
  while (length != req_len) {
    int curr_len = send(resource_socket, req_str.substr(length).c_str(),
                        req_len - length, 0);
    if (curr_len <= 0) {
      return -1;
    }
    length += curr_len;
  }
#ifdef DEBUG
  std::cerr << "Successfully sent a request to the resource sever!"
            << std::endl;
#endif
  delete http_header;
  http_header = new PicoHttpResponse();

  return 0;
}

int Client::connectResourceServer() {
  if (http_header == NULL || !http_header->isRequest()) {
    return -1;
  }
  if (resourceServerAddress == NULL) {
    resourceServerAddress = new sockaddr_in;
    std::pair<std::string, in_port_t> address = parseHostname(
        dynamic_cast<PicoHttpRequest *>(http_header)->getHostName());
    if (isDNSHostname(address.first)) {
      resourceServerAddress->sin_family = AF_INET;
      resourceServerAddress->sin_port = htons(address.second);
      if (inet_pton(AF_INET, address.first.c_str(),
                    &resourceServerAddress->sin_addr) < 0) {
        return -1;
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
      if (getaddrinfo(address.first.c_str(), NULL, &req, &info) < 0) {
        return -1;
      }
      *resourceServerAddress = *(sockaddr_in *)info->ai_addr;
      resourceServerAddress->sin_port = htons(address.second);
      freeaddrinfo(info);
    }
  }

  return connect(resource_socket, (sockaddr *)resourceServerAddress,
                 sizeof(sockaddr_in));
}

std::pair<std::string, in_port_t>
Client::parseHostname(const std::string &host) {
  ssize_t delimiter = host.find(':');
  in_port_t port = 80;

  if (delimiter != std::string::npos) {
    port = atoi(host.substr(delimiter + 1).c_str());
  }

  return std::make_pair(host.substr(0, delimiter), port);
}

bool Client::isDNSHostname(const std::string &host) {
  ssize_t idx = host.find_first_not_of("0123456789.");
  return idx == std::string::npos;
}

int Client::getResource() {
  char buffer[BUFSIZ];
  ssize_t length = 0;

  while ((length = recv(resource_socket, buffer, BUFSIZ, 0)) > 0) {
    char *array = new char[length];
    memcpy(array, buffer, length);
    data.first->pushData(std::make_pair(array, length));
  }

  // check if download is canceled
  if (length <= 0) {
    if (data.first->getExpectedLength() > 0 && !data.first->isFull()) {
      data.first->setDeleteRequest(true);
#ifdef DEBUG
      std::cerr << "Cache coherence is broken!" << std::endl;
#endif
    } else {
      data.first->setCoherence(true);
    }
  }

  return 0;
}

int Client::sendResource() {
  while (true) {
    if (data.first->getDeleteRequest()) {
      return -1;
    }

    // different sending strategies for caching and non-caching resources
    char *buffer = NULL;
    ssize_t length = 0;
    std::pair<char *, ssize_t> data_piece;
    if (data.second == -1) {
      data_piece = data.first->popFirst(&send_mutex);
      if (data_piece.second == -1) {
        return 0;
      }
      length = data_piece.second;
    } else if (data.second != -1) {
      data_piece = data.first->at(data.second, &send_mutex);
      if (data_piece.second == -1) {
        return 0;
      }
      length = data_piece.second;
      data.second++;
    }
    buffer = data_piece.first;

    int sent_len = 0;
    while (sent_len != length) {
      int curr_len = send(socket, buffer + sent_len, length - sent_len, 0);
      if (curr_len <= 0) {
        return -1;
      }
      sent_len += curr_len;
    }

    if (data.second == -1) {
      delete[] data_piece.first;
    }
  }
}

std::pair<Data *, ssize_t> Client::parseResponse() {
  if (http_header == NULL || http_header->isRequest()) {
    return std::make_pair((Data *)NULL, -1);
  }

  // add to cache only OK resources
  data.first = new Data();
  if (getResponseStatus() == PicoHttpResponse::OK) {
    data.second = 0;
  } else {
    data.second = -1;
  }

  std::string content_length =
      dynamic_cast<PicoHttpResponse *>(http_header)->getContentLength();
  if (PicoHttpParser::isNone(content_length)) {
    data.first->setExpectedLength(-1);
  } else {
    data.first->setExpectedLength(atoi(content_length.c_str()));
  }

  // http header is a part of the resource
  std::pair<char *, ssize_t> buffer = http_header->getDataBuffer(0);
  char *array = new char[buffer.second];
  memcpy(array, buffer.first, buffer.second);
  data.first->pushData(std::make_pair(array, buffer.second));

  return data;
}

Client::~Client() {
  delete http_header;
  if (data.second == -1) {
    delete data.first;
  }
  close(socket);
  close(resource_socket);
  delete resourceServerAddress;
}

Data::~Data() {
  pthread_mutex_lock(&mutex);
  for (size_t i = 0; i < data.size(); i++) {
    delete[] data[i].first;
  }
  pthread_cond_destroy(&empty);
  pthread_mutex_unlock(&mutex);
  pthread_mutex_destroy(&mutex);
}

std::pair<char *, ssize_t> Data::popFirst(pthread_mutex_t *client_mutex) {
  while (true) {
    pthread_mutex_lock(&mutex);
    if (data.empty() && !coherence) {
      pthread_mutex_unlock(&mutex);
      pthread_mutex_lock(client_mutex);
      pthread_cond_wait(&empty, client_mutex); // wait data
      pthread_mutex_unlock(client_mutex);
    } else if (data.empty() && coherence) {
      pthread_mutex_unlock(&mutex);
      return std::make_pair((char *)NULL, -1);
    } else {
      break;
    }
  }

  std::pair<char *, ssize_t> first_data = data.front();
  data.erase(data.begin());
  pthread_mutex_unlock(&mutex);

  return first_data;
}

Data::Data()
    : coherence(false), expected_length(0), current_length(0),
      delete_request(false) {
  if (pthread_cond_init(&empty, NULL) < 0) {
    throw std::domain_error("Data::Data : can't init conditional variable!");
  }
  if (pthread_mutex_init(&mutex, NULL) < 0) {
    throw std::domain_error("Data::Data : can't init mutex!");
  }
}

void Data::pushData(const std::pair<char *, ssize_t> new_data) {
  pthread_mutex_lock(&mutex);
  data.push_back(new_data);
  current_length += new_data.second;
  pthread_mutex_unlock(&mutex);
  pthread_cond_broadcast(&empty);
}

std::pair<char *, ssize_t> Data::at(const ssize_t &i,
                                    pthread_mutex_t *client_mutex) {
  while (true) {
    pthread_mutex_lock(&mutex);
    if (i >= data.size() && !coherence) {
      pthread_mutex_unlock(&mutex);
      pthread_mutex_lock(client_mutex);
      pthread_cond_wait(&empty, client_mutex); // wait new data
      pthread_mutex_unlock(client_mutex);
    } else if (i >= data.size() && coherence) {
      pthread_mutex_unlock(&mutex);
      return std::make_pair((char *)NULL, -1);
    } else {
      break;
    }
  }

  std::pair<char *, ssize_t> last_data = data[i];
  pthread_mutex_unlock(&mutex);

  return last_data;
}

void Data::setCoherence(const bool &status) {
  pthread_mutex_lock(&mutex);
  coherence = status;
  pthread_mutex_unlock(&mutex);
}

void Data::setExpectedLength(const ssize_t &length) {
  pthread_mutex_lock(&mutex);
  expected_length = length;
  pthread_mutex_unlock(&mutex);
}

ssize_t Data::getExpectedLength() {
  pthread_mutex_lock(&mutex);
  ssize_t expected_len = expected_length;
  pthread_mutex_unlock(&mutex);

  return expected_len;
}

void Data::setDeleteRequest(const bool &status) {
  pthread_mutex_lock(&mutex);
  delete_request = status;
  pthread_mutex_unlock(&mutex);
}

bool Data::getDeleteRequest() {
  pthread_mutex_lock(&mutex);
  bool delete_req = delete_request;
  pthread_mutex_unlock(&mutex);

  return delete_req;
}

bool Data::isFull() {
  pthread_mutex_lock(&mutex);
  bool is_full = expected_length <= current_length;
  pthread_mutex_unlock(&mutex);

  return is_full;
}
