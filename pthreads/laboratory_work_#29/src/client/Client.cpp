#include "Client.h"

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

bool Client::sendRequest(const std::shared_ptr<HttpRequestInfo> &request) {
  if (!request->getResource().has_value() ||
      !request->getHostName().has_value()) {
    return false;
  }
  auto req = "GET " + request->getResource().value() + " HTTP/1.1\r\n";
  req += "Host: " + request->getHostName().value() + "\r\n";
  if (request->getUserHeaders().has_value()) {
    req += request->getUserHeaders().value();
  }
  req += "\r\n";

#ifdef DEBUG
  std::cerr << "Send request: " << req;
#endif

  auto address = parseHostname(request->getHostName().value());
  if (connectResourceServer(address.first, address.second,
                            isDNSHostname(address.first))) {
    return send(resource_socket, req.c_str(), req.size(), 0) > 0;
  }

  return false;
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
    addrinfo request{};
    request.ai_flags = AI_ADDRCONFIG;
    request.ai_family = AF_INET;
    request.ai_protocol = SOCK_STREAM;
    request.ai_socktype = 0;
    request.ai_addr = nullptr;
    request.ai_addrlen = 0;
    request.ai_canonname = nullptr;
    request.ai_next = nullptr;
    if (getaddrinfo(address.c_str(), nullptr, &request, &info) < 0) {
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
    port = std::stoi(host.substr(delimiter), nullptr, 10);
  }

  return std::make_pair(host.substr(0, delimiter), port);
}

bool Client::isDNSHostname(const std::string &host) {
  std::regex ipv4(R"(\d{1,3}.\d{1,3}.\d{1,3}.\d{1,3})");
  return std::regex_match(host, ipv4);
}

bool Client::proxyingData() const {
  char buffer[BUFSIZ];
  size_t len = recv(resource_socket, buffer, BUFSIZ, 0);

#ifdef DEBUG
  std::cerr << "Receiving data: " << len << "  bytes." << std::endl;
#endif

  if (len <= 0) {
    return false;
  }

  return send(socket, buffer, len, 0) > 0;
}
