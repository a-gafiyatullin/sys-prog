#pragma once

#include "../utility/PicoHttpRequest.h"
#include "../utility/PicoHttpResponse.h"
#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdexcept>

class Client;

class Data {
private:
  std::vector<std::pair<char *, ssize_t> > data;
  bool coherence;          // true if data is downloaded
  bool delete_request;     // true if data is broken
  ssize_t expected_length; // Content-Length header value
  ssize_t current_length;
  pthread_cond_t empty;  // client's would be wait data if it's necessary
  pthread_mutex_t mutex; // protect data

public:
  Data();

  void setCoherence(const bool &status);

  void setExpectedLength(const ssize_t &length);

  ssize_t getExpectedLength();

  void setDeleteRequest(const bool &status);

  bool getDeleteRequest();

  bool isFull();

  void pushData(std::pair<char *, ssize_t> new_data);

  std::pair<char *, ssize_t> at(const ssize_t &i,
                                pthread_mutex_t *client_mutex);

  std::pair<char *, ssize_t> popFirst(pthread_mutex_t *client_mutex);

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

  // object state info
  pthread_mutex_t send_mutex; // mutex for waiting data

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

  inline std::string getRequestedResource() const {
    return requested_resource;
  };

  inline int getResponseStatus() const { return response_status; };

  int connectResourceServer();

  ~Client();
};