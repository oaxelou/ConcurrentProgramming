/* Axelou Olympia, 2161
 * Tsitsopoulou Eirini, 2203
 *
 * This program creates a parallel form of QS algorithm.
 * It's based on the Java algorithm of the book
 * of the Data Structures course.
 *
 * Inputs: size of array.
 *         the array (volatile)
 *
 * Outputs: the array sorted (volatile)
 */
#include "qs.h"

/* The typical QS algorithm. Nothing altered.
 *
 * Inputs: start and end of the assigned array.
 *
 * Output: the final position of the pivot
 */
int find_separation_position(int left, int right){
  int pivot = array[right];
  int i = left - 1;
  int j = right;
  int temp;

  while(1){
    while(array[++i] < pivot){}

    while(pivot < array[--j])
      if(j == left)
        break;

    if(i >= j)
      break;

    temp = array[i];
    array[i] = array[j];
    array[j] = temp;
  }
  temp = array[i];
  array[i] = array[right];
  array[right] = temp;

  return i; //thesh diaxwrismou opou metaferthike o pivot
}

/* worker: recursive function. Every worker creates 2 threads.
 *         Waits for them to finish.
 *
 * Input: struct that includes the start and the end of the assigned
 *        part of the array and an int for the synchronization with
 *        the "parent" thread that created it.
 *
 * Return Value: returns through the arguments, the "synchronization
 *               field" of the struct.
 */
void *worker(void *args){
  int separate_pos, left, right, my_thread_num, check, child_num[2] = {-1, -1};
  pthread_t t1,t2;
  fooStruct *my_args, child0_args, child1_args;

  my_args = (fooStruct*)args;
  left = my_args->left;
  right = my_args->right;
  my_thread_num = my_args->thread_num;

  if(right - left + 1< 2){
    my_args->join_synchr_num = 0;
    return NULL;
  }

  separate_pos = find_separation_position(left,right);

  if(left < separate_pos - 1){ //exei nohma h klhsh
    child_num[0] = ++next_join;

    child0_args.left = left;
    child0_args.right = separate_pos - 1;
    child0_args.thread_num = child_num[0];
    child0_args.join_synchr_num = 1;

    check = pthread_create(&t1, NULL, worker, (void*)&child0_args);
    if(check){
      printf("%d: Error with 1st pthread_create\n", my_thread_num);
      my_args->join_synchr_num = 0;
      return NULL;
    }
  }
  if(separate_pos + 1 < right){ //exei nohma h klhsh
    child_num[1] = ++next_join;

    child1_args.left = separate_pos + 1;
    child1_args.right = right;
    child1_args.thread_num = child_num[1];
    child1_args.join_synchr_num = 1;

    check = pthread_create(&t2, NULL, worker, (void*)&child1_args);
    if(check){
      printf("%d: Error with 1st pthread_create\n", my_thread_num);
      my_args->join_synchr_num = 0;
      return NULL;
    }
  }

  if(child_num[0] >= 0 && child_num[1] >= 0){
    while(child0_args.join_synchr_num || child1_args.join_synchr_num){
    }
  }
  else if(child_num[0] >= 0 && child_num[1] == -1){
    while(child0_args.join_synchr_num);
  }
  else if(child_num[1] >= 0){
    while(child1_args.join_synchr_num);
  }

  my_args->join_synchr_num = 0;

  return NULL;
}

/* main_thread: creates the first two threads as it separates
 *              the array for the 1st time and calls the function
 *              worker. Then waits for the threads that it created
 *              to finish.
 * Inputs: size of array.
 *
 * Return Value: Success: 0
 *               Failure: 1
 */
int main_thread(int size){
  int my_num, check, child_num[2] = {-1,-1};
  int separate_pos, left, right;
  fooStruct child0_args, child1_args;
  pthread_t t1, t2;

  next_join = 0;
  my_num = 0;

  //checking if size is enough
  if(size < 2){
    printf("Array not long enough for sorting.\n");
    return 1;
  }

  left = 0;
  right = size-1;
  separate_pos = find_separation_position(left,right);

  if(left < separate_pos - 1){ //exei nohma h klhsh
    child_num[0] = ++next_join;

    child0_args.join_synchr_num = 1;
    child0_args.left = left;
    child0_args.right = separate_pos - 1;
    child0_args.thread_num = next_join; // == 0

    check = pthread_create(&t1, NULL, worker, (void*)&child0_args);
    if(check){
      printf("Main(%d): Error with 1st pthread_create\n", my_num);
      return 1;
    }
  }
  if(separate_pos + 1 < right){ //exei nohma h klhsh
    child_num[1] = ++next_join;

    child1_args.join_synchr_num = 1;
    child1_args.left = separate_pos + 1;
    child1_args.right = right;
    child1_args.thread_num = next_join; // == 1

    check = pthread_create(&t2, NULL, worker, (void*)&child1_args);
    if(check){
      printf("Main(%d): Error with 1st pthread_create\n", my_num);
      return 1;
    }
  }

  if(child_num[0] >= 0 && child_num[1] >= 0){
    while(child0_args.join_synchr_num || child1_args.join_synchr_num){}
  }
  else if(child_num[0] >= 0 && child_num[1] == -1){
    while(child0_args.join_synchr_num){}
  }
  else if(child_num[1] >= 0){
    while(child1_args.join_synchr_num){}
  }

  return 0;
}
