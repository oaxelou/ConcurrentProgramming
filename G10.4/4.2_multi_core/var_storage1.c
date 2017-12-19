#include "var_storage1.h"

local_varT* init_local_list(){
  local_varT *head;
  head = (local_varT*)malloc(sizeof(local_varT));
	if (head == NULL){
		printf("error with malloc in init_local_list\n");
		exit(1);
	}

  head->name = NULL;
  head->value = NO_TYPE;
	head->next = head;
	head->prev = head;

  return (head);
}

global_varT* init_global_list(){
  global_varT *head;
  head = (global_varT*)malloc(sizeof(global_varT));
  if (head == NULL){
    printf("error with malloc in init_global_list\n");
    exit(1);
  }

  head->name = NULL;
  head->value = NO_TYPE;
  head->next = head;

 mtx_init((pthread_mutex_t*)&(head->nxt_mtx), __LINE__);

  return (head);
}

void destroy_local_list(local_varT *head, int print_flag){
  local_varT *current = head->next->next;

  if(print_flag == PRINT_REPORT)
    printf("***** FREE variables ****\n");

  while(current->prev != head){
    //free ta pedia tou current
    if(print_flag == PRINT_REPORT)
    printf("name to free: %s\n", current->prev->name);
    free(current->prev->name);
    free((struct local_variable *)current->prev);

    current = current->next;
  }

  free((struct local_variable *)head);
}

void destroy_global_list(global_varT *head, int print_flag){
  global_varT *current = head->next->next;
  global_varT *prev = head->next;

  if(print_flag == PRINT_REPORT)
    printf("***** FREE variables ****\n");

  while(prev != head){
    //free ta pedia tou current
    if(print_flag == PRINT_REPORT)
    printf("name to free: %s\n", prev->name);
    free(prev->name);

    // for nxt_mtx
    // mtx_unlock((pthread_mutex_t*) &(prev->nxt_mtx), __LINE__);
    mtx_destroy((pthread_mutex_t*)&(prev->nxt_mtx), __LINE__);

    free((struct global_variable *)prev);
    prev = current;
    current = current->next;
  }

  free((struct global_variable *)head);
}

local_varT* add_local_node(local_varT *head, local_varT *current, char *new_name, int new_value){
  local_varT *new_node;

  new_node = (local_varT*)malloc(sizeof(local_varT));
  if (new_node == NULL){
    fprintf(stderr, "error with malloc in add_node\n");
		exit(1);
    // abort_function(head);
  }

  if (new_name != NULL){
    new_node->name = (char*)malloc(sizeof(char) * (strlen(new_name) + 1));
    if (new_node->name == NULL){
      printf("error with malloc in add_node\n");
  		exit(1);
      // abort_function(head);
    }
    strcpy(new_node->name,new_name);
  }
  new_node->value = new_value;

  new_node->next = current->next;
  new_node->prev = current;
  current->next = new_node;
  new_node->next->prev = new_node;

  return new_node;
}

global_varT* add_global_node(global_varT *head, global_varT *current, char *new_name, int new_value){
  global_varT *new_node;

  //find current (h head->prev)

  new_node = (global_varT*)malloc(sizeof(global_varT));
  if (new_node == NULL){
    fprintf(stderr, "error with malloc in add_node\n");
		exit(1);
    // abort_function(head);
  }

  if (new_name != NULL){
    new_node->name = (char*)malloc(sizeof(char) * (strlen(new_name) + 1));
    if (new_node->name == NULL){
      printf("error with malloc in add_node\n");
  		exit(1);
      // abort_function(head);
    }
    strcpy(new_node->name,new_name);
  }
  new_node->value = new_value;
  // init mtx
  mtx_init((pthread_mutex_t*)&(new_node->nxt_mtx), __LINE__);

  new_node->next = current->next;

  printf("nxt:\n");
  mtx_lock((pthread_mutex_t*)&(current->nxt_mtx), __LINE__);
  current->next = new_node;
  mtx_unlock((pthread_mutex_t*)&(current->nxt_mtx), __LINE__);
  printf("nxt ok\n");

  return new_node;
}

void print_local_contents(local_varT *head){
  local_varT *current;

  printf("<List>\n");

  for (current = head->next; current->name != NULL; current = current->next){
		printf("%s %d\n", current->name, current->value);
	}
  printf("</List>\n");
  // printf("#########################\n");
}

void print_global_contents(global_varT *head){
  global_varT *current, *prev;

  printf("<List>\n");

  mtx_lock((pthread_mutex_t *)&head->nxt_mtx, __LINE__);
  current = head->next;
  mtx_unlock((pthread_mutex_t *)&head->nxt_mtx, __LINE__);

  for (; current->name != NULL; ){
		printf("%s %d\n", current->name, current->value);

    mtx_lock((pthread_mutex_t *)&current->nxt_mtx, __LINE__);
    prev = current;
    current = current->next;
    mtx_unlock((pthread_mutex_t *)&prev->nxt_mtx, __LINE__);
	}
  printf("</List>\n");
  // printf("#########################\n");
}

/******************************************************************************/
