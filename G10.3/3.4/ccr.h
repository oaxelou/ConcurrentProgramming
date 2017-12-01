/* Axelou Olympia 2161, Tsitsopoulou Eirhnh 2203
 *
 * Vivliothiki gia thn ulopoihsh twn macros gia to ccr
 */
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
                                      if(count_loop##label >= 0){ \ //sb woke him up. Not the first time in while
                                        count_loop##label++; \
                                        \
                                        if(count_loop##label == no_q##label){ \
                                          count_loop##label = -1; \ // completed a full circle. Give mtx to newcomers
                                          mtx_unlock(&mtx##label, __LINE__); \
                                        } \
                                        else{ \
                                          no_q##label--; cond_signal(&queue##label, __LINE__); \  //wake up next in queue
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
                                      count_loop##label = 0; \  //initialise loop counter
                                      no_q##label--; \
                                      cond_signal(&queue##label, __LINE__); \ //wake up next in queue
                                    } \
                                    else{ \ //no one waiting in queue
                                      count_loop##label = -1; \
                                      mtx_unlock(&mtx##label, __LINE__); \
                                    } \
                                    mtx_unlock(&mtx_q##label, __LINE__);
// **************************************************
