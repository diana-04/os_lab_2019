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

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  bool with_files = false;
  int pipefd[2];  //0 - чтение, 1 - запись
  
  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            if(seed == 0)
            {
              seed = 1;
            }
            break;
          case 1:
            array_size = atoi(optarg);
            if(array_size == 0)
            {
              array_size = 10;
            }
            break;
          case 2:
            pnum = atoi(optarg);
            if(pnum == 0)
            {
              pnum = 2;
            }
            break;
          case 3:
            with_files = true;
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
  /*for(int i = 0; i < array_size; i++)
  {
    printf("%d ", array[i]);
  }
  printf("\n");*/
  
  if(!with_files)
  {
    pipe(pipefd);
  }

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  int active_child_processes = 0;

  for (int i = 0; i < pnum; i++) 
  {
    pid_t child_pid = fork();
    if (child_pid >= 0)       // successful fork
    {         
      active_child_processes++;
      if (child_pid == 0)       // child process
      {     
        int imin = array_size / pnum * (active_child_processes - 1);
        int imax = array_size / pnum * (active_child_processes - 1);
        int highBoundary;
        if(i != pnum - 1)
        {
          highBoundary = array_size / pnum * active_child_processes;
        }
        else
        {
          highBoundary = array_size;
        }
        for(int j = imin + 1; j < highBoundary; j++)
        {
          if(array[j] < array[imin])
          {
            imin = j;
          }
          else if(array[j] > array[imax])
          {
            imax = j;
          }
        }
        if (with_files) {
          char fileName[20];
          snprintf(fileName, sizeof(fileName), "%d.txt", i);
          FILE *fp;
          if((fp = fopen(fileName, "w")) != NULL)
          {
            fprintf(fp, "%d\n%d", array[imin], array[imax]);
            fclose(fp);
          }
        } 
        else {
          close(pipefd[0]);
          write(pipefd[1], &array[imin], sizeof(int));
          write(pipefd[1], &array[imax], sizeof(int));
        }
        return 0;
      }
    } 
    else 
    {
      printf("Fork failed!\n");
      return 1;
    }
  }

  int status;
  while(active_child_processes)
  {
    wait(&status);
    active_child_processes--;
  }

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++) {
    int min = INT_MAX;
    int max = INT_MIN;
    int buf;
    close(pipefd[1]);
    if (with_files) 
    {
      FILE *fp;
      char fileName[20];
      snprintf(fileName, sizeof(fileName), "%d.txt", i);  
      if((fp = fopen(fileName, "r")) != NULL)
      {
        fscanf(fp, "%d", &buf);
        if(buf < min)
        {
          min = buf;
        }
        fscanf(fp, "%d", &buf);
        if(buf > max)
        {
          max = buf;
        }
        fclose(fp);
      }
      else
      {
        printf("Файл %s не открылся\n", fileName);
      }
      remove(fileName);
    } 
    else 
    {
      while(read(pipefd[0], &buf, sizeof(int)))
      {
        if(buf < min)
        {
          min = buf;
        }
        read(pipefd[0], &buf, sizeof(int));
        if(buf > max)
        {
          max = buf;
        }
      }
    }

    if (min < min_max.min) min_max.min = min;
    if (max > min_max.max) min_max.max = max;
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  gettimeofday(&start_time, NULL);
  int imin = 0;
  int imax = 0;
  for(int i = 0; i < array_size; i++)
  {
    if(array[i] < array[imin])
    {
      imin = i;
    }
    else if(array[i] > array[imax])
    {
      imax = i;
    }
  }
  gettimeofday(&finish_time, NULL);
  double elapsed_time1 = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time1 += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;
  free(array);

  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  printf("Elapsed time (однопроцессорная программа): %fms\n", elapsed_time1);
  fflush(NULL);
  return 0;
}
