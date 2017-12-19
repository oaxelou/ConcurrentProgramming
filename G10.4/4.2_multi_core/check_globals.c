#include "var_storage1.h"
#include <errno.h>
// #include <stdio.h>
// #include <pthread.h>
// #include <string.h>

int main(int argc,char *argv[]){
  // pthread_t t1;
  global_varT *head;

  head = init_global_list();
  print_global_contents(head);

  add_global_node(head, head, "name1", 1);
  print_global_contents(head);
  add_global_node(head, head, "name2", 2);
  print_global_contents(head);

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
  destroy_global_list(head, PRINT_REPORT);

  return 0;
}
