#pragma once

#include <cstdlib>
#include <pthread.h>
#include <stdexcept>
#include <vector>

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
