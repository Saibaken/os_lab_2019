
#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

#include <signal.h>

void kill_all(int sig)         ///
{
    kill(0,SIGKILL);
    printf("timeout");
}

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  bool with_files = false;
  int timeout = -1;
  

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
                                      {"timeout", required_argument, 0, 0}, ///
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            // your code here
            // error handling
            if (seed < 0) 
            {
                printf("seed must be a positive number\n");
                return 1;
            }
            break;
          case 1:
            array_size = atoi(optarg);
            // your code here
            // error handling
            if (array_size<=0) 
            {
                printf("array_size must be greater than zero\n");
                return 1;
            }
            break;
          case 2:
            pnum = atoi(optarg);
            // your code here
            // error handling
            if (pnum <=0)
            {
                printf("pnum must be a positive number\n");
                return 1;
            }
            break;
          case 3:
            with_files = true;
            break;
          case 4:
            timeout = atoi(optarg);
            if (timeout <= 0) 
            {
                printf("timeout must be a positive number");
                return 1;
            }
            break;

          defalut:
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case 'f':
        with_files = true;
        break;

      case '?':
        break;

      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (optind < argc) {
    printf("Has at least one no option argument\n");
    return 1;
  }

  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" \n",
           argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);
    /*int i = 0;
    for (i = 0; i<array_size; i++)
    {
        printf("%d\n", array[i]);
    }*/
  
 
  int part_size = array_size / pnum; ///////
  FILE* tmp;
  int** pipefd;
  if (with_files)
  {
      tmp = fopen("tmp.txt", "w");
  }
  else pipefd = (int**)malloc(sizeof(int*)*pnum); /////////




  int active_child_processes = 0;


  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  for (int i = 0; i < pnum; i++) {
   
    ////////
    if (!with_files)
    {
        pipefd[i]=(int*)malloc(sizeof(int)*2);
        if (pipe(pipefd[i])<0)
        {
            printf("Error while making pipe");
            return 1;
        }
    }
    ////////
     pid_t child_pid = fork();

    if (child_pid >= 0) {
      // successful fork
      active_child_processes += 1;
      if (child_pid == 0) {
        // child process
        struct MinMax sub_min_max;
        // parallel somehow
        if (i != pnum - 1)
        {
            sub_min_max = GetMinMax(array, i*part_size, (i+1) * part_size);
        }
        else 
        {
            sub_min_max = GetMinMax(array, i*part_size, array_size);
        }
        if (with_files) {
          // use files here
            fwrite(&sub_min_max.min, sizeof(int), 1 , tmp);
            fwrite(&sub_min_max.max, sizeof(int), 1 , tmp);
        } else {
          // use pipe here
          write(pipefd[i][1], &sub_min_max.min, sizeof(int));
          write(pipefd[i][1], &sub_min_max.max, sizeof(int));
          close(pipefd[i][1]);
        }
        return 0;
      }

    } else {
      printf("Fork failed!\n");
      return 1;
    }
  }

  /////////
  if (with_files) 
  {
      fclose(tmp);
      tmp = fopen("tmp.txt", "r");
  }
/////////
  if(timeout>0)
    {
    signal(SIGALRM,kill_all);
    alarm(timeout);
    printf("Timeout is on %d \n", timeout);
    sleep(2);
    }
  while (active_child_processes > 0) 
  {
    // your code here
    wait(NULL);  

    active_child_processes -= 1;
  }

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++) {
    int min = INT_MAX;
    int max = INT_MIN;

    if (with_files) {
      // read from files
      fread(&min,sizeof(int),1,tmp);
      fread(&max,sizeof(int),1,tmp);
    } else {
      // read from pipefd
      read(pipefd[i][0],&min,sizeof(int));
      read(pipefd[i][0],&max,sizeof(int));    
      
      close(pipefd[i][0]);    

      free(pipefd[i]) ;
    }

    if (min < min_max.min) min_max.min = min;
    if (max > min_max.max) min_max.max = max;
  }

  if (with_files) {
      fclose(tmp);
      remove("tmp.txt");
    }
     else {
      free(pipefd);
    }
  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);

  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
  return 0;
}
