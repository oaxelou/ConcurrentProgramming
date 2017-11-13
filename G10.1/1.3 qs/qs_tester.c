/* Axelou Olympia, 2161
 * Tsitsopoulou Eirini, 2203
 *
 * The tester program of the parallel form of QS.
 *
 * Takes the inputs from standard input
 * And prints the final sorted array on standard output
 */

 #include "qs.h"

int main (int argc, char * argv[]){
  int size, i;

  printf("Enter size\n");
  scanf("%d", &size);

  array = (int *)malloc(sizeof(int)*size);
  if(array == NULL){
    printf("error allocating memory for array.\n");
    return 1;
  }

  for (i=0; i<size; i++){
    scanf("%d", array+i);
  }

  if(main_thread(size))
    return 1;

  printf("Array sorted:\n");
  for (i=0; i<size; i++){
    printf("%d ", array[i]);
  }
  printf("\n");

  free((int*)array);

  return 0;
}
