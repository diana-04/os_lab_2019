#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "pthread.h"

struct Server {
  char ip[255];
  int port;
  uint64_t begin;
  uint64_t end;
  uint64_t mod;
};

bool ConvertStringToUI64(const char *str, uint64_t *val) {
  char *end = NULL;
  unsigned long long i = strtoull(str, &end, 10);
  if (errno == ERANGE) {
    fprintf(stderr, "Out of uint64_t range: %s\n", str);
    return false;
  }

  if (errno != 0)
    return false;

  *val = i;
  return true;
}

void *connectToServer(void *args) {
  struct Server *server_args = (struct Server*)args;
  struct hostent *hostname = gethostbyname(server_args->ip);
  if (hostname == NULL) {
    fprintf(stderr, "gethostbyname failed with %s\n", server_args->ip);
    exit(1);
  }

  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = htons(server_args->port);
  server.sin_addr.s_addr = *((unsigned long *)hostname->h_addr);

  int sck = socket(AF_INET, SOCK_STREAM, 0);
  if (sck < 0) {
    fprintf(stderr, "Socket creation failed!\n");
    exit(1);
  }

  if (connect(sck, (struct sockaddr *)&server, sizeof(server)) < 0) {
    fprintf(stderr, "Connection failed\n");
    exit(1);
  }
  
  char task[sizeof(uint64_t) * 3];
  memcpy(task, &(server_args->begin), sizeof(uint64_t));
  memcpy(task + sizeof(uint64_t), &(server_args->end), sizeof(uint64_t));
  memcpy(task + 2 * sizeof(uint64_t), &(server_args->mod), sizeof(uint64_t));

  if (send(sck, task, sizeof(task), 0) < 0) {
    fprintf(stderr, "Send failed\n");
    exit(1);
  }

  char response[sizeof(uint64_t)];
  if (recv(sck, response, sizeof(response), 0) < 0) {
    fprintf(stderr, "Recieve failed\n");
    exit(1);
  }

  int64_t flag = -1;
  memcpy(task, &flag, sizeof(int64_t));
  if (send(sck, task, sizeof(task), 0) < 0) {
    fprintf(stderr, "Send end signal failed\n");
    exit(1);
  }

  uint64_t answer = 0;
  memcpy(&answer, response, sizeof(uint64_t));

  close(sck);

  return (void *)(uint64_t *)answer;
}

int main(int argc, char **argv) {
  uint64_t k = 0;
  uint64_t mod = 0;
  char servers[255] = {'\0'}; 
  pthread_t *threads;  

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"k", required_argument, 0, 0},
                                      {"mod", required_argument, 0, 0},
                                      {"servers", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
        if (!ConvertStringToUI64(optarg, &k)) {
          k = 6;
        }
        break;
      case 1:
        if (!ConvertStringToUI64(optarg, &mod)) {
          mod = 200;
        }
        break;
      case 2:
        memcpy(servers, optarg, strlen(optarg));
        break;
      default:
        printf("Index %d is out of options\n", option_index);
      }
    } break;

    case '?':
      printf("Arguments error\n");
      break;
    default:
      fprintf(stderr, "getopt returned character code 0%o?\n", c);
    }
  }

  if (k == 0 || mod == 0 || !strlen(servers)) {
    fprintf(stderr, "Using: %s --k 1000 --mod 5 --servers /path/to/file\n", argv[0]);
    return 1;
  }

  unsigned int servers_num = 0;
  FILE *file;
  struct Server *to = NULL;
  if ((file =fopen(servers, "r")) != NULL) {
    size_t len = 0;
    char *buf = NULL;
    while (getline(&buf, &len, file) != -1) {
      servers_num++;
    }
    free(buf);
    fseek(file, 0, SEEK_SET);
    to = malloc(sizeof(struct Server) * servers_num);
    threads = (pthread_t *)malloc(servers_num * sizeof(pthread_t));
    for (unsigned int i = 0; i < servers_num; i++) {
      fscanf(file, "%d %s", &to[i].port, to[i].ip);
    }
    fclose(file);
  }
  else {
    fprintf(stderr, "Не удалось открыть файл с информацией о серверах\n");
    exit(1);
  }

  for (int i = 0; i < servers_num; i++) {
    to[i].begin = i * k / servers_num + 1;
    if(i != servers_num - 1) {          
	    to[i].end = (i + 1) * k / servers_num + 1;        
	  }        
	  else {          
	    to[i].end = k + 1;        
	  }
    to[i].mod = mod;
    if (pthread_create(&threads[i], NULL, connectToServer, (void *)&to[i])) {
      printf("Error: pthread_create failed!\n");
      exit(1);
    }
  }
  uint64_t result = 1;
  uint64_t buffer;
  for (int32_t i = servers_num - 1; i >= 0; i--) {
	  pthread_join(threads[i], (void **)&buffer);
    result = (result * buffer) % mod;
  }
  printf("Result: %lu\n", result);
  free(to);
  free(threads);
  remove(servers);
  return 0;
}
