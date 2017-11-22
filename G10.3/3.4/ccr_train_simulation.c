/* Axelou Olympia, 2161
 * Tsitsopoulou Eirini, 2203
 *
 * This program simulates a roller coaster.
 * Every new ride begins when train is full.
 *
 * The program never terminates because the train never stops.
 */
//#include "my_sema.h"
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
                                      no_q##label--; \
                                      cond_signal(&queue##label, __LINE__); \
                                    } \
                                    else{ \
                                      count_loop##label = -1; \
                                      mtx_unlock(&mtx##label, __LINE__); \
                                    } \
                                    mtx_unlock(&mtx_q##label, __LINE__);
// **************************************************

CCR_DECLARE(X);

volatile int wait_to_fill, wait_to_empty, train_start;
volatile int pas_entering, pas_exiting;

volatile int waiting, onboard;
volatile int trainCapacity;
volatile int train_w_fill, train_w_empty;

#define RIDE_DURATION 2

/* Controls when a passenger enters.
 *
 * If there are enough passangers for the train to start, they notify the train
 * and then the train allows them to enter.
 *
 * Since the synchronization is made with bsem the very first passenger that enters
 * the train has to unblock the others behind him.
 *
 * The last passenger that enters notifies the train to begin the ride.
 */
void train_enter(){

  CCR_EXEC(X, \
    /* When */  1, \
      waiting++; \
      if(waiting == trainCapacity){ \
        wait_to_fill = 1; \
      } \
  );


  CCR_EXEC(X, \
    /* When */  pas_entering, \
      onboard++; \
      waiting--; \
      if(onboard == trainCapacity){ \
        pas_entering = 0; \
        train_start = 1; \
      } \
  );
}

/* This function is called when a passenger exits the train.
 * Every passenger notifies the next one to exit.
 * The last one notifies the train that it has emptied.
 */
void train_exit(){

  CCR_EXEC(X, \
    /* When */  pas_exiting, \
      onboard--; \
      if(!onboard){ \
        pas_exiting = 0; \
        wait_to_empty = 1; \
      } \
  );
}

/* Thread function which implements a passenger.
 * First, it calls train_enter function,
 * then stays on the train for the duration of the ride (controlled by the train)
 * and then calls train_exit function
 *
 * Doesn't return anything.
 */
void *passenger(void *args){

  train_enter();

  // on ride

  train_exit();

  return NULL;
}

/* Thread function which implements the train.
 *
 * Blocks until enough passengers have arrived. Then notifies passengers to start entering
 * Waits until last passenger has entered.
 * When ride is over, notifies passangers to exit. Waits for the last passanger to exit before next ride
 *
 * Doesn't terminate.
 */
void *train(void *args){

  while(1){
    printf("\t"ANSI_COLOR_GREEN"* * * * * * * * Ready for a new ride * * * * * * * *"ANSI_COLOR_RESET"\n");

    CCR_EXEC(X, \
      /* When */  wait_to_fill, \
        wait_to_fill = 0; \
        pas_entering = 1; \
    );


    CCR_EXEC(X, \
      /* When */  train_start, \
      train_start = 0; \
    );

    printf("after wait train start\n");

    sleep(RIDE_DURATION);

    CCR_EXEC(X, \
      /* When */  1, \
        pas_exiting = 1; \
    );

    CCR_EXEC(X, \
      /* When */  wait_to_empty, \
        wait_to_empty = 0; \
    );
  }
}

/* DEBUGGING FUNCTION
 * This function prints every second the values of waiting passangers and passangers onboard
 */
void *print_time_screenshot(void *arg){
  int i = 0;

  printf("time:   0  onboard = %d, waiting = %d\n", onboard, waiting);
  usleep(800000); // head start so that it prints the values in the middle of the second

  while(1){
    i++;

	  printf("time: %3d  onboard = %d, waiting = %d\n", i, onboard, waiting);

    sleep(1);
  }
  return NULL;
}

void init_function(){
  wait_to_fill = wait_to_empty = train_start = 0;
  pas_entering = pas_exiting = 0;

  waiting = onboard = 0;
}


int main(int argc, char *argv[]) {
  int nofpassegers = 0, i, waiting_time;
  pthread_t *passenger_threads;
  pthread_t roller_coaster_thread, print_time_thread;

  // retrieving arguments: train capacity
  if(argc != 2){
    printf("Wrong number of arguments. Terminating.\n");
    exit(1);
  }

  trainCapacity = atoi(argv[1]);
  if(trainCapacity == 0){
    printf("Not a valid argument for train capacity. Terminating.\n");
    exit(1);
  }
  printf("trainCapacity = %d\n", trainCapacity);sleep(1);

  init_function();
  CCR_INIT(X);

  if (pthread_create(&roller_coaster_thread, NULL, train, NULL)){
    printf("problem in creating roller_coaster_thread\n");
    return 1;
  }

  if (pthread_create(&print_time_thread, NULL, print_time_screenshot, NULL)){
    printf("problem in creating print_time_thread\n");
    return 1;
  }

  scanf("%d", &nofpassegers);
  passenger_threads = (pthread_t *)malloc(sizeof(pthread_t)*nofpassegers);

  for (i = 0; i < nofpassegers; i++){
    scanf("%d", &waiting_time);
    if (pthread_create(&passenger_threads[i], NULL, passenger, NULL)){
      printf("problem in creating thread %d\n", i);
      return 1;
    }
    sleep(waiting_time);
  }

  if(pthread_join(roller_coaster_thread, NULL)){
    printf("problem with join\n");
    return 1;
  }

  /* never reach this point; for cosmetic reasons */

  printf("If you are watching this, something is wrong\n");
  return 0;
}
