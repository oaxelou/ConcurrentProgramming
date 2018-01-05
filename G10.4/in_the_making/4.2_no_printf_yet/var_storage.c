#include "var_storage.h"

void destroy_list(varT *head, int print_flag){
  varT *current = head->next->next;
  if(print_flag == PRINT_REPORT)
  printf("***** FREE variables ****\n");
  while(current->prev != head){
    //free ta pedia tou current
    if(print_flag == PRINT_REPORT)
    printf("name to free: %s\n", current->prev->name);
    free(current->prev->name);
    free((struct variable *)current->prev);

    current = current->next;
  }

  free((struct variable *)head);
}

void abort_function(varT *head){
  destroy_list(head, !PRINT_REPORT);
  exit(1);
}

//initialises a list
varT* init_list(){
  varT *head;
  head = (varT*)malloc(sizeof(varT));
	if (head == NULL){
		printf("error with malloc in init_list\n");
		exit(1);
	}

  head->name = NULL;
  head->value = NO_TYPE;
	head->next = head;
	head->prev = head;

  return (head);
}

varT* add_node(varT *head, varT *current, char *new_name, int new_value){
  varT *new_node;

  new_node = (varT*)malloc(sizeof(varT));
  if (new_node == NULL){
    fprintf(stderr, "error with malloc in add_node\n");
		// exit(1);
    abort_function(head);
  }

  if (new_name != NULL){
    new_node->name = (char*)malloc(sizeof(char) * (strlen(new_name) + 1));
    if (new_node->name == NULL){
      printf("error with malloc in add_node\n");
  		// exit(1);
      abort_function(head);
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

//find the instruction with a given name
varT *find_name(varT *head, char name[], int lvalue/*if 1: create it, if 0: require its existence*/, int print_flag){
	varT *current;

	for (current = head->next; current->name != NULL; current = current->next){
		if(strcmp(name, current->name) == 0){
      if(print_flag == PRINT_REPORT){
        printf("Found node with name: %s\n", name);
      }
      return(current);
    }
	}
  if(print_flag == PRINT_REPORT){
    printf("Not found node with name: %s\n", name);
  }
  if(lvalue){
    return add_node(head, head->prev, name, 0);
  }
  else{
    return NULL;
  }
}

varT *create_array(varT *head, char array_name[], int new_last_index/*dhladh atoi(name)*/){
  int i;
  char name[strlen(array_name) + 2 + NO_DIGIT];
  varT* last_array_cell;

  for(i = 0; i <= new_last_index; i++){
    sprintf(name, "%s[%d]", array_name, i);
    last_array_cell = add_node(head, head->prev, name, 0);
  }
  return last_array_cell;
}

varT *realloc_array(varT *head, varT *current, char array_name[], int old_last_index, int new_last_index){
  int i;
  char name[strlen(array_name) + 2 + NO_DIGIT];
  varT* last_array_cell;

  for(i = old_last_index+1; i <= new_last_index; i++, current = current->next){
    sprintf(name, "%s[%d]", array_name, i);
    last_array_cell = add_node(head, current, name, 0);
  }
  return last_array_cell;
}

varT *find_array_name(varT *head, char name[] /*tou stul "argv[3]" */, int lvalue/*if 1: create it, if 0: require its existence*/, int print_flag){
  varT *current;
  int array_area = 0;
  char *array_name, *temp;

  array_name = (char *)malloc(sizeof(char) * (strlen(name) + 1));
  strcpy(array_name, name);
  temp = strchr(array_name, '[');
  if(temp == NULL){
    printf("Not an array.\n");
    // exit(1);
    abort_function(head);
  }
  temp[1] = '\0';

  for (current = head->next; current->name != NULL; current = current->next){
    if(!strncmp(current->name, array_name, strlen(array_name)) && !array_area){
      array_area = 1;
    }
    else if(strncmp(current->name, array_name, strlen(array_name)) && array_area){
      if(print_flag == PRINT_REPORT){
        printf("Error 404: End of array and node not found with name: %s\n", name);
      }
      temp[0] = '\0';
      if(lvalue){
        return(realloc_array(head, current->prev, array_name, \
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

  temp[0] = '\0';
  if(array_area == 1){
    if(print_flag == PRINT_REPORT){
      printf("Last node of the list is type \"array\"\n");
    }
    if(lvalue){
      return (realloc_array(head, head->prev, array_name, \
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
      return create_array(head, array_name, atoi(name + strlen(array_name) + 1));
    }
    else{
      return NULL;
    }
  }
}

void print_contents(varT *head){
  varT *current;

  printf("<List>\n");

  for (current = head->next; current->name != NULL; current = current->next){
		printf("%s %d\n", current->name, current->value);
	}
  printf("</List>\n");
  // printf("#########################\n");
}

// Returns 0: Success
// Returns 1: Failure
int modify_node(varT *head, char name[], int new_value, int print_flag){
  varT *current;

  if(strchr(name, '[') != NULL){
    current = find_array_name(head, name, CREATE_PERMISSION, print_flag);
  }
  else{
    current = find_name(head, name, CREATE_PERMISSION, print_flag);
  }
  if(current == NULL){
    printf("Sth went terribly wrong with modify_node\n");
    // exit(1);
    abort_function(head);
  }

  current->value = new_value;
  return 0;
}

int read_node(varT *head, char name[], int print_flag){
  varT *current;

  if(strchr(name, '[') != NULL){
    current = find_array_name(head, name, !CREATE_PERMISSION, print_flag);
  }
  else{
    current = find_name(head, name, !CREATE_PERMISSION, print_flag);
  }
  if(current == NULL){
    fprintf(stderr, "Syntax error: %s used uninitialised in this.\n", name);
    // exit(1);
    abort_function(head);
  }

  return current->value;
}

/******************************************************************************/
