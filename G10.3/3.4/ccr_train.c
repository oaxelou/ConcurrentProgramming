/* Axelou Olympia, 2161
 * Tsitsopoulou Eirini, 2203
 *
 * This program simulates a roller coaster.
 * Every new ride begins when train is full.
 *
 * The program never terminates because the train never stops.
 */
//#include "my_sema.h"
#include "ccr.h"

CCR_DECLARE(X);

volatile int wait_to_fill, wait_to_empty, train_start;
volatile int pas_entering, pas_exiting;

volatile int waiting, onboard;
volatile int trainCapacity;

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
        printf("\t"ANSI_COLOR_CYAN"Train full. Going to Start."ANSI_COLOR_RESET"\n");
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
