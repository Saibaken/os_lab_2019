#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>

#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <getopt.h>
#include <stdbool.h>

//#define SERV_PORT 20001
//#define BUFSIZE 1024
#define SADDR struct sockaddr
#define SLEN sizeof(struct sockaddr_in)

int main(int argc, char **argv) {
  int server_port = -1;
  int buff_size = -1;
  char ip[255];

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

    switch (c) {
    case 0: {
      switch (option_index) {
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

  if (!strlen(ip) || buff_size == -1 || server_port == -1) {
    fprintf(stderr, "Using: %s --ip 0.0.0.0 --buff_size 1024 --server_port 20001\n",
            argv[0]);
    return 1;
  }


  int sockfd, n;
  char sendline[buff_size], recvline[buff_size + 1];
  struct sockaddr_in servaddr;
  struct sockaddr_in cliaddr;

  //if (argc != 2) {
   // printf("usage: client <IPaddress of server>\n");
    //exit(1);
  //}

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(server_port);

  if (inet_pton(AF_INET, ip, &servaddr.sin_addr) < 0) {
    perror("inet_pton problem");
    exit(1);
  }
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket problem");
    exit(1);
  }

  write(1, "Enter string\n", 13);

  while ((n = read(0, sendline, buff_size)) > 0) {
    if (sendto(sockfd, sendline, n, 0, (SADDR *)&servaddr, SLEN) == -1) {
      perror("sendto problem");
      exit(1);
    }

    if (recvfrom(sockfd, recvline, buff_size, 0, NULL, NULL) == -1) {
      perror("recvfrom problem");
      exit(1);
    }

    printf("REPLY FROM SERVER= %s\n", recvline);
  }
  close(sockfd);
}
