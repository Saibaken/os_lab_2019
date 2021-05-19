#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


#include <getopt.h>
#include <stdbool.h>

//#define SERV_PORT 10050
//#define BUFSIZE 100
#define SADDR struct sockaddr

int main(int argc, char *argv[]) {
  int buff_size = -1;
  int server_port = -1;

while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"buff_size", required_argument, 0, 0},
                                      {"server_port", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
        buff_size = atoi(optarg);
        if (buff_size <= 0)
        {
            printf("buff_size must be greater than zero.\n");
            return 1;
        }
        break;
      case 1:
        server_port = atoi(optarg);
        if (server_port <= 1024)
        {
            printf("server_port must be > 1024.\n");
            return 1;
        }
        break;
      default:
        printf("Index %d is out of options\n", option_index);
      }
    } break;

    case '?': break;
    default:
      fprintf(stderr, "getopt returned character code 0%o?\n", c);
    }
  }

  if (buff_size == -1 || server_port == -1) {
    fprintf(stderr, "Using: %s --buff_size 1024 --server_port 20001\n",
            argv[0]);
    return 1;
  }

  const size_t kSize = sizeof(struct sockaddr_in);

  int lfd, cfd;
  int nread;
  char buf[buff_size];
  struct sockaddr_in servaddr;
  struct sockaddr_in cliaddr;

  if ((lfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket");
    exit(1);
  }

  memset(&servaddr, 0, kSize);
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(server_port);

  if (bind(lfd, (SADDR *)&servaddr, kSize) < 0) {
    perror("bind");
    exit(1);
  }

  if (listen(lfd, 5) < 0) {
    perror("listen");
    exit(1);
  }

  while (1) {
    unsigned int clilen = kSize;

    if ((cfd = accept(lfd, (SADDR *)&cliaddr, &clilen)) < 0) {
      perror("accept");
      exit(1);
    }
    printf("connection established\n");

    while ((nread = read(cfd, buf, buff_size)) > 0) {
      write(1, &buf, nread);
    }

    if (nread == -1) {
      perror("read");
      exit(1);
    }
    close(cfd);
  }
}
