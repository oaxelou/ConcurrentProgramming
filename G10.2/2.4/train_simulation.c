/* Axelou Olympia, 2161
 * Tsitsopoulou Eirini, 2203
 *
 * This program simulates a roller coaster.
 * Every new ride begins when train is full.
 *
 * The program never terminates because the train never stops.
 */
#include "../2.1/my_sema.h"
#include <errno.h>

volatile my_bsem mtx;
volatile my_bsem wait_to_fill;
volatile my_bsem train_start;
volatile my_bsem wait_to_empty;

volatile my_bsem pas_entering;
volatile my_bsem pas_exiting;

volatile int waiting, onboard;
volatile int trainCapacity;

#define RIDE_DURATION 4

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

  my_bsem_down((my_bsem*)&mtx, __LINE__);
  waiting++;

  // enough passengers to notify the train to start taking passengers
  if(waiting == trainCapacity){
    my_bsem_up((my_bsem*)&wait_to_fill, __LINE__);
  }

  my_bsem_up((my_bsem*)&mtx, __LINE__);

  my_bsem_down((my_bsem*)&pas_entering, __LINE__);  //waiting for train to allow them to get in

  my_bsem_down((my_bsem*)&mtx, __LINE__);

  if(onboard < trainCapacity){    //unblock one passenger behind him
    waiting--;
    onboard++;
    my_bsem_up((my_bsem*)&pas_entering, __LINE__);
  }
  else if(onboard == trainCapacity){
    printf("\t"ANSI_COLOR_CYAN"Train full. Going to Start."ANSI_COLOR_RESET"\n");
    my_bsem_up((my_bsem*)&train_start, __LINE__); //notify the train to start
  }
  my_bsem_up((my_bsem*)&mtx, __LINE__);
}

/* This function is called when a passenger exits the train.
 * Every passenger notifies the next one to exit.
 * The last one notifies the train that it has emptied.
 */
void train_exit(){

  my_bsem_down((my_bsem*)&pas_exiting, __LINE__);

  my_bsem_down((my_bsem*)&mtx, __LINE__);
  if(onboard > 0){
    onboard--;
    my_bsem_up((my_bsem*)&pas_exiting, __LINE__);
  }
  else if(onboard == 0){
    my_bsem_up((my_bsem*)&wait_to_empty, __LINE__);
  }

  my_bsem_up((my_bsem*)&mtx, __LINE__);
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

	  my_bsem_down((my_bsem*)&mtx, __LINE__);

    if(waiting < 2*trainCapacity){
      my_bsem_up((my_bsem*)&mtx, __LINE__);
      my_bsem_down((my_bsem*)&wait_to_fill, __LINE__); //waits until enough passangers have arrived

      my_bsem_down((my_bsem*)&mtx, __LINE__);
    }
    onboard++; waiting--;
    my_bsem_up((my_bsem*)&pas_entering, __LINE__); // notify passangers to enter
    my_bsem_up((my_bsem*)&mtx, __LINE__);

    my_bsem_down((my_bsem*)&train_start, __LINE__); // waits until last passanger has got in

    sleep(RIDE_DURATION);

    my_bsem_down((my_bsem*)&mtx, __LINE__);
    onboard--;
    my_bsem_up((my_bsem*)&pas_exiting, __LINE__); // notifies one passanger to exit
    my_bsem_up((my_bsem*)&mtx, __LINE__);

    my_bsem_down((my_bsem*)&wait_to_empty, __LINE__); //Waits for the last passanger to exit
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

void init_semaphores(){
  my_bsem_init((my_bsem*)&mtx, 1, __LINE__);
  my_bsem_init((my_bsem*)&wait_to_fill, 0, __LINE__);
  my_bsem_init((my_bsem*)&wait_to_empty, 0, __LINE__);
  my_bsem_init((my_bsem*)&pas_entering, 0, __LINE__);
  my_bsem_init((my_bsem*)&pas_exiting, 0, __LINE__);
  my_bsem_init((my_bsem*)&train_start, 0, __LINE__);

  waiting = onboard = 0;
}

void destroy_semaphores(){
  my_bsem_destroy((my_bsem*)&mtx, __LINE__);
  my_bsem_destroy((my_bsem*)&wait_to_fill, __LINE__);
  my_bsem_destroy((my_bsem*)&wait_to_empty, __LINE__);
  my_bsem_destroy((my_bsem*)&pas_entering, __LINE__);
  my_bsem_destroy((my_bsem*)&pas_exiting, __LINE__);
  my_bsem_destroy((my_bsem*)&train_start, __LINE__);
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

  init_semaphores();

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

  destroy_semaphores();

  printf("If you are watching this, something is wrong\n");
  return 0;
}
