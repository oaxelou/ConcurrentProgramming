#include "my_sema.h"

volatile my_bsem *sem;

void *core (void *arg){
  int my_no = *(int *)arg;
  // int mtx_res;

  //be sure to take the right args
  my_bsem_up((my_bsem*)&(sem[my_no]), __LINE__);
  printf("my no: %d\n", my_no);
  // while(1){
    // // wait for main to assign job
    // my_bsem_down((my_bsem*)&(sem_assign[my_no]), __LINE__);
    //
    // // perform the Mandelbrot computation
    // mandel_Calc((mandel_Pars*)slices+my_no,maxIterations,(int*)res + my_no*slices[my_no].imSteps*slices[my_no].reSteps);
    //
    // //notify main
    // my_bsem_down((my_bsem*)&(sem_draw[my_no]), __LINE__);

  // }
  return NULL;
}

int main(int argc, char const *argv[]) {
  int i, nofcores;
  pthread_t *coreT;

  printf("Number of cores: ");
  scanf("%d", &nofcores);

  //allocating volatile sems
  sem = (volatile my_bsem *)malloc(sizeof(my_bsem)*nofcores);
  if(sem == NULL){fprintf(stderr, "error malloc sem\n");exit(1);}
  coreT = (pthread_t *)malloc(sizeof(pthread_t)*nofcores);
  if(coreT == NULL){fprintf(stderr, "error malloc pthread_t\n");exit(1);}

  // initialising semaphores
  for (i=0; i<nofcores; i++){
      my_bsem_init((my_bsem*)&(sem[i]), 0, __LINE__);
  }

  // creating threads
  for (i=0; i<nofcores; i++){
      if (pthread_create(&coreT[i], NULL, core, (void*)&i)){
        printf("error at pthread_create no: %d\n", i);
        exit(1);
      }

      // waits for the thread to take its argument
      // before continuing with the next one
      my_bsem_down((my_bsem*)&(sem[i]), __LINE__);
  }


  //join
  for(i = 0; i < nofcores; i++){
    pthread_join(coreT[i], NULL);
    my_bsem_destroy((my_bsem*)sem + i, __LINE__);
  }
  free(coreT);
  free((my_bsem*)sem);
  return 0;
}
