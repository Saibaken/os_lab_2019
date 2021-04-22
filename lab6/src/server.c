#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <getopt.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "pthread.h"
#include "libr.h"

struct FactorialArgs {
  uint64_t begin;
  uint64_t end;
  uint64_t mod;
};

uint64_t Factorial(const struct FactorialArgs *args) {
  uint64_t ans = 1;
  
  // TODO: your code here
  
  int i;
  for (i = args->begin; i <= args->end; i++)
  {
      ans = MultModulo(ans, i, args->mod);
  }

  return ans;
}

void *ThreadFactorial(void *args) {
  struct FactorialArgs *fargs = (struct FactorialArgs *)args;
  return (void *)(uint64_t *)Factorial(fargs);
}

int main(int argc, char **argv) {
  int tnum = -1;
  int port = -1;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"port", required_argument, 0, 0},
                                      {"tnum", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
        port = atoi(optarg);
        // TODO: your code here
        if (port < 0)
        {
            printf("Error: bad port value\n");
            return 1;
        }
       // FILE* file;
      //  bool correct = true;
       // if ((file = fopen("ports_list.txt", "r")) != NULL)
        //{
          //  while (getc(file) != EOF)
            //{
              //  fseek(file, -1, SEEK_CUR);
                //char buff[30];
               // fgets(buff, 29, file);
               // int read_port = atoi(buff);
                //if (read_port == port)
               // {
                 //   correct = false;
               // }
            //}
     //   }
       // if (!correct)
      //  {
      //      printf("Server with this port is already exist. Input another port.\n");
      //      return 1;
     //   }
        break;
      case 1:
        tnum = atoi(optarg);
        // TODO: your code here
        if (tnum <= 0)
        {
            printf("Error: bad tnum value\n");
            return 1;
        }
        break;
      default:
        printf("Index %d is out of options\n", option_index);
      }
    } break;

    case '?':
      printf("Unknown argument\n");
      break;
    default:
      fprintf(stderr, "getopt returned character code 0%o?\n", c);
    }
  }

  if (port == -1 || tnum == -1) {
    fprintf(stderr, "Using: %s --port 20001 --tnum 4\n", argv[0]);
    return 1;
  }

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    fprintf(stderr, "Can not create server socket!\n");
    return 1;
  }

  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = htons((uint16_t)port);
  server.sin_addr.s_addr = htonl(INADDR_ANY);

  int opt_val = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));

  int err = bind(server_fd, (struct sockaddr *)&server, sizeof(server));
  if (err < 0) {
    fprintf(stderr, "Can not bind to socket!\n");
    return 1;
  }

  err = listen(server_fd, 128);
  if (err < 0) {
    fprintf(stderr, "Could not listen on socket\n");
    return 1;
  }

  printf("Server listening at %d\n", port);

  // Запись в файл для чтения портов клиентом
  //FILE* file;
 // if ((file = fopen("ports_list.txt", "a")) != NULL)
 // {
  //  char buff[30];
  //  sprintf(buff, "%d", port);
  //  fputs(buff, file);
  //  fputc('\n', file);
  //  fclose(file);
 // }
  //else
  //{
  //  fprintf(stderr, "Error with opening file \"prots_lsit.txt\"\n");
   // return 1;
  //}
  
  while (true) {
    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);
    int client_fd = accept(server_fd, (struct sockaddr *)&client, &client_len);

    if (client_fd < 0) {
      fprintf(stderr, "Could not establish new connection\n");
      continue;
    }

    while (true) {
      unsigned int buffer_size = sizeof(uint64_t) * 3;
      char from_client[buffer_size];
      int read = recv(client_fd, from_client, buffer_size, 0);

      if (!read)
        break;
      if (read < 0) {
        fprintf(stderr, "Client read failed\n");
        break;
      }
      if (read < buffer_size) {
        fprintf(stderr, "Client send wrong data format\n");
        break;
      }

      pthread_t threads[tnum];

      uint64_t begin = 0;
      uint64_t end = 0;
      uint64_t mod = 0;
      memcpy(&begin, from_client, sizeof(uint64_t));
      memcpy(&end, from_client + sizeof(uint64_t), sizeof(uint64_t));
      memcpy(&mod, from_client + 2 * sizeof(uint64_t), sizeof(uint64_t));

      fprintf(stdout, "Receive: %llu %llu %llu\n", begin, end, mod);
      
      // Определение необходимого количества потоков
      int iterations = end - begin + 1;
      int needed_threads = iterations < tnum ? iterations : tnum;       //Если вышло, что элементов меньше, чем потоков

      // Распределение по потокам
      int part;
      if (tnum >= iterations)
      {
        part = 1;
      }
      else
      {
          part = (int)((float)iterations / (float)tnum);
      }

      int count = begin;

      struct FactorialArgs args[needed_threads];
      for (uint32_t i = 0; i < needed_threads; i++) {
        // TODO: parallel somehow
        args[i].begin = count;
        args[i].end = count + part <= end ? count + part : end;
        args[i].mod = mod;

        if (pthread_create(&threads[i], NULL, ThreadFactorial,
                           (void *)&args[i])) {
          printf("Error: pthread_create failed!\n");
          return 1;
        }

        count = count + part + 1;
      }

      uint64_t total = 1;
      for (uint32_t i = 0; i < needed_threads; i++) {
        uint64_t result = 0;
        pthread_join(threads[i], (void **)&result);
        total = MultModulo(total, result, mod);
      }

      printf("Total: %llu\n", total);

      char buffer[sizeof(total)];
      memcpy(buffer, &total, sizeof(total));
      err = send(client_fd, buffer, sizeof(total), 0);
      if (err < 0) {
        fprintf(stderr, "Can't send data to client\n");
        break;
      }
    }

    shutdown(client_fd, SHUT_RDWR);
    close(client_fd);
  }

  return 0;
}