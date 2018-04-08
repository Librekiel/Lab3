#include <stdio.h>

int main(int argc, int *argv[])
{
    int pid = fork();
    
    if (pid==0)
    {
        execv("sequential_min_max",argv);
    }
    
    wait(2);

    return 0;
}