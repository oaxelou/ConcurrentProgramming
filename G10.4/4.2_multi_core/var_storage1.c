#include "var_storage1.h"

/******************************* locals ***************************************/
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
void abort_local_function(local_varT *head){
  destroy_local_list(head, PRINT_REPORT);
  exit(1);
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

void print_local_contents(local_varT *head){
  local_varT *current;

  printf("<List>\n");

  for (current = head->next; current->name != NULL; current = current->next){
		printf("%s %d\n", current->name, current->value);
	}
  printf("</List>\n");
  // printf("#########################\n");
}

local_varT *find_local_name(local_varT *head, char name[], int lvalue/*if 1: create it, if 0: require its existence*/, int print_flag){
	local_varT *current;

	for (current = head->next; current->name != NULL; current = current->next){
		if(strcmp(name, current->name) == 0){
      if(print_flag == PRINT_REPORT){
        printf("Found node with name: %s\n", name);
      }
      return(current);
    }
	}
  if(print_flag == PRINT_REPORT){
    printf("Error 404: Not found node with name: %s\n", name);
  }
  if(lvalue){
    return add_local_node(head, head->prev, name, 0);
  }
  else{
    return NULL;
  }
}

local_varT *create_local_array(local_varT *head, char array_name[], int new_last_index){
  int i;
  char name[strlen(array_name) + 2 + NO_DIGIT];
  local_varT* last_array_cell;

  for(i = 0; i <= new_last_index; i++){
    sprintf(name, "%s[%d]", array_name, i);
    last_array_cell = add_local_node(head, head->prev, name, 0);
  }
  return last_array_cell;
}
local_varT *realloc_local_array(local_varT *head, local_varT *current, char array_name[], int old_last_index, int new_last_index){
  int i;
  char name[strlen(array_name) + 2 + NO_DIGIT];
  local_varT* last_array_cell;

  for(i = old_last_index; i <= new_last_index; i++, current = current->next){
    sprintf(name, "%s[%d]", array_name, i);
    last_array_cell = add_local_node(head, current, name, 0);
  }
  return last_array_cell;
}
local_varT *find_local_array_name(local_varT *head, char name[] /*tou stul "argv[3]" */, int lvalue/*if 1: create it, if 0: require its existence*/, int print_flag){
  local_varT *current;
  int array_area = 0;
  char *array_name, *temp;

  array_name = (char *)malloc(sizeof(char) * (strlen(name) + 1));
  strcpy(array_name, name);
  temp = strchr(array_name, '[');
  if(temp == NULL){
    printf("Not an array.\n");
    // exit(1);
    abort_local_function(head);
  }
  temp[0] = '\0';

  for (current = head->next; current->name != NULL; current = current->next){
    if(!strncmp(current->name, array_name, strlen(array_name)) && !array_area){
      array_area = 1;
    }
    else if(strncmp(current->name, array_name, strlen(array_name)) && array_area){
      if(print_flag == PRINT_REPORT){
        printf("Error 404: End of array and node not found with name: %s\n", name);
      }
      if(lvalue){
        return(realloc_local_array(head, current->prev, array_name, \
               atoi(current->prev->name + strlen(array_name) + 1), \
               atoi(name + strlen(array_name) + 1)));
      }
      else{
        return NULL;
      }
    }

    if(array_area && strcmp(current->name, name) == 0){
      if(print_flag == PRINT_REPORT){
        printf("Found node with name: %s\n", name);
      }
      return(current);
    }
  }

  if(array_area == 1){
    if(print_flag == PRINT_REPORT){
      printf("Last node of the list is type \"array\"\n");
    }
    if(lvalue){
      return (realloc_local_array(head, head->prev, array_name, \
              atoi(head->prev->name + strlen(array_name) + 1), \
              atoi(name + strlen(array_name) + 1)));
    }
    else{
      return NULL;
    }
  }
  else{
    if(print_flag == PRINT_REPORT){
      printf("Nothing of type \"array\" in list\n");
    }
    if(lvalue){
      return create_local_array(head, array_name, atoi(name + strlen(array_name) + 1));
    }
    else{
      return NULL;
    }
  }
}

void modify_local_node(local_varT *head, char name[], int new_value, int print_flag){
  local_varT *current;

  if(strchr(name, '[') != NULL){
    current = find_local_array_name(head, name, CREATE_PERMISSION, print_flag);
  }
  else{
    current = find_local_name(head, name, CREATE_PERMISSION, print_flag);
  }
  if(current == NULL){
    printf("Sth went terribly wrong with modify_node\n");
    // exit(1);
    abort_local_function(head);
  }

  current->value = new_value;
}
int read_local_node(local_varT *head, char name[], int print_flag){
  local_varT *current;

  if(strchr(name, '[') != NULL){
    current = find_local_array_name(head, name, !CREATE_PERMISSION, print_flag);
  }
  else{
    current = find_local_name(head, name, !CREATE_PERMISSION, print_flag);
  }
  if(current == NULL){
    fprintf(stderr, "Syntax error: %s used uninitialised in this.\n", name);
    // exit(1);
    abort_local_function(head);
  }

  return current->value;
}


/****************************** globals ***************************************/
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
void abort_global_function(global_varT *head){
  destroy_global_list(head, PRINT_REPORT);
  exit(1);
}

global_varT* add_global_node(global_varT *head, global_varT *current, char *new_name, int new_value){
  global_varT *new_node;

  //find current (h head->prev)
  if(current == NULL){
    current = find_global_head_prev(head);
    // if(current != head)
      // printf("current->name = %s\n", current->name);
  }

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

global_varT *find_global_head_prev(global_varT *head){
  global_varT *current, *prev, *next;

  mtx_lock((pthread_mutex_t *)&head->nxt_mtx, __LINE__);
  current = head->next;
  mtx_unlock((pthread_mutex_t *)&head->nxt_mtx, __LINE__);

  mtx_lock((pthread_mutex_t *)&current->nxt_mtx, __LINE__);
  next = current->next;
  mtx_unlock((pthread_mutex_t *)&current->nxt_mtx, __LINE__);

  for (; next != head; ){
    mtx_lock((pthread_mutex_t *)&current->nxt_mtx, __LINE__);
    prev = current;
    current = current->next;
    next = current->next;
    mtx_unlock((pthread_mutex_t *)&prev->nxt_mtx, __LINE__);
	}

  return current;
}
global_varT *find_global_name(global_varT *head, char name[], int lvalue/*if 1: create it, if 0: require its existence*/, int print_flag){
	global_varT *current, *prev;

  mtx_lock((pthread_mutex_t *)&head->nxt_mtx, __LINE__);
  current = head->next;
  mtx_unlock((pthread_mutex_t *)&head->nxt_mtx, __LINE__);

	for (; current->name != NULL; ){
		if(strcmp(name, current->name) == 0){
      if(print_flag == PRINT_REPORT){
        printf("Found node with name: %s\n", name);
      }
      return(current);
    }

    mtx_lock((pthread_mutex_t *)&current->nxt_mtx, __LINE__);
    prev = current;
    current = current->next;
    mtx_unlock((pthread_mutex_t *)&prev->nxt_mtx, __LINE__);

	}
  if(print_flag == PRINT_REPORT){
    printf("Error 404: Not found node with name: %s\n", name);
  }
  if(lvalue){
    return add_global_node(head, NULL, name, 0);
  }
  else{
    return NULL;
  }
}

global_varT *create_global_array(global_varT *head, char array_name[], int new_last_index){
  int i;
  char name[strlen(array_name) + 2 + NO_DIGIT];
  global_varT* last_array_cell;

  for(i = 0; i <= new_last_index; i++){
    sprintf(name, "%s[%d]", array_name, i);
    last_array_cell = add_global_node(head, NULL, name, 0);
  }
  return last_array_cell;
}
global_varT *realloc_global_array(global_varT *head, global_varT *current, char array_name[], int old_last_index, int new_last_index){
  int i;
  char name[strlen(array_name) + 2 + NO_DIGIT];
  global_varT* last_array_cell, *prev;

  for(i = old_last_index; i <= new_last_index;){
    sprintf(name, "%s[%d]", array_name, i);
    last_array_cell = add_global_node(head, current, name, 0);

    i++;
    mtx_lock((pthread_mutex_t *)&current->nxt_mtx, __LINE__);
    prev = current;
    current = current->next;
    mtx_unlock((pthread_mutex_t *)&prev->nxt_mtx, __LINE__);
  }
  return last_array_cell;
}
global_varT *find_global_array_name(global_varT *head, char name[] /*tou stul "argv[3]" */, int lvalue/*if 1: create it, if 0: require its existence*/, int print_flag){
  global_varT *current, *prev, *head_prev;
  int array_area = 0;
  char *array_name, *temp;

  array_name = (char *)malloc(sizeof(char) * (strlen(name) + 1));
  strcpy(array_name, name);
  temp = strchr(array_name, '[');
  if(temp == NULL){
    printf("Not an array.\n");
    // exit(1);
    abort_global_function(head);
  }
  temp[0] = '\0';

  prev = head;
  mtx_lock((pthread_mutex_t *)&head->nxt_mtx, __LINE__);
  current = head->next;
  mtx_unlock((pthread_mutex_t *)&head->nxt_mtx, __LINE__);

  for (; current->name != NULL;){
    if(!strncmp(current->name, array_name, strlen(array_name)) && !array_area){
      array_area = 1;
    }
    else if(strncmp(current->name, array_name, strlen(array_name)) && array_area){
      if(print_flag == PRINT_REPORT){
        printf("Error 404: End of array and node not found with name: %s\n", name);
      }
      if(lvalue){
        return(realloc_global_array(head, prev, array_name, \
               atoi(prev->name + strlen(array_name) + 1), \
               atoi(name + strlen(array_name) + 1)));
      }
      else{
        return NULL;
      }
    }

    if(array_area && strcmp(current->name, name) == 0){
      if(print_flag == PRINT_REPORT){
        printf("Found node with name: %s\n", name);
      }
      return(current);
    }

    prev = current;
    mtx_lock((pthread_mutex_t *)&current->nxt_mtx, __LINE__);
    current = current->next;
    mtx_unlock((pthread_mutex_t *)&prev->nxt_mtx, __LINE__);
  }

  if(array_area == 1){
    if(print_flag == PRINT_REPORT){
      printf("Last node of the list is type \"array\"\n");
    }
    if(lvalue){
      head_prev = find_global_head_prev(head);
      return (realloc_global_array(head, head_prev, array_name, \
              atoi(head_prev->name + strlen(array_name) + 1), \
              atoi(name + strlen(array_name) + 1)));
    }
    else{
      return NULL;
    }
  }
  else{
    if(print_flag == PRINT_REPORT){
      printf("Nothing of type \"array\" in list\n");
    }
    if(lvalue){
      return create_global_array(head, array_name, atoi(name + strlen(array_name) + 1));
    }
    else{
      return NULL;
    }
  }
}

void modify_global_node(global_varT *head, char name[], int new_value, int print_flag){
  global_varT *current;

  if(strchr(name, '[') != NULL){
    current = find_global_array_name(head, name, CREATE_PERMISSION, print_flag);
  }
  else{
    current = find_global_name(head, name, CREATE_PERMISSION, print_flag);
  }
  if(current == NULL){
    printf("Sth went terribly wrong with modify_node\n");
    // exit(1);
    abort_global_function(head);
  }

  current->value = new_value;
}
int read_global_node(global_varT *head, char name[], int print_flag){
  global_varT *current;

  if(strchr(name, '[') != NULL){
    current = find_global_array_name(head, name, !CREATE_PERMISSION, print_flag);
  }
  else{
    current = find_global_name(head, name, !CREATE_PERMISSION, print_flag);
  }
  if(current == NULL){
    fprintf(stderr, "Syntax error: %s used uninitialised in this.\n", name);
    // exit(1);
    abort_global_function(head);
  }

  return current->value;
}

/******************************************************************************/
