#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "libr.h"         ///
#include <pthread.h>

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;       ///

struct sockaddr_in create_sockaddr(uint16_t port, uint32_t s_addr);

struct fac_args {
  uint64_t begin;
  uint64_t end;
  uint64_t mod;
};

struct fac_server {
  /* External */
  char ip[255];
  int port;

  /* Internal */
  pthread_t thread;
  int socket;
  struct fac_args args;
};

struct fac_server_list {
  struct fac_server server;
  struct fac_server_list* next;
};

struct client_data {
  struct fac_args start_args;
  struct fac_server_list* servers_list;
  uint32_t servers_num;
  uint64_t res;
} client;

struct Server {
  char ip[255];
  int port;
};

static void server_recieve_task(struct fac_server* server) {
  int socket = server->socket;
  char response[sizeof(uint64_t)];
  if (recv(socket, response, sizeof(response), 0) < 0) {
    fprintf(stderr, "Recieve failed\n");
    return;
  }
  close(socket);

  uint64_t answer = 0;
  memcpy(&answer, response, sizeof(uint64_t));
  printf("Answer: %llu\n", answer);

  pthread_mutex_lock(&mtx);
  client.res = MultModulo(client.res, answer, client.start_args.mod);
  pthread_mutex_unlock(&mtx);
}

static void finalize_tasks() {
 struct fac_server_list* iter = client.servers_list;
  for (int i = 0; iter != NULL; ) {
    if (i < client.servers_num)
      if (pthread_join(iter->server.thread, NULL) != 0) {
        printf("Error: cannot join %d\n", iter->server.thread);
        exit(1);
      }
    printf("Thread %d joined\n", iter->server.thread);
    struct fac_server_list* prev = iter;
    iter = iter->next;
    free(prev);
    i++;
  }
}

static struct fac_server_list* read_servers_file(const char* filename, int* len) {
  if (access(filename, F_OK) == -1) {
    printf("Error: file %s does not exist\n", filename);
    return 0;
  }

  FILE* file = fopen(filename, "r");
  if (!file) {
    printf("Error: cannot open file %s\n", filename);
    return 0;
  }

  struct fac_server_list* first = NULL;
  int i;
  for (i = 0 ;; ++i) {
    struct fac_server_list* head = (struct fac_server_list*)malloc(sizeof(struct fac_server_list));
    head->next = NULL;
    int res = fscanf(file, "%s : %d", head->server.ip, &head->server.port);
    if (res != 2) {
      free(head);
      if (res == EOF)  /* No more strings */
        break;
      fclose(file);  /* Else error occured */
      return 0;
    }
    if (!first)
      first = head;
    else {
      head->next = first->next;
      first->next = head;
    }
  }

  fclose(file);
  *len = i;
  return first;
}


bool ConvertStringToUI64(const char *str, uint64_t *val) {
  char *end = NULL;
  unsigned long long i = strtoull(str, &end, 10);
  if (errno == ERANGE) {
    fprintf(stderr, "Out of uint64_t range: %s\n", str);
    return false;
  }

  if (errno != 0)
    return false;

  *val = i;
  return true;
}

int main(int argc, char **argv) {
  uint64_t k = -1;
  uint64_t mod = -1;
  char servers[255] = {'\0'}; // TODO: explain why 255

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"k", required_argument, 0, 0},
                                      {"mod", required_argument, 0, 0},
                                      {"servers", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
        ConvertStringToUI64(optarg, &k);
        // TODO: your code here
        if (k <= 0)
            {
              printf("k must be a positive number\n");
              return 1;
            }
        break;
      case 1:
        ConvertStringToUI64(optarg, &mod);
        // TODO: your code here
        if (mod <= 0)
            {
              printf("mod must be a positive number\n");
              return 1;
            }
        break;
      case 2:
        // TODO: your code here
        memcpy(servers, optarg, strlen(optarg));
        if (strlen(servers) == sizeof('\0'))
            {
              printf("servers must be a path to file with ip:port\n");
              return 1;
            }
        break;
      default:
        printf("Index %d is out of options\n", option_index);
      }
    } break;

    case '?':
      printf("Arguments error\n");
      break;
    default:
      fprintf(stderr, "getopt returned character code 0%o?\n", c);
    }
  }

  if (k == -1 || mod == -1 || !strlen(servers)) {
    fprintf(stderr, "Using: %s --k 1000 --mod 5 --servers /path/to/file\n",
            argv[0]);
    return 1;
  }
  client.start_args.begin = 1;
  client.start_args.end = k+1;
  client.start_args.mod = mod;

  // Получаем список серверов
  if ((client.servers_list = read_servers_file(servers, &client.servers_num)) == 0) {
    printf("Error: cannot read servers file\n");
    return -1;
  }

  if (client.servers_num > k / 2) {
    client.servers_num = k / 2;
    printf("Warning: too much servers. Continue with %d\n", client.servers_num);
  }
#ifdef VERBOSE
  printf("Got server list, len=%d\n", servers_num);
#endif

  //Отправляем данные и ждем результатов
  float block = (float)k / client.servers_num;
  struct fac_server_list* servers_list_item = client.servers_list;
  for (int i = 0; i < client.servers_num; i++) {

    struct fac_server* server = &servers_list_item->server;
    /* Prepare package */
    server->args.begin = (int)(block * (float)i) + 1;
    server->args.end = (int)(block * (i + 1.f)) + 1;
    server->args.mod = mod;

    // Подготовка сокета
       struct sockaddr_in server_sockaddr;
         server_sockaddr.sin_family = AF_INET;
         server_sockaddr.sin_port = htons(server->port);
         server_sockaddr.sin_addr.s_addr = *((unsigned long *)0);
    if (!inet_aton(server->ip, &server_sockaddr.sin_addr)) {
      printf("Error: cannot translate %s into int value\n", server->ip);
      return -1;
    }

    // Сокет для каждого сервера
    int sck = socket(AF_INET, SOCK_STREAM, 0);
    if (sck < 0) {
      fprintf(stderr, "Socket creation failed!\n");
      exit(1);
    }
    server->socket = sck;
#ifdef VERBOSE
    printf("Socket [%s:%d] created\n", server->ip, server->port);
#endif

    // Коннектит сокет sck к address серверу */
    if (connect(sck, (struct sockaddr *)&server_sockaddr, sizeof(struct sockaddr_in)) < 0) {
      fprintf(stderr, "Connection failed\n");
      exit(1);
    }

    // Отправляем данные
    if (send(sck, &server->args, sizeof(struct fac_args), 0) < 0) {
      fprintf(stderr, "Send failed\n");
      exit(1);
    }
#ifdef VERBOSE
    printf("Data sent\n");
#endif

    //Выполняем задание и получаем данные
    if (pthread_create(&server->thread, NULL, (void*)server_recieve_task, (void*)server) != 0) {
      printf("Error: cannot create thread to recieve from server [%s:%p]\n", server->ip, server->port);
      return -1;
    }

    //Итерируем список
    servers_list_item = servers_list_item->next;
  }

  finalize_tasks();
  printf("Result: %lu\n", client.res);
  return 0;
}