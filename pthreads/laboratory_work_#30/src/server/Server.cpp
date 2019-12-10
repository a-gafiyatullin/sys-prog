#include "Server.h"

Server *Server::instance = NULL;

Server::Server(const in_port_t &port)
    : server_socket(socket(AF_INET, SOCK_STREAM, 0)) {
  if (server_socket < 0) {
    throw std::out_of_range("Server::Server : socket < 0!");
  }
  if (port < 1024) {
    throw std::out_of_range("Server::Server : port < 1024!");
  }

  address.sin_family = AF_INET;
  address.sin_port = htons(port);
  address.sin_addr.s_addr = INADDR_ANY;
  if (bind(server_socket, (sockaddr *)&address, sizeof(sockaddr_in)) < 0) {
    throw std::domain_error("Server::Server : cannot bind port!");
  }
  if (::listen(server_socket, SOMAXCONN) < 0) {
    throw std::domain_error("Server::Server : cannot listen socket!");
  }

  if (pthread_mutex_init(&data_mutex, NULL) < 0) {
    throw std::domain_error("Server::Server : cannot init mutex!");
  }
  if (pthread_mutex_init(&clients_mutex, NULL) < 0) {
    throw std::domain_error("Server::Server : cannot init mutex!");
  }

  if (pthread_create(&update_thread, NULL, Server::updateStructuresAdapter,
                     new AdapterArgument(NULL, this)) < 0) {
    throw std::domain_error("Server::Server : cannot init update thread!");
  }

  if (pthread_cond_init(&wake_up_update_thread, NULL) < 0) {
    throw std::domain_error(
        "Server::Server : cannot init update thread wake up variable!");
  }
}

Server *Server::getInstance(const in_port_t &port) {
  if (instance == NULL) {
    instance = new Server(port);
  }

  return instance;
}

int Server::accept() {
  sockaddr_in remote;
  socklen_t sockaddr_in_len = sizeof(sockaddr_in);
  int client_socket;
  if ((client_socket = ::accept(server_socket, (sockaddr *)&remote,
                                &sockaddr_in_len)) < 0) {
    return client_socket;
  }
#ifdef DEBUG
  std::cerr << "New connection!" << std::endl;
#endif

  Client *client = new Client(client_socket);
  if (client == NULL) {
    return -1;
  }
  pthread_t *thread = new pthread_t;
  if (thread == NULL) {
    delete client;
    return -1;
  }
  if (pthread_create(thread, NULL, Server::downloadAdapter,
                     new AdapterArgument(client, this)) < 0) {
    delete client;
    delete thread;
    return -1;
  }

  pthread_mutex_lock(&clients_mutex);
  client_threads.insert(std::make_pair(client, thread));
  pthread_mutex_unlock(&clients_mutex);
#ifdef DEBUG
  std::cerr << "New connection is added successfully!" << std::endl;
#endif
  return client_socket;
}

Data *Server::getCachedResource(const std::string &url) {
  pthread_mutex_lock(&data_mutex);
  std::map<std::string, Data *>::const_iterator resource = cache.find(url);
  if (resource == cache.end()) {
    pthread_mutex_unlock(&data_mutex);
    return NULL;
  }
  pthread_mutex_unlock(&data_mutex);

  return resource->second;
}

Server::~Server() {
  pthread_cancel(update_thread);
  pthread_cond_signal(&wake_up_update_thread);
  pthread_join(update_thread, NULL);

  pthread_mutex_lock(&clients_mutex);
  for (std::map<Client *, pthread_t *>::iterator client =
           client_threads.begin();
       client != client_threads.end(); client++) {
    pthread_join(*client->second, NULL);
    delete client->second;
    delete client->first;
  }
  pthread_mutex_unlock(&clients_mutex);
  for (std::map<std::string, Data *>::iterator data = cache.begin();
       data != cache.end(); data++) {
    delete data->second;
  }
  pthread_mutex_destroy(&data_mutex);
  pthread_mutex_destroy(&clients_mutex);
  pthread_cond_destroy(&wake_up_update_thread);

  close(server_socket);
}

void Server::updateCache() {
  pthread_mutex_lock(&data_mutex);
  std::vector<std::map<std::string, Data *>::iterator> to_delete;
  for (std::map<std::string, Data *>::iterator data = cache.begin();
       data != cache.end(); data++) {
    if (data->second->getDeleteRequest()) {
      delete data->second;
      to_delete.push_back(data);
    }
  }
  for (size_t i = 0; i < to_delete.size(); i++) {
    cache.erase(to_delete[i]);
  }
  pthread_mutex_unlock(&data_mutex);
}

