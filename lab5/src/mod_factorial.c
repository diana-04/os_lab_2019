#include <ctype.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <getopt.h>
#include <pthread.h>

typedef struct args
{
    uint32_t begin;
    uint32_t end;
    uint32_t mod;
    uint32_t *result;
} Args;

void *mod_factorial(void *);
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char **argv) 
{
    uint32_t pnum = 0;
    uint32_t k = 0;
    uint32_t mod = 0;
    uint32_t result = 1;
    pthread_t *threads;  
    while (1) 
    {    
	    int current_optind = optind ? optind : 1;
	    static struct option options[] = {{"pnum", required_argument, 0, 0}, 
                                          {"mod", required_argument, 0, 0},                                       
					                      {0, 0, 0, 0}};    
	    int option_index = 0;    
	    int c = getopt_long(argc, argv, "k:", options, &option_index);    
	    if (c == -1) 
	        break;    
	    switch (c) 
	    {      
	        case 0:        
	            switch (option_index) 
	            {          
	                case 0:            
	                    pnum = atoi(optarg);            
	                    if(pnum == 0)            
	                    {              
	                        pnum = 4;            
	                    }            
	                    break;          
	                case 1:            
	                    mod = atoi(optarg);            
	                    if(mod == 0)            
	                    {              
	                        mod = 10;            
	                    }            
	                    break;    
	                defalut:            
	                    printf("Index %d is out of options\n", option_index);        
	            }        
	            break;  
            case 'k':
                k = atoi(optarg);
                if(k == 0)
                {
                    k = 5;
                }
                break;
	        case '?':        
	            break;      
	        default:        
	            printf("getopt returned character code 0%o?\n", c);    
	    }  
    }  
    printf("%u %u %u\n", k, pnum, mod);
    fflush(stdout);
    if (optind < argc) 
    {    
	    printf("Has at least one no option argument\n");    
	    return 1;  
    } 
    threads = (pthread_t *)malloc(pnum * sizeof(pthread_t));
    Args *arguments = (Args *)malloc(pnum * sizeof(Args));
    for (uint32_t i = 0; i < pnum; i++) {
        arguments[i].begin = k / pnum * i + 1;
        if(i != pnum - 1)
        {
            arguments[i].end = k / pnum * (i + 1) + 1;
        }
        else 
        {
            arguments[i].end = k + 1;
        }
        arguments[i].mod = mod;
        arguments[i].result = &result;
	    if (pthread_create(&threads[i], NULL, mod_factorial, (void *)&arguments[i])) {
            printf("Error: pthread_create failed!\n");
            return 1;
        }
    }
    for (int32_t i = pnum - 1; i >= 0; i--) 
    {
	    pthread_join(threads[i], NULL);
    }
    printf("Значение %u! %% %u равно %u\n", k, mod, result);
    if(threads)
    {
        free(threads);
    }
    if(arguments)
    {
        free(arguments);
    }
    return 0;
}

void *mod_factorial(void *void_arguments)
{
    Args *arguments = (Args *)void_arguments;
    for(uint32_t i = arguments->begin; i < arguments->end; i++)
    {
        pthread_mutex_lock(&mut);
        *(arguments->result) = ((*(arguments->result)) * i) % arguments->mod;
        pthread_mutex_unlock(&mut);
    }
}