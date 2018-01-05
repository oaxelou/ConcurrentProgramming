#include "program_handler1.h"
#include "parser.h"
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define VAR_SIZE 20
#define PROGRAM_NAME_S 20

int check_tag(int fd){
  char input_buffer[100];
  int i;
  my_read(fd, input_buffer, __LINE__);
  if (input_buffer[0] != '#'){
    fprintf(stderr, "Program Tag does not start with '#'\n");
    close(fd);
    return 1;
  }

  discard_spaces(fd, input_buffer, ALLOW_N_LINE_CHAR);
  i = read_island(fd, input_buffer);
  input_buffer[i] = '\0';

  if (strcmp(input_buffer, PROGRAM_TAG)!=0){
    fprintf(stderr, "Incorrect Program Tag\n");
    return 1;
  }
  return 0;
}

void take_arguments(int **arguments){
  int i, *args;
  char byte;

  args = (int *)malloc(3 * sizeof(int));
  if(args == NULL){
    fprintf(stderr, "Error with malloc\n");
    exit(1);
  }
  args[0] = 1234; //program_id
  args[1] = 0; //argc
  i = 2;

  while(1){
    byte = getchar();
    if(byte == '\n'){
      break;
    }
    scanf("%d", args + i);
    args[1]++;
    args = (int *) realloc(args, (i + 2) * sizeof(int));
    if(args == NULL){
      fprintf(stderr, "Realloc couldn't realloc\n");
    }
    i++;
  }
  *arguments = args;
}

int main (int argc, char *argv[]){
  int i, fd;
  int *args = NULL;
  char format_str_p[20];
  char program_name[PROGRAM_NAME_S], var_name[VAR_SIZE];
  programT *program_node;

  // INITIALISATIONS
  sprintf(format_str_p, "%%%ds", PROGRAM_NAME_S - 1);

  program_node = (programT*)malloc(sizeof(programT));
  if (program_node == NULL){
    fprintf(stderr, "error with malloc in add_program\n");
    exit(1);
  }

  while(1){

    /* READ PROGRAM NAME */
    printf("Enter program name to run: ");
    scanf(format_str_p, program_name);

    fd = open(program_name, O_RDWR, S_IRWXU);
    if(fd == -1){
      perror("open");
    }
    else{
      break;
    }
  }

  globals = init_list();
  program_node->labels = init_labels();
  program_node->locals = init_list();
  program_node->fd = fd;
  program_node->running = 1;
  program_node->name = strdup(program_name);  if(program_node->name == NULL){fprintf(stderr, "error with strdup\n");exit(1);}

  take_arguments(&args);
  /******************* INITIALISATIONS AND TAG CHECK *************************/

  add_node(program_node->locals, program_node->locals->prev, "$argv[0]", args[0]);
  for(i = 0; i < args[1]; i++){
    sprintf(var_name , "$argv[%d]", i + 1);
    add_node(program_node->locals, program_node->locals->prev, var_name, args[i + 2]);
  }
  add_node(program_node->locals, program_node->locals->prev, "$argc", args[1] + 1);

  if(check_tag(fd)){exit(1);}

  /************************* PROGRAM READY TO RUN ****************************/
  printf("\n"ANSI_COLOR_GREEN"PROGRAM JUST STARTED !"ANSI_COLOR_RESET"\n\n");
  while(program_node->running){
    main_program(program_node);
  }

  printf("\n"ANSI_COLOR_GREEN"PROGRAM RETURNED SUCCESSFULLY !"ANSI_COLOR_RESET"\n\n");

  
  
  close(program_node->fd);

  #ifdef debug
    printf("\nLabels:\n");destroy_labels(program_node->labels, PRINT_REPORT);
  #else
    destroy_labels(program_node->labels, !PRINT_REPORT);
  #endif

  #ifdef debug
    printf("\nLocals:\n");destroy_list(program_node->locals, PRINT_REPORT);
  #else
    destroy_list(program_node->locals, !PRINT_REPORT);
  #endif

  #ifdef debug
    printf("\nGlobals:\n");destroy_list(globals, PRINT_REPORT);
  #else
    destroy_list(globals, !PRINT_REPORT);
  #endif


  printf("deallocating memory...\nmain terminating\n");

  return 0;
}
