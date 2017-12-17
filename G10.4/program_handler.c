#include "program_handler.h"

//initialises a list
programT* init_program_list(){
  programT *head;
  head = (programT*)malloc(sizeof(programT));
	if (head == NULL){
		printf("error with malloc in init_program_list\n");
		exit(1);
	}

  head->name = NULL;
  head->pid = INIT_PID;
  head->running = RUNNING;
	head->next = head;
	head->prev = head;

  return (head);
}

void add_program(programT *head, char *new_name, pthread_t new_pid, int new_id){
  programT *new_node;

  new_node = (programT*)malloc(sizeof(programT));
  if (new_node == NULL){
    fprintf(stderr, "error with malloc in add_program\n");
		exit(1);
    // abort_function(head);
  }

  if (new_name != NULL){
    new_node->name = (char*)malloc(sizeof(char) * (strlen(new_name) + 1));
    if (new_node->name == NULL){
      printf("error with malloc in add_program\n");
  		exit(1);
      // abort_function(head);
    }
    strcpy(new_node->name,new_name);
  }
  new_node->pid = new_pid;
  new_node->id = new_id;
  new_node->running = RUNNING;

  new_node->next = head->prev->next;
  new_node->prev = head->prev;
  head->prev->next = new_node;
  new_node->next->prev = new_node;
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
    printf("Error 404: Not found program with id: %d\n", id);
  }
  return(NULL);
}

void kill_all(programT *head, int print_flag){
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

int kill_program(programT *head, int id, int print_flag){
  programT *current;

  current = search_program_id(head, id, print_flag);
  if(current == NULL){
    if(print_flag){
      fprintf(stderr, "What is dead may never die. id = %d\n", id);
    }
    return 1;
  }

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
		printf("%d %s %lu ", current->id, current->name, (long unsigned)current->pid);
    if(current->running == RUNNING){
      printf("running");
    }
    printf("\n");
	}
  printf("\n* * * * * * * * * *\n");
  // printf("#########################\n");
}
