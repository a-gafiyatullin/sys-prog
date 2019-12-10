#pragma once

#include "../client/Client.h"
#include <algorithm>
#include <csignal>
#include <map>
#include <stdexcept>

class Server {
private:
  static Server *instance;
  int server_socket;
  sockaddr_in address;

  pthread_mutex_t data_mutex;
  pthread_mutex_t clients_mutex;

  std::map<Client *, pthread_t *> client_threads;
  std::map<std::string, Data *> cache;

  pthread_t update_thread;
  pthread_cond_t wake_up_update_thread;

  struct AdapterArgument {
    Server *server;
    Client *client;
    AdapterArgument(Client *client, Server *server)
        : client(client), server(server) {}
  };

  explicit Server(const in_port_t &port);

  Data *getCachedResource(const std::string &url);

  static void *downloadAdapter(void *data);
  void *download(void *client);

  static void *uploadAdapter(void *data);
  static void *upload(void *client);

  static void *updateStructuresAdapter(void *data);
  void *updateStructures(void *arg); // update client_threads and cache

  void updateCache(); // delete bad cache entries

public:
  int accept();

  static Server *getInstance(const in_port_t &port);

  ~Server();
};