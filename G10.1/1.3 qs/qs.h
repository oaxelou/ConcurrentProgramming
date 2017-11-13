#ifndef __QS_H_
#define __QS_H_

#include "qs.h"
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct{
  int left;
  int right;
  int thread_num; // The no. of thread. Simply for debbuging and error printing purposes
  int join_synchr_num; // 0 or 1. used for the synchronization
}fooStruct;

volatile int *volatile array;
volatile int next_join; // Simply for debbuging and error printing purposes

int find_separation_position(int left, int right);
void *worker(void *arg);
int main_thread(int size);

#endif
