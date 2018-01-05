#include "program_handler1.h"

//initialises a list
programT* init_program_list(){
  programT *head;
  head = (programT*)malloc(sizeof(programT));
	if (head == NULL){
		printf("error with malloc in init_program_list\n");
		exit(1);
	}

  head->name = NULL;
  head->core = INIT_CORE;
  head->running = RUNNING;
	head->next = head;
	head->prev = head;
  head->locals = NULL;
  head->blocked = 0;
  head->sleep_start = -1;
  head->sleep_time = -1;
  head->down_sem = NULL;
  head->woke_up = 0;
  head->running_now = 0;

  return (head);
}

programT *add_program(programT *head, char *new_name, int new_core, int new_id, varT *locals, labelsT *labels, int fd){
  programT *new_node;

  new_node = (programT*)malloc(sizeof(programT));
  if (new_node == NULL){
    fprintf(stderr, "error with malloc in add_program\n");
    destroy_programs(head, !PRINT_REPORT);
    exit(1);
  }

  if (new_name != NULL){
    new_node->name = (char*)malloc(sizeof(char) * (strlen(new_name) + 1));
    if (new_node->name == NULL){
      printf("error with malloc in add_program\n");
      destroy_programs(head, !PRINT_REPORT);
  		exit(1);
    }
    strcpy(new_node->name,new_name);
  }
  new_node->core = new_core;
  new_node->id = new_id;
  new_node->running = RUNNING;
  new_node->locals = locals;
  new_node->labels = labels;
  new_node->fd = fd;
  new_node->blocked = 0;
  new_node->sleep_start = -1;
  new_node->sleep_time = -1;
  head->woke_up = 0;
  head->running_now = 0;

  new_node->next = head->prev->next;
  new_node->prev = head->prev;
  head->prev->next = new_node;
  new_node->next->prev = new_node;

  return new_node;
}

programT* search_program_id(programT *head, int id, int print_flag){
	programT *current;

	for (current = head->next; current->name != NULL; current = current->next){
		if(current->id == id){
      if(print_flag == PRINT_REPORT){
        printf("Found program with id: %d\n", id);
      }
      return(current);
    }
	}
  if(print_flag == PRINT_REPORT){
    printf("Not found program with id: %d\n", id);
  }
  return(NULL);
}

void destroy_programs(programT *head, int print_flag){
  programT *current = head->next->next;
  if(print_flag == PRINT_REPORT)
    printf("***** FREE programs    ****\n");
  while(current->prev != head){
    //free ta pedia tou current
    if(print_flag == PRINT_REPORT)
      printf("program to free: %d\n", current->prev->id);
    free(current->prev->name);
    free((struct program_struct *)current->prev);

    current = current->next;
  }
  free((struct program_struct *)head);
}

int remove_program(programT *current){

  current->prev->next = current->next;
  current->next->prev = current->prev;

  free(current->name);
  free((struct program_struct *)current);

  return 0;
}

void print_programs(programT *head){
  programT *current;

  printf("\n* * * * * * * * * *\nPrograms:\n");

  for (current = head->next; current->name != NULL; current = current->next){
		printf("id: %d name: %s core: %d ", current->id, current->name, current->core);
    if(current->running == RUNNING){
      printf("running");
      if(current->blocked){
        printf(" (blocked) ");
        if(current->sleep_start >= 0){
          printf("sleep_time = %d sleep_start = %ld ", current->sleep_time, (long int)current->sleep_start);
        }
        else if(current->down_sem != NULL){
          printf("'%s' value = %d",current->down_sem->name, current->down_sem->value);
        }
        else{
          fprintf(stderr, "wrong wrong wrong\n");destroy_programs(head, !PRINT_REPORT);exit(1);
        }
      }
	   }
     printf("\n");
  }
  printf("\n* * * * * * * * * *\n");
  // printf("#########################\n");
}

int find_less_busy_core(programT *head, int nofcores){
  int min;
  int min_core, i;
  int programs_per_core[nofcores];
  programT *current;

  for(i = 0; i < nofcores; i++){
    programs_per_core[i] = 0;
  }

  for(current = head->next; current->name != NULL; current = current->next){
    if(current->blocked == 0){
      programs_per_core[current->core]++;
    }
  }

  min = programs_per_core[0];
  min_core = 0;
  for(i = 0; i < nofcores; i++){
    if(programs_per_core[i] < min){
      min = programs_per_core[i];
      min_core = i;
    }
  }
  return min_core;
}

void rebalance_cores(programT *head, int nofcores){
  programT *current;
  int running_core, blocked_core;

  running_core = blocked_core = 0;
  for(current = head->next; current != head; current = current->next){
    if(current->running && !current->running_now){
      if(current->blocked){
        current->core = blocked_core % nofcores;
        blocked_core++;printf("blocked_core = %d\n", blocked_core);
      }
      else{
        current->core = running_core % nofcores;
        running_core++;printf("running_core = %d\n", running_core);
      }
    }
    else{

      // remove_program(current);
    }
  }
}
