/* Axelou Olympia, 2161
 * Tsitsopoulou Eirini, 2203
 *
 * This program simulates a one way bridge where cars from the two different
 * directions try to pass and we have to control the flow.
 *
 * The synchronization betweeen the cars is achieved directly by the car threads
 * without any auxiliary thread to control the flow.
 *
 * The cars from one direction are considered as blue cars and the others as red.
 *
 * Since all of the cars have to act the same when they try to enter and exit bridge
 * the function for every car is the same.
 */
#include "../2.1/my_sema.h"
#include <errno.h>

volatile my_bsem mtx;
volatile my_bsem waitq[2];
volatile int waiting[2];
volatile int onbridge[2];
volatile int carsPassing[2];
volatile int bridgeCapacity;

#define MAX_PASSING 2   // the maximum number of cars that the waiting cars allow to pass before them

#define RED 0
#define BLUE 1
const char *colors[] = {"red", "blue"};

/* Controls when a car can enter.
 * The car blocks when the bridge is full, when cars of the other color are on bridge
 * or when cars of the other color have been waiting for a very long time.
 *
 * Since the synchronization is made with bsem the very first car that is
 * unblocked has to unblock the others behind him.
 */
void bridge_enter(int my_color){

  my_bsem_down((my_bsem*)&mtx, __LINE__);

  if(carsPassing[my_color] >= 0){ // checks if there is a car from the other color waiting
    carsPassing[my_color]++;
  }

  if(onbridge[!my_color] || (onbridge[my_color] >= bridgeCapacity) || carsPassing[my_color] > MAX_PASSING){
    waiting[my_color]++;

    if(carsPassing[!my_color] == -1){
      carsPassing[!my_color] = 0;
    }

    my_bsem_up((my_bsem*)&mtx, __LINE__);
    my_bsem_down((my_bsem*)&(waitq[my_color]), __LINE__);

    my_bsem_down((my_bsem*)&mtx, __LINE__);

    if(carsPassing[!my_color] >= 0){
      carsPassing[!my_color] = -1;
    }

    //awakens the one behind him if there is one and if the bridge is not full
    if((waiting[my_color] > 0) && (onbridge[my_color] < bridgeCapacity)){
      waiting[my_color]--;
      onbridge[my_color]++;
      my_bsem_up((my_bsem*)&(waitq[my_color]), __LINE__);
    }
  }
  else{
    onbridge[my_color]++;
  }
  my_bsem_up((my_bsem*)&mtx, __LINE__);
}

/* This function is called when a car wishes to exit bridge.
 * It controls if and which waiting car will be unlocked.
 */
void bridge_exit(int my_color){

  my_bsem_down((my_bsem*)&mtx, __LINE__);
  onbridge[my_color]--;

  //there is none left on bridge while there are of the other color waiting
  if(onbridge[my_color] == 0 && waiting[!my_color] > 0 ){
    waiting[!my_color]--;
    onbridge[!my_color]++;
    my_bsem_up((my_bsem*)&(waitq[!my_color]), __LINE__);
  }
  else if(onbridge[my_color] >= bridgeCapacity - 1 && waiting[my_color] > 0 ){ //the bridge is not full anymore, allows of same color to pass
    waiting[my_color]--;
    onbridge[my_color]++;
    my_bsem_up((my_bsem*)&(waitq[my_color]), __LINE__);
  }

  my_bsem_up((my_bsem*)&mtx, __LINE__);
}

/* Thread function which implements a car.
 * First, it calls bridge_enter function,
 * then stays on the bridge for double the time given by input file (time that main waits before creating new car-thread)
 * and then calls bridge_exit function
 * Doesn't return anything.
 */
void* car (void *arg){
  int my_color, my_time;
  my_color = *(int *)arg;
  my_time = *((int *)arg+1);

  bridge_enter(my_color);

  printf("\t\t\t"ANSI_COLOR_GREEN"my color is %s, my time is %d"ANSI_COLOR_RESET"\n", colors[my_color], my_time);
  sleep(my_time * 2); //time on bridge

  bridge_exit(my_color);

  return NULL;
}

/* DEBUGGING FUNCTION
 * This function prints every second the values of waiting cars and cars on bridge
 */
void *print_time_screenshot(void *arg){
  int i = 0;

  printf("time:   0  onbridge[red, blue] = [%d, %d], waiting[red, blue] = [%d, %d]\n", onbridge[RED], onbridge[BLUE], waiting[RED], waiting[BLUE]);
  usleep(500000); // head start so that it prints the values in the middle of the second

  while(1){
    i++;

    printf("time: %3d  onbridge[red, blue] = [%d, %d], waiting[red, blue] = [%d, %d]\n", i, onbridge[RED], onbridge[BLUE], waiting[RED], waiting[BLUE]);
    sleep(1);
  }

  return NULL;
}

//auxiliary function
void read_car_args(int *args, int *nofred, int *nofblue){
  char car_color;
  scanf(" %c%d", &car_color, args + 1);
  if (car_color == 'b'){
    args[0] = BLUE;
    nofblue[0]++;
  }
  else if (car_color == 'r'){
    args[0] = RED;
    nofred[0]++;
  }
  else{
    printf("something went HORRIBLY wrong: input not suitable\n");
    exit(1);
  }
  //printf("car color: %c, bridge_time: %d\n", car_color, bridge_time);
}

//auxiliary function
void init_semaphores(){
  int i;

  my_bsem_init((my_bsem*)&mtx, 1, __LINE__);
  for(i = 0; i < 2; i++){
    my_bsem_init((my_bsem*)&waitq[i], 0, __LINE__);
    waiting[i] = 0;
    onbridge[i] = 0;
    carsPassing[i] = -1;
  }
}

//auxiliary function
void destroy_semaphores(){
  int i;

  my_bsem_destroy((my_bsem*)&mtx, __LINE__);
  for(i = 0; i < 2; i++){
    my_bsem_destroy((my_bsem*)&waitq[i], __LINE__);
  }
}

int main(int argc, char *argv[]) {
  int nofcars = 0, nofblue = 0, nofred = 0, i, args[2];
  pthread_t *car_threads;
  pthread_t print_time_thread;

  if(argc != 2){
    printf("Wrong number of arguments. Terminating.\n");
    exit(1);
  }

  bridgeCapacity = atoi(argv[1]);
  if(bridgeCapacity == 0){
    printf("Not a valid argument for train capacity. Terminating.\n");
    exit(1);
  }

  init_semaphores();

  scanf("%d", &nofcars);
  car_threads = (pthread_t *)malloc(sizeof(pthread_t)*nofcars);

  if (pthread_create(&print_time_thread, NULL, print_time_screenshot, NULL)){
    printf("problem in creating print_time_thread\n");
    return 1;
  }

  for (i = 0; i < nofcars; i++){
    read_car_args(args, &nofred, &nofblue);

    if (pthread_create(&car_threads[i], NULL, car, args)){
      printf("problem in creating thread %d\n", i);
      return 1;
    }

    sleep(args[1]);
  }

  for (i = 0; i < nofcars; i++){
    pthread_join(car_threads[i], NULL);
  }

  sleep(1); //so that print_time_thread function prints one final time to confirm that last car(s) have exited the bridge
            // which means -> all zeros.

  destroy_semaphores();

  printf("\nAll threads have terminated. The original input, counted by main, has:\n");
  printf("# of blue: %d\n", nofblue);
  printf("# of red : %d\n", nofred);

  return 0;
}
