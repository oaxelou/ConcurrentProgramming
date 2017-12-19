#include "var_storage1.h"
#include <errno.h>
// #include <stdio.h>
// #include <pthread.h>
// #include <string.h>

global_varT *head;

void *foo(void *args){

  modify_global_node(head, "$array[4]", 2, PRINT_REPORT);

  return NULL;
}

int main(int argc,char *argv[]){
  pthread_t t1;

  head = init_global_list();
  print_global_contents(head);

  modify_global_node(head, "$array[2]", 12, PRINT_REPORT);

  if(pthread_create(&t1, NULL, foo, NULL)){
    fprintf(stderr, "error pthread_create\n");
    return 1;
  }
  // current = create_global_array(head, "array", 3);
  // realloc_global_array(head, current, "array", 4, 5);

  // add_global_node(head, NULL, "name1", 1);
  print_global_contents(head);
  // if(find_global_name(head, "name1", 1, PRINT_REPORT) == NULL){
  //   printf("Did not find it.\n");
  //   exit(1);
  // }
  // add_global_node(head, NULL, "name2", 2);
  // print_global_contents(head);



  // mtx_res = pthread_mutex_lock((pthread_mutex_t*)&(head->nxt_mtx));
  // if(mtx_res){
  //   fprintf(stderr, "error with lock: %s\n", strerror(mtx_res));
  //   return 1;
  // }

  // pthread_mutex_t mtx;
  // mtx_init(&mtx, __LINE__);
  // mtx_lock(&mtx, __LINE__);
  // mtx_unlock(&mtx, __LINE__);
  // mtx_destroy(&mtx, __LINE__);

  printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
  pthread_join(t1, NULL);
  destroy_global_list(head, PRINT_REPORT);

  return 0;
}
