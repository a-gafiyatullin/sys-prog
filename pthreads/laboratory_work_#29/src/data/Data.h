#pragma once

#include <cstdlib>
#include <vector>
#include <stdexcept>

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
