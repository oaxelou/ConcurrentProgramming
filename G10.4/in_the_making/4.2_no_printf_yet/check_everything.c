#include "program_parser.h"

// varT *globals;

void *foo(void *args){
  int i[] = {12, 13};
  struct programT *program;

  program = (struct programT*)malloc(sizeof(struct programT));
  if(program == NULL){
    fprintf(stderr, "Error allocating memory for program\n");
    exit(1);
  }

  program->locals = init_list();
  globals = init_list();
  main_program(0, 2, i, "print", program);

  destroy_list(program->locals, PRINT_REPORT);
  free(program);
  return NULL;
}
void *foo1(void *args){
  int i[] = {11, 15, 20};
  struct programT *program;

  program = (struct programT*)malloc(sizeof(struct programT));
  if(program == NULL){
    fprintf(stderr, "Error allocating memory for program\n");
    exit(1);
  }

  program->locals = init_list();
  globals = init_list();
  main_program(1, 3, i, "set", program);

  destroy_list(program->locals, PRINT_REPORT);
  free(program);
  return NULL;
}
void *foo2(void *args){
  int i[] = {11, 15, 22, 30};
  struct programT *program;

  program = (struct programT*)malloc(sizeof(struct programT));
  if(program == NULL){
    fprintf(stderr, "Error allocating memory for program\n");
    exit(1);
  }

  program->locals = init_list();
  globals = init_list();
  main_program(1, 4, i, "load", program);

  destroy_list(program->locals, PRINT_REPORT);
  free(program);
  return NULL;
}

int main(int argc,char *argv[]){
  pthread_t t1, t2, t3;

  if(pthread_create(&t1, NULL, foo, NULL)){
    fprintf(stderr, "error pthread_create\n");
    return 1;
  }
  if(pthread_create(&t2, NULL, foo1, NULL)){
    fprintf(stderr, "error pthread_create\n");
    return 1;
  }
  if(pthread_create(&t3, NULL, foo2, NULL)){
    fprintf(stderr, "error pthread_create\n");
    return 1;
  }
  // find_global_head_prev(head);
  // modify_global_node(head, "$array[2]", 11, PRINT_REPORT);
  // current = create_global_array(head, "array", 3);
  // realloc_global_array(head, current, "array", 4, 5);

  // add_global_node(head, NULL, "name1", 1);
  // print_global_contents(head);
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

  pthread_join(t1, NULL);
  pthread_join(t2, NULL);
  pthread_join(t3, NULL);
  printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
  destroy_list(globals, PRINT_REPORT);
  mtx_destroy(&glob_mtx, __LINE__);
  // print_global_contents(head);
  // destroy_global_list(head, PRINT_REPORT);

  return 0;
}
