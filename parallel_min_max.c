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
            // your code here
            if (seed<0)
            {
                printf("Error: seed<0\n");
                return 0;
            }
            // error handling
            break;
          case 1:
            array_size = atoi(optarg);
            // your code here
            if (array_size<=0)
            {
                printf("Error: array_size<0\n");
                return 0;
            }
            // error handling
            break;
          case 2:
            pnum = atoi(optarg);
            // your code here
            if (pnum<=0)
            {
                printf("Error: pnum<0\n");
                return 0;
            }
            // error handling
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
  int active_child_processes = 0;

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  int **min_pipe = malloc(sizeof(int*)*pnum);
  int **max_pipe = malloc(sizeof(int*)*pnum);
  
  for (int i=0; i<pnum; i++)
  {
      min_pipe[i] = malloc(sizeof(int)*2);
      pipe(min_pipe[i]);
      max_pipe[i] = malloc(sizeof(int)*2);
      pipe(max_pipe[i]);
  }

  for (int i = 0; i < pnum; i++) {
    pid_t child_pid = fork();
    if (child_pid >= 0) {
      // successful fork
      active_child_processes += 1;
      if (child_pid == 0) {
        // child process
        struct MinMax ChildProc = GetMinMax(array,0,array_size-1);
        // parallel somehow

        if (with_files) {
          // use files here
          char filename[16];
          sprintf(filename,"%d",i);
          FILE *fp;
          fp=fopen(filename,"w");
          fprintf(fp,"%f %f.",ChildProc.min, ChildProc.max);
          
        } else {
          // use pipe here
          char min_str[16];
          sprintf(min_str,"%d",ChildProc.min);
          write(min_pipe[i][1],min_str,strlen(min_str));
          char max_str[16];
          sprintf(max_str,"%d",ChildProc.max);
          write(max_pipe[i][1],max_str,strlen(max_str));
          
        }
        return 0;
      }

    } else {
      printf("Fork failed!\n");
      return 1;
    }
  }

  while (active_child_processes > 0) {
    // your code here
    
    
    active_child_processes -= 1;
  }

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++) {
    int min = INT_MAX;
    int max = INT_MIN;

    if (with_files) {
      // read from files
      char *buffer[16];
      char *filename[16];
      sprintf(filename,"%d",i);
      FILE *fp;
      fp=fopen(filename,"r");
      fscanf(fp,buffer);
      char nums[2][8];
      int k=0;
      for (int j=0; j<strlen(buffer); j++)
      {
          if (buffer[j]==" ")
          {
              k++;
          }
          else
          {
              nums[k][i]=buffer[i];
          }
      }
      min_max.min=atoi(nums[0]);
      min_max.max=atoi(nums[1]);
      
    } else {
      // read from pipes
      char nums[2][8];
      read(min_pipe[i][0],nums[0],strlen(nums[0]));
      min_max.min=atoi(nums[0]);
      read(max_pipe[i][0],nums[1],strlen(nums[1]));
      min_max.max=atoi(nums[1]);
      
    }

    if (min < min_max.min) min_max.min = min;
    if (max > min_max.max) min_max.max = max;
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);

  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
  return 0;
}
