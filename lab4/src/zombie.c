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

int main()
{
    pid_t child_pid = fork();
    if(!child_pid)
    {
        printf("Процесс-ребенок\n");
    }
    else if(child_pid > 0)
    {
        printf("Процесс-родитель\n");
        sleep(15);
        printf("Родитель после паузы\n");
    }
    return 0;
}