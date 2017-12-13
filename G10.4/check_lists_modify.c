#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <pthread.h>

//DEFINE A CODE FOR EACH INSTRUCTION value?
#define NO_TYPE -13
#define PRINT_REPORT 1
#define SIZE 11
#define N 10
#define NO_DIGIT 5
#define DOESNT_NOT_EXIT -1
#define CREATE_PERMISSION 1

typedef volatile struct localVariable{
  char *name;
  int value;
  volatile struct localVariable *  next;
  volatile struct localVariable *  prev;
}localVar;

localVar *volatile head;

//initialises a list
localVar* init_list(int line){
  head = (localVar*)malloc(sizeof(localVar));
	if (head == NULL){
		printf("error with malloc in init_list at line: %d\n", line);
		exit(1);
	}

  head->name = NULL;
  head->value = NO_TYPE;
	head->next = head;
	head->prev = head;

  return (head);
}

localVar* add_node(localVar *current, char *new_name, int new_value, int line){
  localVar *new_node;

  new_node = (localVar*)malloc(sizeof(localVar));
  if (new_node == NULL){
    printf("error with malloc in add_node at line: %d\n", line);
		exit(1);
  }

  if (new_name != NULL){
    new_node->name = (char*)malloc(sizeof(char) * (strlen(new_name) + 1));
    if (new_node->name == NULL){
      printf("error with malloc in add_node at line: %d\n", line);
  		exit(1);
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
localVar *find_name(char name[], int lvalue/*if 1: create it, if 0: require its existence*/, int print_flag){
	localVar *current;

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
    return add_node(head->prev, name, 0, __LINE__);
  }
  else{
    return NULL;
  }
}

localVar *create_array(char array_name[], int new_last_index/*dhladh atoi(name)*/){
  int i;
  char name[strlen(array_name) + 2 + NO_DIGIT];
  localVar* last_array_cell;

  for(i = 0; i <= new_last_index; i++){
    sprintf(name, "%s[%d]", array_name, i);
    last_array_cell = add_node(head->prev, name, 0, __LINE__);
  }
  return last_array_cell;
}

localVar *realloc_array(localVar *current, char array_name[], int old_last_index, int new_last_index){
  int i;
  char name[strlen(array_name) + 2 + NO_DIGIT];
  localVar* last_array_cell;

  for(i = old_last_index; i <= new_last_index; i++, current = current->next){
    sprintf(name, "%s[%d]", array_name, i);
    last_array_cell = add_node(current, name, 0, __LINE__);
  }
  return last_array_cell;
}

localVar *find_array_name(char name[] /*tou stul "argv[3]" */, int lvalue/*if 1: create it, if 0: require its existence*/, int print_flag){
  localVar *current;
  int array_area = 0;
  char *array_name, *temp;

  array_name = (char *)malloc(sizeof(char) * (strlen(name) + 1));
  strcpy(array_name, name);
  temp = strchr(array_name, '[');
  if(temp == NULL){
    printf("Not an array.\n");
    exit(1);
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
      return(realloc_array(current->prev, array_name, \
                           atoi(current->prev->name + strlen(array_name) + 1), \
                           atoi(name + strlen(array_name) + 1)));
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
    return (realloc_array(head->prev, array_name, \
            atoi(head->prev->name + strlen(array_name) + 1), \
            atoi(name + strlen(array_name) + 1)));
  }
  else{
    if(print_flag == PRINT_REPORT){
      printf("Nothing of type \"array\" in list\n");
    }
    return create_array(array_name, atoi(name + strlen(array_name) + 1));
  }
}


void print_contents(){
  localVar *current;

  printf("<List>\n");

  for (current = head->next; current->name != NULL; current = current->next){
		printf("%s %d\n", current->name, current->value);
	}
  printf("</List>\n");
  // printf("#########################\n");
}

// Returns 0: Success
// Returns 1: Failure
int modify_node(char name[], int new_value, int print_flag){
  localVar *current;

  if(strchr(name, '[') != NULL){
    current = find_array_name(name, CREATE_PERMISSION, print_flag);
  }
  else{
    current = find_name(name, CREATE_PERMISSION, print_flag);
  }
  if(current == NULL){
    printf("Sth went terribly wrong with modify_node\n");
    exit(1);
  }

  current->value = new_value;
  return 0;
}

void destroy_list(localVar *head){
  localVar *current = head->next->next;

  while(current->prev != head){
    //free ta pedia tou current
    printf("name to free: %s\n", current->prev->name);
    free(current->prev->name);
    free((struct localVariable *)current->prev);

    current = current->next;
  }

  free((struct localVariable *)head);
}

/******************************************************************************/

int main(int argc, char *argv[]) {
  // localVar *current;
  int i, value;
  char temp[10];

  head = init_list(__LINE__);

  print_contents();

  // add_node(head->prev, "name[0]", 12, __LINE__);
  // add_node(head->prev, "name[1]", 10, __LINE__);
  // add_node(head->prev, "temp", 9, __LINE__);
  // print_contents();
  //
  // current = find_array_name("name[6]", CREATE_PERMISSION, !PRINT_REPORT);
  // print_contents();
  // current = find_array_name("name[2]", CREATE_PERMISSION, PRINT_REPORT);
  // print_contents();

  for(i = 0; i < 3; i++){
    printf("Enter int to modify: ");
    scanf("%9s %d", temp, &value);

    modify_node(temp, value, PRINT_REPORT);
    print_contents();
  }

  destroy_list(head);

  return 0;
}
