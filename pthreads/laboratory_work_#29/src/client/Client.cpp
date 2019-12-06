#include "Client.h"

Client::Client(const int &socket)
    : socket(socket), request(nullptr), data(nullptr),
      resource_socket(::socket(AF_INET, SOCK_STREAM, 0)) {
  if (socket < 0) {
    throw std::out_of_range("Client::resource_socket < 0!");
  }
}

std::pair<std::shared_ptr<PicoHttpRequest>, int> Client::readRequest() {
  if (request == nullptr) {
    request = std::make_shared<PicoHttpRequest>();
  }

  auto buffer = request->getDataBuffer();
  size_t length = recv(socket, buffer.first, buffer.second, 0);
  if (length <= 0) {
    request = nullptr;
    return std::make_pair(nullptr, -1);
  }

  int error = request->parseData(length);
  if (error == -1) {
    request = nullptr;
    return std::make_pair(nullptr, -1);
  }

  if (error > 0) {
    std::pair<std::shared_ptr<PicoHttpRequest>, int> rez;
    rez.first = request;
    rez.second = 0;
    request = nullptr;
    return rez;
  }

  return std::make_pair(nullptr, error);
}

Client::~Client() {
  close(socket);
  close(resource_socket);
}

std::shared_ptr<Data>
Client::sendRequest(const std::shared_ptr<PicoHttpRequest> &request) {
  if (!request->getResource().has_value() ||
      !request->getHostName().has_value()) {
    return nullptr;
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
  int error = 0;
  if (connectResourceServer(address.first, address.second,
                            isDNSHostname(address.first))) {
    error = send(resource_socket, req.c_str(), req.size(), 0) > 0;
  }

  if (error <= 0) {
    return nullptr;
  }

  data = std::make_shared<Data>();
  return data;
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
    port = std::stoi(host.substr(delimiter + 1), nullptr, 10);
  }

  return std::make_pair(host.substr(0, delimiter), port);
}

bool Client::isDNSHostname(const std::string &host) {
  std::regex ipv4(R"(\d{1,3}.\d{1,3}.\d{1,3}.\d{1,3})");
  return std::regex_match(host, ipv4);
}

bool Client::proxyingData() const {
  if (data == nullptr) {
    return false;
  }
  char buffer[BUFSIZ];
  size_t len = recv(resource_socket, buffer, BUFSIZ, 0);

#ifdef DEBUG
  std::cerr << "Receiving data: " << len << "  bytes." << std::endl;
#endif

  if (len <= 0) {
    data->setCoherence(true);
    return false;
  }
  std::shared_ptr<char> array(new char[len], std::default_delete<char[]>());
  memcpy(array.get(), buffer, len);
  data->pushData(std::make_pair(array, len));

  return send(socket, buffer, len, 0) > 0;
}

bool Client::sendData(
    const std::pair<std::shared_ptr<char>, size_t> &data) const {
#ifdef DEBUG
  std::cerr << "Send data: " << std::string(data.first.get(), data.second)
            << std::endl;
#endif
  return send(socket, data.first.get(), data.second, 0) > 0;
}

bool Data::sendDataToClients() {
  std::vector<std::shared_ptr<Client>> to_delete;
  for (const auto &client : clients) {
    if (client.second >= data.size()) {
      if (coherence) {
        to_delete.push_back(client.first);
      }
      continue;
    }
    if (!client.first->sendData(data[client.second])) {
      to_delete.push_back(client.first);
      continue;
    }
    clients[client.first]++;
  }

  for (auto client : to_delete) {
    clients.erase(client);
  }

  return true;
}
