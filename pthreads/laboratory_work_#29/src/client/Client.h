#pragma once

#include "../data/Data.h"
#include "../utility/PicoHttpRequest.h"
#include "../utility/PicoHttpResponse.h"
#include "../utility/network_utility.h"
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

class Client {
private:
  // connection info
  int socket;
  int resource_socket;
  sockaddr_in *resourceServerAddress;

  // resource info
  PicoHttpParser *http_header;
  std::pair<Data *, ssize_t> data;
  std::string requested_resource;
  int response_status;

  // current data exchange info (for send functions only)
  char *current_data_buffer;
  // end of the current_data_buffer from previous iteration
  ssize_t current_buffer_rest;
  ssize_t current_buffer_length;

  // object state info
  bool delete_request;

  static std::pair<std::string, in_port_t>
  parseHostname(const std::string &host);

  static bool isDNSHostname(const std::string &host);

public:
  explicit Client(const int &socket);

  int readHttpHeader();

  int getResource(); // get a data from the resource server

  int sendResource(); // send a data to the client from the data structure

  int sendRequest(); // send a request to the resource server

  // parse a response from the server and check the possibility of the caching
  std::pair<Data *, ssize_t> parseResponse();

  // set a data that would be used in sendData method
  inline void setSendData(Data *send_data) {
    data.first = send_data;
    data.second = 0;
  }

  inline int getResourceSocket() const { return resource_socket; }

  inline int getSocket() const { return socket; }

  inline std::string getRequestedResource() const {
    return requested_resource;
  };

  inline int getResponseStatus() const { return response_status; };

  inline void closeSocket() {
    close(socket);
    socket = -1;
  }

  inline void closeResourceSocket() {
    close(resource_socket);
    resource_socket = -1;
  }

  inline void setDeleteRequest(const bool &status) { delete_request = status; }

  inline bool getDeleteRequest() const { return delete_request; }

  int connectResourceServer();

  ~Client();
};