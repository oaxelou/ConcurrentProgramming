#include "labels.h"

//initialises a list
labelsT* init_labels(){
  labelsT *head;
  head = (labelsT*)malloc(sizeof(labelsT));
	if (head == NULL){
		printf("error with malloc in init_labels\n");
		exit(1);
	}

  head->name = NULL;
  head->offset = INIT_OFFSET;
	head->next = head;
	head->prev = head;

  return (head);
}

void add_label(labelsT *head, char *new_name, off_t new_offset){ //opou new_offset = current offset - strlen(label_name)
  labelsT *new_node;

  if (search_label(head, new_name, !PRINT_REPORT) != INVALID_OFFSET){//label already exists in list
    return;
  }

  new_node = (labelsT*)malloc(sizeof(labelsT));
  if (new_node == NULL){
    fprintf(stderr, "error with malloc in add_label\n");
    destroy_labels(head, !PRINT_REPORT);
		exit(1);
  }

  if (new_name != NULL){
    new_node->name = (char*)malloc(sizeof(char) * (strlen(new_name) + 1));
    if (new_node->name == NULL){
      printf("error with malloc in add_label\n");
      destroy_labels(head, !PRINT_REPORT);
  		exit(1);
    }
    strcpy(new_node->name,new_name);
  }
  new_node->offset = new_offset;

  new_node->next = head->prev->next;
  new_node->prev = head->prev;
  head->prev->next = new_node;
  new_node->next->prev = new_node;
}

off_t search_label(labelsT *head, char name[], int print_flag){
	labelsT *current;

	for (current = head->next; current->name != NULL; current = current->next){
		if(strcmp(name, current->name) == 0){
      if(print_flag == PRINT_REPORT){
        printf("Found label with name: %s\n", name);
      }
      return(current->offset);
    }
	}
  if(print_flag == PRINT_REPORT){
    printf("Not found label with name: %s. Going to add it\n", name);
  }
  return(INVALID_OFFSET);
}

void destroy_labels(labelsT *head, int print_flag){
  labelsT *current = head->next->next;
  if(print_flag == PRINT_REPORT)
    printf("***** FREE labels    ****\n");
  while(current->prev != head){
    //free ta pedia tou current
    if(print_flag == PRINT_REPORT)
      printf("label to free: %s\n", current->prev->name);
    free(current->prev->name);
    free((struct label_struct *)current->prev);

    current = current->next;
  }
  free((struct label_struct *)head);
}

void print_labels(labelsT *head){
  labelsT *current;

  printf("<Labels List>\n");

  for (current = head->next; current->name != NULL; current = current->next){
		printf("%s %d\n", current->name, (int)current->offset);
	}
  printf("</Labels List>\n");
}
