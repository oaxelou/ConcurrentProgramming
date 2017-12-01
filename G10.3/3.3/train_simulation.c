/* Axelou Olympia, 2161
 * Tsitsopoulou Eirini, 2203
 *
 * This program simulates a roller coaster.
 * Every new ride begins when train is full.
 *
 * Implemented with conditions and mutexes
 *
 * The program never terminates because the train never stops.
 */
 #define _GNU_SOURCE
#include "mtx_cond.h"
#include <errno.h>

pthread_mutex_t mtx;
pthread_cond_t wait_to_fill;
pthread_cond_t train_start;
pthread_cond_t wait_to_empty;

pthread_cond_t pas_entering;
pthread_cond_t pas_exiting;

volatile int waiting, onboard;
volatile int trainCapacity;
volatile int train_w_fill, train_w_start, train_w_empty;

#define RIDE_DURATION 2

/* Controls when a passenger enters.
 *
 * If there are enough pasengers for the train to start, they notify the train
 * and then the train allows them to enter.
 *
 * The first passenger that unblocks, unblocks the others behind him.
 *
 * The last passenger that enters notifies the train to begin the ride.
 */
void train_enter(){

  mtx_lock(&mtx, __LINE__);
  waiting++;

  // enough passengers to notify the train to start taking passengers
  // and train is actually waiting
  if(waiting == trainCapacity && train_w_fill == 1){
    train_w_fill = 0;
    cond_signal(&wait_to_fill, __LINE__);
  }

  cond_wait(&pas_entering, &mtx, __LINE__);  //waiting for train to allow them to get in
  // printf("woke up and onboard = %d\n", onboard);
  waiting--;
  onboard++;

  if(onboard < trainCapacity){    //unblock one passenger behind him
    cond_signal(&pas_entering, __LINE__);
  }
  else if(onboard == trainCapacity){
    printf("\t"ANSI_COLOR_CYAN"Train full. Going to Start."ANSI_COLOR_RESET"\n");

    if(train_w_start){
      train_w_start = 0;
      cond_signal(&train_start, __LINE__); //notify the train to start
    }
  }

  mtx_unlock(&mtx, __LINE__);
}

/* This function is called when a passenger exits the train.
 * Every passenger notifies the next one to exit.
 * The last one notifies the train that it has emptied.
 */
void train_exit(){

  mtx_lock(&mtx, __LINE__);
  cond_wait(&pas_exiting, &mtx, __LINE__);

  // printf("going to exit with onboard = %d\n", onboard);
  if(onboard > 0){  //unblock one behind them
    onboard--;
    cond_signal(&pas_exiting, __LINE__);
  }
  else if(onboard == 0 && train_w_empty == 1){  //last passenger notifies train it has emptied
    train_w_empty = 0;
    cond_signal(&wait_to_empty, __LINE__);
  }

  mtx_unlock(&mtx, __LINE__);
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

	  mtx_lock(&mtx, __LINE__);

    if(waiting < trainCapacity){
      train_w_fill = 1;
      cond_wait(&wait_to_fill, &mtx, __LINE__); // waits until enough passengers have arrived
    }

    // onboard++; waiting--;
    // printf("train. going to wake up first passanger with onboard = %d\n", onboard);
    cond_signal(&pas_entering, __LINE__); // notify passengers to enter
    // printf("train after signa: onboard = %d\n", onboard);
    if (onboard < trainCapacity){
      train_w_start = 1;
      cond_wait(&train_start, &mtx, __LINE__); // waits until last passenger has got in
    }

    sleep(RIDE_DURATION);

    onboard--;
    cond_signal(&pas_exiting, __LINE__); // notifies one passenger to exit

    if (onboard > 0){
      train_w_empty = 1;
      cond_wait(&wait_to_empty, &mtx, __LINE__); // waits for the last passenger to exit
      // printf("just woke up train. everybode gone\n");
    }
    mtx_unlock(&mtx, __LINE__);
  }

}

/* DEBUGGING FUNCTION
 * This function prints every second the values of waiting passengers and passengers onboard
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

void init_mtx_cond(){
  mtx_init(&mtx, __LINE__);
  cond_init(&wait_to_fill, __LINE__);
  cond_init(&wait_to_empty, __LINE__);
  cond_init(&pas_entering, __LINE__);
  cond_init(&pas_exiting, __LINE__);
  cond_init(&train_start, __LINE__);

  waiting = onboard = train_w_start = 0;
  train_w_fill = train_w_empty = 0;
}

void destroy_mtx_cond(){
  mtx_destroy(&mtx, __LINE__);
  cond_destroy(&wait_to_fill, __LINE__);
  cond_destroy(&wait_to_empty, __LINE__);
  cond_destroy(&pas_entering, __LINE__);
  cond_destroy(&pas_exiting, __LINE__);
  cond_destroy(&train_start, __LINE__);
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

  init_mtx_cond();

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

  destroy_mtx_cond();

  printf("If you are watching this, something is wrong\n");
  return 0;
}
