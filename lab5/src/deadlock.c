#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <getopt.h>
#include <pthread.h>

void *firstThreadFunction(void *);
void *secondThreadFunction(void *);
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char **argv) 
{
    pthread_t thread1, thread2;  
    if (pthread_create(&thread1, NULL, firstThreadFunction, NULL) != 0) {
        perror("pthread_create");
        exit(1);
    }
    if (pthread_create(&thread2, NULL, secondThreadFunction, NULL) != 0) {
        perror("pthread_create");
        exit(1);
    }
    if (pthread_join(thread1, NULL) != 0) {
        perror("pthread_join");
        exit(1);
    }
    if (pthread_join(thread2, NULL) != 0) {
        perror("pthread_join");
        exit(1);
    }
    return 0;
}

void *firstThreadFunction(void *args)
{
    pthread_mutex_lock(&mutex1);
    printf("Поток 1 заблокировал ресурс 1\n");
    sleep(1000);
    pthread_mutex_lock(&mutex2);
    printf("Поток 1 заблокировал ресурс 2\n");
    pthread_mutex_unlock(&mutex1);
    printf("Поток 1 разблокировал ресурс 1\n");
    pthread_mutex_unlock(&mutex2);
    printf("Поток 1 разблокировал ресурс 2\n");
}

void *secondThreadFunction(void *args)
{
    pthread_mutex_lock(&mutex2);
    printf("Поток 2 заблокировал ресурс 2\n");
    sleep(1000);
    pthread_mutex_lock(&mutex1);
    printf("Поток 2 заблокировал ресурс 1\n");
    pthread_mutex_unlock(&mutex2);
    printf("Поток 2 разблокировал ресурс 2\n");
    pthread_mutex_unlock(&mutex1);
    printf("Поток 2 разблокировал ресурс 1\n");
}