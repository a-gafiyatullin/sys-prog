#include "Client.h"
#include <iostream>

Client::Client(const int &socket)
    : socket(socket), curr_req(nullptr),
      resource_socket(::socket(AF_INET, SOCK_STREAM, 0)) {
  if (socket < 0) {
    throw std::out_of_range("Client::resource_socket < 0!");
  }
}

std::pair<std::shared_ptr<HttpRequestInfo>, int> Client::readRequest() {
  if (curr_req == nullptr) {
    curr_req = std::make_shared<HttpRequestInfo>();
  }

  auto buffer = curr_req->getRequestBuffer();
  size_t length = recv(socket, buffer.first, buffer.second, 0);
  if (length <= 0) {
    curr_req = nullptr;
    return std::make_pair(nullptr, -1);
  }

  int error = curr_req->parseRequest(length);
  if (error == -1) {
    curr_req = nullptr;
    return std::make_pair(nullptr, -1);
  }

  if (error > 0) {
    std::pair<std::shared_ptr<HttpRequestInfo>, int> rez;
    rez.first = curr_req;
    rez.second = 0;
    curr_req = nullptr;
    return rez;
  }

  return std::make_pair(nullptr, error);
}

Client::~Client() {
  close(socket);
  close(resource_socket);
}

bool Client::sendRequest(const std::shared_ptr<HttpRequestInfo> &request) {}
