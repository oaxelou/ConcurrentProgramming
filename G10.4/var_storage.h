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

typedef volatile struct localVariable{
  char *name;
  int value;
  volatile struct localVariable *  next;
  volatile struct localVariable *  prev;
}localVar;

void destroy_list(localVar *head, int print_flag);
void abort_function(localVar *head);
void print_contents(localVar *head);

localVar* add_node(localVar *head, localVar *current, char *new_name, int new_value);

localVar *find_name(localVar *head, char name[], int lvalue, int print_flag);
localVar *find_array_name(localVar *head, char name[], int lvalue, int print_flag);

localVar *create_array(localVar *head, char array_name[], int new_last_index);
localVar *realloc_array(localVar *head, localVar *current, char array_name[], int old_last_index, int new_last_index);

int modify_node(localVar *head, char name[], int new_value, int print_flag);
int read_node(localVar *head, char name[], int print_flag);

/******************************************************************************/

#endif
