#include "mtx_cond.h"

#define CCR_DECLARE(label) pthread_mutex_t mtx##label, mtx_q##label; \
                           pthread_cond_t queue##label;\
                           volatile int no_q##label, count_loop##label;

#define CCR_INIT(label) mtx_init(&mtx##label, __LINE__); \
                        mtx_init(&mtx_q##label, __LINE__); \
                        cond_init(&queue##label, __LINE__); \
                        no_q##label = 0; count_loop##label = -1;

#define CCR_EXEC(label, cond, body) mtx_lock(&mtx##label, __LINE__); \
                                    mtx_lock(&mtx_q##label, __LINE__); \
                                    while (!cond){ \
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
                                    \
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

volatile int var;
CCR_DECLARE(region);

void *foo(void *argv){
  // printf("Hello from %lu\n", (long unsigned)pthread_self());

  CCR_EXEC(region, (var > 0), printf("got value! var = %d\n", var););

  return(NULL);
}

int main(int argc, char *argv[]) {
  pthread_t t;
  int checkValue;

  CCR_INIT(region);

  var = 0;
  checkValue = pthread_create(&t, NULL, foo, NULL);
  if (checkValue) {
    printf("Error with thread. Terminating...\n");
    return 1;
  }

  // sleep(5);
  printf("main woke up\n");
  CCR_EXEC(region, 1, var = 1;printf("in ccr. var = 1\n"););


  checkValue = pthread_join(t, NULL);
  if(checkValue){
    printf("error with pthread_join\n");
  }
  printf("main bye\n");

  return 0;
}
