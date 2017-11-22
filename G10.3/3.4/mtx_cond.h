#ifndef __MTX_COND_H__
#define __MTX_COND_H__

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define MUTEX_TYPE PTHREAD_MUTEX_NORMAL

/* PTHREAD_MUTEXES */
void mtx_init(pthread_mutex_t *mtx, int line);
void mtx_lock(pthread_mutex_t *mtx, int line);
void mtx_unlock(pthread_mutex_t *mtx, int line);
void mtx_destroy(pthread_mutex_t *mtx, int line);

/* PTHREAD_CONDITIONS */
void cond_init(pthread_cond_t *cond, int line);
void cond_wait(pthread_cond_t *cond, pthread_mutex_t *mtx, int line);
void cond_signal(pthread_cond_t *cond, int line);
void cond_destroy(pthread_cond_t *cond, int line);

#endif
