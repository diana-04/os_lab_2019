#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <getopt.h>
#include <pthread.h>
#include "../../lab3/src/utils.h"
#include "thread_sum.h"

int main(int argc, char **argv) 
{
  uint32_t threads_num = 0;
  uint32_t array_size = 0;
  uint32_t seed = 0;
  pthread_t *threads;  
  while (1) 
  {    
	int current_optind = optind ? optind : 1;
	static struct option options[] = {{"threads_num", required_argument, 0, 0},                                      
					                  {"array_size", required_argument, 0, 0}, 
                                      {"seed", required_argument, 0, 0},                                       
					                  {0, 0, 0, 0}};    
	int option_index = 0;    
	int c = getopt_long(argc, argv, "f", options, &option_index);    
	if (c == -1) 
	  break;    
	switch (c) 
	{      
	  case 0:        
	    switch (option_index) 
	    {          
	      case 0:            
	        threads_num = atoi(optarg);            
	        if(threads_num == 0)            
	        {              
	          threads_num = 2;            
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
	        seed = atoi(optarg);            
	        if(seed == 0)            
	        {              
	          seed = 1;            
	        }            
	        break;            
	      defalut:            
	        printf("Index %d is out of options\n", option_index);        
	      }        
	      break;     
	  case '?':        
	    break;      
	  default:        
	    printf("getopt returned character code 0%o?\n", c);    
	}  
  }  
  if (optind < argc) 
  {    
	printf("Has at least one no option argument\n");    
	return 1;  
  } 
  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed); 
  /*for(int i = 0; i < array_size; i++)
  {
	printf("%d ", array[i]);
  }
  printf("\n");*/
  struct SumArgs *args;
  threads = (pthread_t *)malloc(threads_num * sizeof(pthread_t));
  args = (struct SumArgs *)malloc(threads_num * sizeof(struct SumArgs));
  struct timeval start_time;  
  gettimeofday(&start_time, NULL);
  for (uint32_t i = 0; i < threads_num; i++) {
	args[i].begin = i * array_size / threads_num;
	if(i != threads_num - 1)        
	{          
	  args[i].end = (i + 1) * array_size / threads_num;        
	}        
	else        
	{          
	  args[i].end = array_size;        
	}
	args[i].array = array;
    if (pthread_create(&threads[i], NULL, ThreadSum, (void *)&args[i])) {
      printf("Error: pthread_create failed!\n");
      return 1;
    }
  }
  
  int total_sum = 0;
  for (int32_t i = threads_num - 1; i >= 0; i--) 
  {
    int sum = 0;
	pthread_join(threads[i], (void **)&sum);
    total_sum += sum;
  }
  struct timeval finish_time;  
  gettimeofday(&finish_time, NULL);  
  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;  
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;
  /*int check_sum = 0;
  for(int i = 0; i < array_size; i++)
  {
	check_sum += array[i];
  }*/

  free(array);
  free(threads);
  free(args);
  printf("Total: %d\nЗатраченное время: %lfms\n", total_sum, elapsed_time);
  return 0;
}
