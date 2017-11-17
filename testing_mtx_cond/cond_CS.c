/*
 * Checking it with both cases: signal in CS, signal after CS
 *
 * In the first case, no -1. Is it not open? Is it eggshell?
 * In the second case, a -1 was found. Seems logical.
 * It has already turned up the value and when signaled, it was not in CS.
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define MAX_ITER 5
#define signalMode 1

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

pthread_cond_t cond;
pthread_mutex_t mtx;
volatile int val;

void *foo1(void *args){
  int i, pthread_res;

// *****************************************************************
  for(i = 0; i < MAX_ITER; i++){
    printf(ANSI_COLOR_GREEN"t1: %d time trying to get in CS"ANSI_COLOR_RESET"\n", i);
    pthread_res = pthread_mutex_lock(&mtx);                       if(pthread_res){printf("error with %d lock: %s\n", i, strerror(pthread_res));exit(1);}
    printf(ANSI_COLOR_GREEN"t1: %d time in CS. pre_val = %d"ANSI_COLOR_RESET"\n", i,  val);

    if(val == 0){
      pthread_res = pthread_cond_wait(&cond, &mtx);               if(pthread_res){printf("error with %d wait: %s\n", i, strerror(pthread_res));exit(1);}
    }

    val--;
    if(val == -1){
      printf(ANSI_COLOR_RED"t1: new_val = %d"ANSI_COLOR_RESET"\n", val);
    }
    else{
      printf(ANSI_COLOR_GREEN"t1: new_val = %d"ANSI_COLOR_RESET"\n", val);
    }

    pthread_res = pthread_mutex_unlock(&mtx);                     if(pthread_res){printf("error with %d unlock: %s\n", i, strerror(pthread_res));exit(1);}
  }
  // *****************************************************************

  printf("t1: bye\n");
  return NULL;
}

void *foo2(void *args){
  int i, pthread_res;

  // *****************************************************************
  for(i = 0; i < MAX_ITER; i++){

    printf(ANSI_COLOR_CYAN"t2: %d time trying to get in CS"ANSI_COLOR_RESET"\n", i);
    pthread_res = pthread_mutex_lock(&mtx);                        if(pthread_res){printf("error with %d lock: %s\n", i, strerror(pthread_res));exit(1);}

    printf(ANSI_COLOR_CYAN"t2: in CS. pre_val = %d"ANSI_COLOR_RESET"\n",  val);
    val++;

    if(signalMode){
      printf(ANSI_COLOR_CYAN"t2: %d time Executing signal."ANSI_COLOR_RESET"\n", i);
      pthread_res = pthread_cond_signal(&cond);                     if(pthread_res){printf("Error with %d signal: %s\n", i, strerror(pthread_res));exit(1);}
    }

    pthread_res = pthread_mutex_unlock(&mtx);                      if(pthread_res){printf("error with %d unlock: %s\n", i, strerror(pthread_res));exit(1);}
    printf(ANSI_COLOR_CYAN"t2: %d time out of CS."ANSI_COLOR_RESET"\n", i);

    if(!signalMode){
      printf(ANSI_COLOR_CYAN"t2: %d time executing signal."ANSI_COLOR_RESET"\n", i);
      pthread_res = pthread_cond_signal(&cond);                     if(pthread_res){printf("Error with %d signal: %s\n", i, strerror(pthread_res));}
    }
  }
  // *****************************************************************

  printf("t2: bye\n");
  return NULL;
}

int main(int argc,char *argv[]){
  pthread_t t1,t2;
  int pthread_res;

  pthread_mutex_init(&mtx, NULL);
  pthread_cond_init(&cond, NULL);
  val = 0;

  printf("main: creating t1\n");
  pthread_res = pthread_create(&t1, NULL, foo1, NULL);
  if(pthread_res){
    printf("error with 1st pthread_create: %s\n", strerror(pthread_res));
    exit(1);
  }

  printf("main: creating t2\n");
  pthread_res = pthread_create(&t2, NULL, foo2, NULL);
  if(pthread_res){
    printf("error with 2nd pthread_create: %s\n", strerror(pthread_res));
    exit(1);
  }


  pthread_join(t1, NULL);
  pthread_join(t2, NULL);

  pthread_res = pthread_mutex_destroy(&mtx);
  if(pthread_res){
    printf("error with pthread_mutex_destroy: %s\n", strerror(pthread_res));
    exit(1);
  }

  pthread_res = pthread_cond_destroy(&cond);
  if(pthread_res){
    printf("error with pthread_cond_destroy: %s\n", strerror(pthread_res));
    exit(1);
  }

  printf("main: bye\n");
  return 0;
}
