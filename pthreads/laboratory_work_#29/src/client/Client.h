#pragma once

#include "../utility/PicoHttpRequest.h"
#include "../utility/PicoHttpResponse.h"
#include "../utility/network_utility.h"
#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>

class Client;

class Data {
private:
  std::vector<std::pair<char *, ssize_t> > data;
  bool coherence;          // true if data is downloaded
  bool delete_request;     // true if data is broken
  ssize_t expected_length; // Content-Length header value
  ssize_t current_length;

public:
  Data()
      : coherence(false), expected_length(0), current_length(0),
        delete_request(false){};

  inline void setCoherence(const bool &status) { coherence = status; }

  inline void setExpectedLength(const ssize_t &length) {
    expected_length = length;
  }

  inline ssize_t getExpectedLength() const { return expected_length; }

  inline void setDeleteRequest(const bool &status) { delete_request = status; }

  inline bool getDeleteRequest() const { return delete_request; }

  inline bool getCoherence() const { return coherence; }

  inline bool isFull() const { return expected_length <= current_length; }

  inline void pushData(const std::pair<char *, ssize_t> new_data) {
    data.push_back(new_data);
    current_length += new_data.second;
  }

  inline ssize_t getDataPieceCounter() const { return data.size(); }

  inline std::pair<char *, ssize_t> at(const ssize_t &i) const {
    return data[i];
  }

  std::pair<char *, ssize_t> popFirst();

  ~Data();
};

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

  // current data exchange info
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