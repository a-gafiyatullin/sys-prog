#pragma once

#include "../utility/PicoHttpRequest.h"
#include "../utility/PicoHttpResponse.h"
#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

class Client;

class Data {
private:
  std::vector<std::pair<char *, size_t> > data;
  bool coherence;
  bool delete_request;
  ssize_t expected_length;
  size_t current_length;
  std::string resource_path;

public:
  Data(const std::string &resource_path)
      : coherence(false), expected_length(0), current_length(0),
        resource_path(resource_path), delete_request(false){};

  inline void setCoherence(const bool &status) { coherence = status; }

  inline void setExpectedLength(ssize_t length) { expected_length = length; }

  inline ssize_t getExpectedLength() const { return expected_length; }

  inline void setDeleteRequest(const bool status) { delete_request = status; }

  inline bool getDeleteRequest() const { return delete_request; }

  inline bool getCoherence() const { return coherence; }

  inline bool isFull() const { return expected_length <= current_length; }

  inline std::string getResourcePath() const { return resource_path; }

  inline void pushData(const std::pair<char *, size_t> new_data) {
    data.push_back(new_data);
    current_length += new_data.second;
  }

  inline size_t getDataPieceCounter() const { return data.size(); }

  inline std::pair<char *, size_t> at(size_t i) const { return data[i]; }

  ~Data();
};

class Client {
private:
  int socket;
  int resource_socket;
  PicoHttpParser *http_header;
  sockaddr_in resourceServerAddress;
  std::pair<Data *, size_t> data;

  bool connectResourceServer(const std::string &address, const in_port_t &port,
                             const bool &isIp);

  static std::pair<std::string, in_port_t>
  parseHostname(const std::string &host);

  static bool isDNSHostname(const std::string &host);

public:
  explicit Client(const int &socket);

  int readHttpHeader(const char *buffer = NULL, const size_t &length = 0);

  int proxyingData(); // get a data from the resource server

  int sendData(); // send a data to the client from the data structure

  Data *sendRequest(); // send the request to the resource server

  inline void setSendData(
      Data *send_data) { // set a data that would be used in sendData method
    data.first = send_data;
    data.second = 0;
  }

  inline int getResourceSocket() { return resource_socket; }

  std::string getRequestedResource() const;

  inline void closeSocket() { close(socket); }

  inline void closeResourceSocket() { close(resource_socket); }

  ~Client() { delete http_header; };
};