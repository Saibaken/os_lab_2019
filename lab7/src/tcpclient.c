#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <getopt.h>
#include <stdbool.h>

//#define BUFSIZE 100
#define SADDR struct sockaddr
#define SIZE sizeof(struct sockaddr_in)

int main(int argc, char *argv[]) {
  char ip[255];
  int buff_size = -1;
  int server_port = -1;

while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"ip", required_argument, 0, 0},
                                      {"buff_size", required_argument, 0, 0},
                                      {"server_port", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c) 
    {
    case 0: {
      switch (option_index) 
      {
      case 0:
        strcpy(ip, optarg);
        break;
      case 1:
        buff_size = atoi(optarg);
        if (buff_size <= 0)
        {
            printf("buff_size must be greater than zero.\n");
            return 1;
        }
        break;
      case 2:
        server_port = atoi(optarg);
        if (server_port <= 1024)
        {
            printf("Port of the server must be > 1024.\n");
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

  if (!strlen(ip) || buff_size == -1 || server_port == -1) {
    fprintf(stderr, "Using: %s --ip 0.0.0.0 --buff_size 1024 --server_port 20001\n",
            argv[0]);
    return 1;
  }


  int fd;
  int nread;
  char buf[buff_size];
  struct sockaddr_in servaddr;
  //if (argc < 3) {
    //printf("Too few arguments \n");
    //exit(1);
  //}

  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket creating");
    exit(1);
  }

  memset(&servaddr, 0, SIZE);
  servaddr.sin_family = AF_INET;

  if (inet_pton(AF_INET, ip, &servaddr.sin_addr) <= 0) {
    perror("bad address");
    exit(1);
  }

  servaddr.sin_port = htons(server_port);

  if (connect(fd, (SADDR *)&servaddr, SIZE) < 0) {
    perror("connect");
    exit(1);
  }

  write(1, "Input message to send\n", 22);
  while ((nread = read(0, buf, buff_size)) > 0) {
    if (write(fd, buf, nread) < 0) {
      perror("write");
      exit(1);
    }
  }

  close(fd);
  exit(0);
}
