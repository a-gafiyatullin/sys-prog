#include "Data.h"

Data::~Data() {
  for (size_t i = 0; i < data.size(); i++) {
    delete[] data[i].first;
  }
}

std::pair<char *, ssize_t> Data::popFirst() {
  if (data.empty()) {
    return std::make_pair((char *)NULL, -1);
  }
  std::pair<char *, ssize_t> first_data = data.front();
  data.erase(data.begin());

  return first_data;
}