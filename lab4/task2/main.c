#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void)
{
    int pid;
    int status;

    if ((pid = fork()) < 0) {
        perror("fork");
        exit(1);
    }

    /* Child */
    if (pid > 0){
        printf("Zombie pid=%d\n", pid);
        sleep(10);
    }
    else {
    exit(0);
    }

    return 0;
}