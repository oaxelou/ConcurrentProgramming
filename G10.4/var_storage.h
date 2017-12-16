#ifndef __VAR_STORAGE_H__
#define __VAR_STORAGE_H__

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define NO_TYPE -13
#define PRINT_REPORT 1
#define SIZE 11
#define N 10
#define NO_DIGIT 5
#define DOESNT_NOT_EXIT -1
#define CREATE_PERMISSION 1

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

typedef volatile struct variable{
  char *name;
  int value;
  volatile struct variable *  next;
  volatile struct variable *  prev;
}varT;

void destroy_list(varT *head, int print_flag);
void abort_function(varT *head);
varT* init_list();
void print_contents(varT *head);

varT* add_node(varT *head, varT *current, char *new_name, int new_value);

varT *find_name(varT *head, char name[], int lvalue, int print_flag);
varT *find_array_name(varT *head, char name[], int lvalue, int print_flag);

varT *create_array(varT *head, char array_name[], int new_last_index);
varT *realloc_array(varT *head, varT *current, char array_name[], int old_last_index, int new_last_index);

int modify_node(varT *head, char name[], int new_value, int print_flag);
int read_node(varT *head, char name[], int print_flag);

/******************************************************************************/

#endif
