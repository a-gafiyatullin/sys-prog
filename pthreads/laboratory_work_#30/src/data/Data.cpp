#include "Data.h"

Data::~Data() {
  pthread_mutex_lock(&mutex);
  for (size_t i = 0; i < data.size(); i++) {
    delete[] data[i].first;
  }
  pthread_cond_destroy(&empty);
  pthread_mutex_unlock(&mutex);
  pthread_mutex_destroy(&mutex);
}

std::pair<char *, ssize_t> Data::popFirst(pthread_mutex_t *client_mutex) {
  while (true) {
    pthread_mutex_lock(&mutex);
    if (data.empty() && !coherence) {
      pthread_mutex_unlock(&mutex);
      pthread_mutex_lock(client_mutex);
      pthread_cond_wait(&empty, client_mutex); // wait data
      pthread_mutex_unlock(client_mutex);
    } else if (data.empty() && coherence) {
      pthread_mutex_unlock(&mutex);
      return std::make_pair((char *)NULL, -1);
    } else {
      break;
    }
  }

  std::pair<char *, ssize_t> first_data = data.front();
  data.erase(data.begin());
  pthread_mutex_unlock(&mutex);

  return first_data;
}

Data::Data()
    : coherence(false), expected_length(0), current_length(0),
      delete_request(false) {
  if (pthread_cond_init(&empty, NULL) < 0) {
    throw std::domain_error("Data::Data : can't init conditional variable!");
  }
  if (pthread_mutex_init(&mutex, NULL) < 0) {
    throw std::domain_error("Data::Data : can't init mutex!");
  }
}

void Data::pushData(const std::pair<char *, ssize_t> new_data) {
  pthread_mutex_lock(&mutex);
  data.push_back(new_data);
  current_length += new_data.second;
  pthread_mutex_unlock(&mutex);
  pthread_cond_broadcast(&empty);
}

std::pair<char *, ssize_t> Data::at(const ssize_t &i,
                                    pthread_mutex_t *client_mutex) {
  while (true) {
    pthread_mutex_lock(&mutex);
    if (i >= data.size() && !coherence) {
      pthread_mutex_unlock(&mutex);
      pthread_mutex_lock(client_mutex);
      pthread_cond_wait(&empty, client_mutex); // wait new data
      pthread_mutex_unlock(client_mutex);
    } else if (i >= data.size() && coherence) {
      pthread_mutex_unlock(&mutex);
      return std::make_pair((char *)NULL, -1);
    } else {
      break;
    }
  }

  std::pair<char *, ssize_t> last_data = data[i];
  pthread_mutex_unlock(&mutex);

  return last_data;
}

void Data::setCoherence(const bool &status) {
  pthread_mutex_lock(&mutex);
  coherence = status;
  pthread_mutex_unlock(&mutex);
}

void Data::setExpectedLength(const ssize_t &length) {
  pthread_mutex_lock(&mutex);
  expected_length = length;
  pthread_mutex_unlock(&mutex);
}

ssize_t Data::getExpectedLength() {
  pthread_mutex_lock(&mutex);
  ssize_t expected_len = expected_length;
  pthread_mutex_unlock(&mutex);

  return expected_len;
}

void Data::setDeleteRequest(const bool &status) {
  pthread_mutex_lock(&mutex);
  delete_request = status;
  pthread_mutex_unlock(&mutex);
}

bool Data::getDeleteRequest() {
  pthread_mutex_lock(&mutex);
  bool delete_req = delete_request;
  pthread_mutex_unlock(&mutex);

  return delete_req;
}

bool Data::isFull() {
  pthread_mutex_lock(&mutex);
  bool is_full = expected_length <= current_length;
  pthread_mutex_unlock(&mutex);

  return is_full;
}
