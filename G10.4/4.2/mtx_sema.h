/* Axelou Olympia, 2161
* Tsitsopoulou Eirini, 2203
*
* Library: Binary semaphores using pthread_mutexes.
*/

#ifndef __MTX_SEMA__
#define __MTX_SEMA__

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define MUTEX_TYPE PTHREAD_MUTEX_NORMAL

typedef struct{
  pthread_mutex_t mtx;        // mutex of semaphore
  pthread_mutex_t waitingCS;  // mutex for the CS where we the value field
  int value;
}my_bsem;

// the semaphore functions.
// the argument "int line" is for error printing
int my_bsem_up(my_bsem *sem, int line);
void my_bsem_down(my_bsem *sem, int line);
void my_bsem_init(my_bsem *sem, int init_value, int line);
void my_bsem_destroy(my_bsem *sem, int line);

/* PTHREAD_MUTEXES */
void mtx_init(pthread_mutex_t *mtx, int line);
void mtx_lock(pthread_mutex_t *mtx, int line);
void mtx_unlock(pthread_mutex_t *mtx, int line);
void mtx_destroy(pthread_mutex_t *mtx, int line);

#endif
