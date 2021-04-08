#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

static pthread_mutex_t mtx_first = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mtx_second = PTHREAD_MUTEX_INITIALIZER;


static void first_func() {
  printf("Executing first function...\n");
  pthread_mutex_lock(&mtx_first);
  sleep(2);
  pthread_mutex_lock(&mtx_second);
  printf("First in process...\n");
  pthread_mutex_unlock(&mtx_second);
  pthread_mutex_unlock(&mtx_first);
  printf("First function done\n");
}

static void second_func() {
  printf("Executing second function...\n");
  pthread_mutex_lock(&mtx_second);
  pthread_mutex_lock(&mtx_first);
  printf("Second in proces...\n");
  pthread_mutex_unlock(&mtx_first);
  pthread_mutex_unlock(&mtx_second);
  printf("Second function done\n");
}

int main(int argc, char* argv[]) {
  pthread_t t1, t2;

  if (pthread_create(&t1, NULL, (void*)first_func, NULL) != 0) {
    printf("Error: cannot create first thread\n");
    return -1;
  }
  if (pthread_create(&t2, NULL, (void*)second_func, NULL) != 0) {
    printf("Error: cannot create second thread\n");
    return -1;
  }

  if (pthread_join(t1, 0) != 0) {
    printf("Error: cannot join first thread\n");
    return -1;
  }

  if (pthread_join(t2, 0) != 0) {
    printf("Error: cannot join second thread\n");
    return -1;
  }
}