void *Server::download(void *client) {
  Client *client_info = static_cast<Client *>(client);
  // ignore SIGPIPE from sockets
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGPIPE);
  sigaddset(&mask, SIGINT);
  if (pthread_sigmask(SIG_BLOCK, &mask, NULL) < 0) {
    pthread_cond_signal(&wake_up_update_thread);
    pthread_exit((void *)-1);
  }

  int status = client_info->readHttpHeader();
  if (status == -1) {
#ifdef DEBUG
    std::cerr << "GET_REQUEST: Connection is rejected!" << std::endl;
#endif
    pthread_cond_signal(&wake_up_update_thread);
    pthread_exit((void *)-1);
  }
  if (status == 0) {
    std::string resource = client_info->getRequestedResource();
    if (PicoHttpRequest::isNone(resource)) {
#ifdef DEBUG
      std::cerr << "Nothing to download!" << std::endl;
#endif
      pthread_cond_signal(&wake_up_update_thread);
      pthread_exit((void *)-1);
    }
    Data *data = getCachedResource(resource);
    if (data != NULL) {
#ifdef DEBUG
      std::cerr << "Getting resource " << resource << " from cache."
                << std::endl;
#endif
      client_info->setSendData(data);
      pthread_cond_signal(&wake_up_update_thread);
      pthread_exit((void *)client_info->sendResource());
    } else {
      status = client_info->connectResourceServer();
      if (status < 0) {
#ifdef DEBUG
        std::cerr << "CONNECT: Connection is rejected!" << std::endl;
#endif
        pthread_cond_signal(&wake_up_update_thread);
        pthread_exit((void *)-1);
      } else if (status == 0) {
#ifdef DEBUG
        std::cerr << "CONNECT: Connection is successful!" << std::endl;
#endif
        status = client_info->sendRequest();
        if (status < 0) {
#ifdef DEBUG
          std::cerr << "SEND_REQUEST: Connection is rejected!" << std::endl;
#endif
          pthread_cond_signal(&wake_up_update_thread);
          pthread_exit((void *)-1);
        } else if (status == 0) {
          status = client_info->readHttpHeader();
          if (status == -1) {
#ifdef DEBUG
            std::cerr << "GET_RESPONSE: Connection is rejected!" << std::endl;
#endif
            pthread_cond_signal(&wake_up_update_thread);
            pthread_exit((void *)-1);
          } else if (status == 0) {
            std::pair<Data *, ssize_t> response = client_info->parseResponse();
            if (response.second == 0) {
              pthread_mutex_lock(&data_mutex);
              cache.insert(std::make_pair(client_info->getRequestedResource(),
                                          response.first));
              pthread_mutex_unlock(&data_mutex);
            }
#ifdef DEBUG
            std::cerr << "Start to download resource "
                      << client_info->getRequestedResource() << std::endl;
#endif
            pthread_t upload_thread;
            if (pthread_create(&upload_thread, NULL, &Server::uploadAdapter,
                               new AdapterArgument(client_info, this)) < 0) {
              pthread_exit((void *)-1);
            }

            client_info->getResource();
#ifdef DEBUG
            std::cerr << "Resource " << client_info->getRequestedResource()
                      << " is downloaded!" << std::endl;
#endif
            pthread_join(upload_thread, NULL);
            pthread_cond_signal(&wake_up_update_thread);
            pthread_exit((void *)0);
          }
        }
      }
    }
  }
}

void *Server::upload(void *client) {
  // ignore SIGPIPE and SIGINT
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGPIPE);
  sigaddset(&mask, SIGINT);
  if (pthread_sigmask(SIG_BLOCK, &mask, NULL) < 0) {
    pthread_exit((void *)-1);
  }

  Client *client_info = static_cast<Client *>(client);
  pthread_exit((void *)client_info->sendResource());
}

void *Server::updateStructures(void *arg) {
  // ignore SIGPIPE from sockets
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGPIPE);
  sigaddset(&mask, SIGINT);
  if (pthread_sigmask(SIG_BLOCK, &mask, NULL) < 0) {
    std::cerr << "Update Structures thread cannot start!" << std::endl;
    pthread_exit((void *)-1);
  }
  pthread_mutex_t mutex;
  if (pthread_mutex_init(&mutex, NULL) < 0) {
    std::cerr << "Update Structures thread cannot start!" << std::endl;
    pthread_exit((void *)-1);
  }
  while (true) {
    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&wake_up_update_thread, &mutex);
    pthread_mutex_unlock(&mutex);
#ifdef DEBUG
    std::cerr << "Update Structures thread is woken up!" << std::endl;
#endif

    pthread_testcancel(); // cancellation point

    std::vector<std::map<Client *, pthread_t *>::iterator> to_delete;

    pthread_mutex_lock(&clients_mutex);
    std::map<Client *, pthread_t *> tmp_client_threads = client_threads;
    pthread_mutex_unlock(&clients_mutex);

    for (std::map<Client *, pthread_t *>::iterator iter =
             tmp_client_threads.begin();
         iter != tmp_client_threads.end(); iter++) {
      pthread_join(*iter->second, NULL);
      delete iter->first;
      delete iter->second;
      to_delete.push_back(iter);
    }

    pthread_mutex_lock(&clients_mutex);
    for (size_t i = 0; i < to_delete.size(); i++) {
      client_threads.erase(to_delete[i]->first);
    }
    pthread_mutex_unlock(&clients_mutex);

    updateCache();
    pthread_testcancel(); // cancellation point
  }
}

void *Server::downloadAdapter(void *data) {
  AdapterArgument *argument = static_cast<AdapterArgument *>(data);
  void *ret = argument->server->download(argument->client);
  delete argument;
  pthread_exit(ret);
}

void *Server::uploadAdapter(void *data) {
  AdapterArgument *argument = static_cast<AdapterArgument *>(data);
  void *ret = argument->server->upload(argument->client);
  delete argument;
  pthread_exit(ret);
}

void *Server::updateStructuresAdapter(void *data) {
  AdapterArgument *argument = static_cast<AdapterArgument *>(data);
  void *ret = argument->server->updateStructures(NULL);
  delete argument;
  pthread_exit(ret);
}
