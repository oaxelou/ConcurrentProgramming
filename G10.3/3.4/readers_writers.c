#include "mtx_cond.h"

#define CCR_DECLARE(label) pthread_mutex_t mtx##label, mtx_q##label; \
                           pthread_cond_t queue##label;\
                           volatile int no_q##label, count_loop##label;

#define CCR_INIT(label) mtx_init(&mtx##label, __LINE__); \
                        mtx_init(&mtx_q##label, __LINE__); \
                        cond_init(&queue##label, __LINE__); \
                        no_q##label = 0; count_loop##label = -1;

#define CCR_EXEC(label, cond, body) printf("%lu locking mtxs\n", (long unsigned)pthread_self());mtx_lock(&mtx##label, __LINE__); \
                                    printf("%lu locking mtxq\n", (long unsigned)pthread_self());mtx_lock(&mtx_q##label, __LINE__); \
                                    printf("%lu going to check with:%d\n", (long unsigned)pthread_self(), cond);\
                                    while (!cond){ \
                                      printf("%lu going to check AGAIN with:%d\n", (long unsigned)pthread_self(), cond);\
                                      no_q##label++; \
                                      if(count_loop##label >= 0){ \
                                        count_loop##label++; \
                                    \
                                        if(count_loop##label == no_q##label){ \
                                          count_loop##label = -1; \
                                          mtx_unlock(&mtx##label, __LINE__); \
                                        } \
                                        else{ \
                                          no_q##label--; cond_signal(&queue##label, __LINE__); \
                                        } \
                                      } \
                                      else{ \
                                        mtx_unlock(&mtx##label, __LINE__); \
                                      }\
                                      cond_wait(&queue##label, &mtx_q##label, __LINE__); \
                                    } \
                                    printf("%lu got in body with no_q = %d!\n", (long unsigned)pthread_self(), no_q##label);\
                                    body \
                                    \
                                    if(no_q##label > 0){ \
                                      count_loop##label = 0; \
                                      cond_signal(&queue##label, __LINE__); \
                                    } \
                                    else{ \
                                      count_loop##label = -1; \
                                      mtx_unlock(&mtx##label, __LINE__); \
                                    } \
                                    mtx_unlock(&mtx_q##label, __LINE__);
// **************************************************

volatile int wrs, rds;
CCR_DECLARE(X);

void *reader(void *argv){
  printf("reader going to get in\n");
  CCR_EXEC(X, (wrs == 0), rds++;);

  printf("reader: reading data\n");
  sleep(2);
  printf("reader: data ok\n");

  CCR_EXEC(X, 1, rds--;);
  printf("reader got out, rds = %d, wrs = %d\n", wrs, rds);

  return(NULL);
}

void *writer(void *argv){
  int i;
  for(i = 0; i < 5; i ++){
    printf("writer %lu going to get in\n", (long unsigned)pthread_self());
    // printf("Hello from %lu\n", (long unsigned)pthread_self());

    CCR_EXEC(X, (/*rds + */wrs == 0), wrs = 1;);

    printf("writer %lu: writing data\n", (long unsigned)pthread_self());
    sleep(2);

    CCR_EXEC(X, (1 > 0), wrs = 0;);
    printf("writer %lu got out, rds = %d, wrs = %d\n\n\n", (long unsigned)pthread_self(), rds, wrs);

  }

  return(NULL);
}

int main(int argc, char *argv[]) {
  pthread_t t1, t2;
  int checkValue;

  CCR_INIT(X);

  wrs = rds = 0;
  checkValue = pthread_create(&t1, NULL, writer, NULL);
  if (checkValue) {
    printf("Error with reader thread create. Terminating...\n");
    return 1;
  }

  // checkValue = pthread_create(&t2, NULL, writer, NULL);
  // if (checkValue) {
  //   printf("Error with writer thread create. Terminating...\n");
  //   return 1;
  // }

  printf("main woke up\n");

  checkValue = pthread_join(t1, NULL);
  if(checkValue){
    printf("error with reader pthread_join\n");
  }

  // checkValue = pthread_join(t2, NULL);
  // if(checkValue){
  //   printf("error with writer pthread_join\n");
  // }
  printf("main bye\n");

  return 0;
}
