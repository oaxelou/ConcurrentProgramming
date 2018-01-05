/* Axelou Olympia, 2161
 * Tsitsopoulou Eirini, 2203
 *
 * The functions of mtx_sema library.
 */
#include "mtx_sema.h"

/*Semaphore functions*/

// Return value: 1 for failure (sem already up -> value remains 1)
//               0 for success
int my_bsem_up(my_bsem *sem, int line){
  int mtx_res;

  // entering CS
  mtx_res = pthread_mutex_lock((pthread_mutex_t*)&(sem->waitingCS));
  if(mtx_res){
    printf("Line %d:(waitingCS) pthread_mutex_lock(%d): %s\n", line, mtx_res, strerror(mtx_res));
    exit(1);
  }


  if(sem->value == 1){  // Sem is already up.
    mtx_res = pthread_mutex_unlock((pthread_mutex_t*)&(sem->waitingCS));
    if(mtx_res){
      printf("Line %d:(waitingCS) pthread_mutex_unlock(%d): %s\n", line, mtx_res, strerror(mtx_res));
      exit(1);
    }
    return 1;
  }

  sem->value++;

  if(sem->value <= 0){  //someone is blocked
    mtx_res = pthread_mutex_unlock((pthread_mutex_t*)&(sem->mtx));
    if(mtx_res){
      printf("Line %d: pthread_mutex_unlock(%d): %s\n", line, mtx_res, strerror(mtx_res));
      exit(1);
    }
  }

  // exiting CS
  mtx_res = pthread_mutex_unlock((pthread_mutex_t*)&(sem->waitingCS));
  if(mtx_res){
    printf("Line %d:(waitingCS) pthread_mutex_unlock(%d): %s\n", line, mtx_res, strerror(mtx_res));
    exit(1);
  }

  return 0;
}

void my_bsem_down(my_bsem *sem, int line){
  int mtx_res;

  // entering CS
  mtx_res = pthread_mutex_lock((pthread_mutex_t*)&(sem->waitingCS));
  if(mtx_res){
    printf("Line %d:(waitingCS) pthread_mutex_lock(%d): %s\n", line, mtx_res, strerror(mtx_res));
    exit(1);
  }

  sem->value--;

  if(sem->value < 0){
    // unlocking waitingCS in case of blocking with mtx
    mtx_res = pthread_mutex_unlock((pthread_mutex_t*)&(sem->waitingCS));
    if(mtx_res){
      printf("Line %d:(waitingCS) pthread_mutex_unlock(%d): %s\n", line, mtx_res, strerror(mtx_res));
      exit(1);
    }

    mtx_res = pthread_mutex_lock((pthread_mutex_t*)&(sem->mtx));
    if(mtx_res){
      printf("Line %d: pthread_mutex_lock(%d): %s\n", line, mtx_res, strerror(mtx_res));
      exit(1);
    }
  }
  else if(sem->value == 0){
    // entering CS
    mtx_res = pthread_mutex_unlock((pthread_mutex_t*)&(sem->waitingCS));
    if(mtx_res){
      printf("Line %d:(waitingCS) pthread_mutex_unlock(%d): %s\n", line, mtx_res, strerror(mtx_res));
      exit(1);
    }
  }
}

void my_bsem_init(my_bsem *sem, int init_value, int line){
  pthread_mutexattr_t attr, cs_attr;
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

  sem->value = init_value;

  // init tou semaphore mutex
  mtx_res = pthread_mutex_init((pthread_mutex_t*)&(sem->mtx), &attr);
  if(mtx_res){
    printf("Line %d: pthread_mutex_init(%d): %s\n", line, mtx_res, strerror(mtx_res));
    exit(1);
  }

  // init tou CS mutex
  cs_attr = attr; //same attributes
  mtx_res = pthread_mutex_init((pthread_mutex_t*)&(sem->waitingCS), &cs_attr);
  if(mtx_res){
    printf("Line %d:(waitingCS) pthread_mutex_init(%d): %s\n", line, mtx_res, strerror(mtx_res));
    exit(1);
  }

  if (!init_value){ // if (init_value == 0) mutex must be locked first
    mtx_res = pthread_mutex_lock((pthread_mutex_t*)&(sem->mtx));
    if(mtx_res){
      printf("Line %d: pthread_mutex_lock(%d): %s\n", line, mtx_res, strerror(mtx_res));
      exit(1);
    }
  }
}

void my_bsem_destroy(my_bsem *sem, int line){
  int mtx_res;

  // entering CS
  mtx_res = pthread_mutex_lock((pthread_mutex_t*)&(sem->waitingCS));
  if(mtx_res){
    printf("Line %d:(waitingCS) pthread_mutex_lock(%d): %s\n", line, mtx_res, strerror(mtx_res));
    exit(1);
  }

  // checking if sem->mtx is locked and if it does, it is then being unlocked.
  // (so that destroy won't fail with error 16)
  if (sem->value != 1){
    mtx_res = pthread_mutex_unlock((pthread_mutex_t*)&(sem->mtx));
    if(mtx_res){
      printf("Line %d: (mtx) pthread_mutex_unlock(%d): %s\n", line, mtx_res, strerror(mtx_res));
      exit(1);
    }
  }

  // exiting CS
  mtx_res = pthread_mutex_unlock((pthread_mutex_t*)&(sem->waitingCS));
  if(mtx_res){
    printf("Line %d:(waitingCS) pthread_mutex_unlock(%d): %s\n", line, mtx_res, strerror(mtx_res));
    exit(1);
  }

  mtx_res = pthread_mutex_destroy((pthread_mutex_t*)&(sem->mtx));
  if(mtx_res){
      printf("Line %d: pthread_mutex_destroy(%d): %s\n", line, mtx_res, strerror(mtx_res));
      exit(1);
  }

  mtx_res = pthread_mutex_destroy((pthread_mutex_t*)&(sem->waitingCS));
  if(mtx_res){
      printf("Line %d: (waitingCS) pthread_mutex_destroy(%d): %s\n", line, mtx_res, strerror(mtx_res));
      exit(1);
  }
}


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
