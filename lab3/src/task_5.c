#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>


int main(int argc, char* argv[]){
    pid_t child_pid = fork();
    if (child_pid<0)
    {
        printf("Error");
        return 1;
    }
    else{
        if (child_pid==0)
        execv("sequential_min_max",argv);
        wait(NULL);
        return 0;
    } 
    return 0;
}