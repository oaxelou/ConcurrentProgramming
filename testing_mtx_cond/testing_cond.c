/*
 * Testing condition. (and mutexes obviously)
 * The behavior is sometimes unexpected as far as it concerns
 * SIG_BLOCK & SIG_CONT
 *
 * The signal can literally be "lost".
 * If signal is called before wait, it does nothing.
 * So if then sb calls wait, he blocks permanently.
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

pthread_cond_t cond;
pthread_mutex_t mtx;

void *foo1(void *args){
  int pthread_res;
  // struct timespec timeVal;
  // timeVal.tv_sec = 0; timeVal.tv_nsec = 1000000;

  printf("hello from t1. Going to wait\n");
  pthread_res = pthread_cond_wait(&cond, &mtx);

  /*pthread_res = pthread_cond_timedwait(&cond, &mtx, &timeVal);
  if(pthread_res == ETIMEDOUT){
    printf("waited for too long: %s. Going to continue my life now\n", strerror(pthread_res));
  }
  else*/ if(pthread_res){
    printf("Error with pthread_cond_wait: %s\n", strerror(pthread_res));
    exit(1);
  }
  printf("t1: just woke up. Bye\n");
  return NULL;
}

void *foo2(void *args){
  int pthread_res;

  printf("hello from t2\n");
  pthread_res = pthread_cond_signal(&cond);
  if(pthread_res){
    printf("Error with pthread_cond_signal: %s\n", strerror(pthread_res));
    exit(1);
  }
  printf("t2: signaled. Bye\n");
  return NULL;
}

int main(int argc, char *argv[]){

  pthread_t t1, t2;
  int pthread_res;

  pthread_res = pthread_cond_init(&cond, NULL);
  if(pthread_res){
    printf("Error with pthread_cond_init: %s\n", strerror(pthread_res));
    exit(1);
  }

  pthread_res = pthread_mutex_init(&mtx, NULL);
  if(pthread_res){
    printf("Error with pthread_mutex_init: %s\n", strerror(pthread_res));
    exit(1);
  }

  printf("main: creating t1\n");

  pthread_res = pthread_create(&t1, NULL, foo1, NULL);
  if(pthread_res){
    printf("Error with 1st pthread_create: %s\n", strerror(pthread_res));
    exit(1);
  }

  // maybe wait for a little while.
  printf("main: going to sleep\n");
  sleep(2);

  printf("main: creating t2\n");

  pthread_res = pthread_create(&t2, NULL, foo2, NULL);
  if(pthread_res){
    printf("Error with 2nd pthread_create: %s\n", strerror(pthread_res));
    exit(1);
  }

  pthread_join(t1, NULL);
  pthread_join(t2, NULL);

  pthread_res = pthread_cond_destroy(&cond);
  if(pthread_res){
    printf("Error with pthread_cond_destroy: %s\n", strerror(pthread_res));
    exit(1);
  }

  pthread_res = pthread_mutex_destroy(&mtx);
  if(pthread_res){
    printf("Error with pthread_mutex_destroy: %s\n", strerror(pthread_res));
    exit(1);
  }

  return 0;
}
