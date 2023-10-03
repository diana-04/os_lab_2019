#include <ctype.h>
#include <limits.h>
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
    char *argv[4] = {"sequential", "2", "10", NULL};
    int pid = fork();
    if(!pid)
    {
        execv("sequential", argv);
    }
    int status;
    wait(&status);
    return 0;
}