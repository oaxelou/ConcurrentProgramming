#include "mtx.h"

/* PTHREAD_MUTEXES */
void mtx_init(pthread_mutex_t *mtx, int line){
  pthread_mutexattr_t attr;
  int mtx_res;

  mtx_res = pthread_mutexattr_init(&attr);
  if(mtx_res){
    printf("Line %d: pthread_mutexattr_init(%d): %s\n", line, mtx_res, strerror(mtx_res));
    exit(1);
  }

  mtx_res = pthread_mutexattr_settype(&attr, MUTEX_TYPE);
  if(mtx_res){
    printf("Line %d: pthread_mutexattr_settype(%d): %s\n", line, mtx_res, strerror(mtx_res));
    exit(1);
  }

  mtx_res = pthread_mutexattr_init(&attr);
  if(mtx_res){
    printf("Line %d: pthread_mutexattr_init(%d): %s\n", line, mtx_res, strerror(mtx_res));
    exit(1);
  }

  mtx_res = pthread_mutex_init(mtx, &attr);
  if(mtx_res){
    printf("Line %d: pthread_mutex_init(%d): %s\n", line, mtx_res, strerror(mtx_res));
    exit(1);
  }
}

void mtx_lock(pthread_mutex_t *mtx, int line){
  int mtx_res;

  mtx_res = pthread_mutex_lock((pthread_mutex_t*)mtx);
  if(mtx_res){
    printf("Line %d: pthread_mutex_lock(%d): %s\n", line, mtx_res, strerror(mtx_res));
    exit(1);
  }
}

void mtx_unlock(pthread_mutex_t *mtx, int line){
  int mtx_res;

  mtx_res = pthread_mutex_unlock((pthread_mutex_t*)mtx);
  if(mtx_res){
    printf("Line %d: pthread_mutex_unlock(%d): %s\n", line, mtx_res, strerror(mtx_res));
    exit(1);
  }
}

void mtx_destroy(pthread_mutex_t *mtx, int line){
  int mtx_res;

  mtx_res = pthread_mutex_destroy(mtx);
  if(mtx_res){
      printf("Line %d: pthread_mutex_destroy(%d): %s\n", line, mtx_res, strerror(mtx_res));
      exit(1);
  }
}
