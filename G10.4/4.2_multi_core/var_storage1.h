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
#include <pthread.h>
#include "mtx.h"

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

typedef volatile struct local_variable{
  char *name;
  int value;
  volatile struct local_variable *  next;
  volatile struct local_variable *  prev;
}local_varT;

typedef volatile struct global_variable{
  char *name;
  int value;
  pthread_mutex_t nxt_mtx;
  volatile struct global_variable *  next;
}global_varT;

/******************************* locals ***************************************/
local_varT* init_local_list();

void destroy_local_list(local_varT *head, int print_flag);
void abort_local_function(local_varT *head);

local_varT* add_local_node(local_varT *head, local_varT *current, char *new_name, int new_value);

void print_local_contents(local_varT *head);
/*if 1: create it, if 0: require its existence*/
local_varT *find_local_name(local_varT *head, char name[], int lvalue, int print_flag);

/* name: e.g. "argv[3]", lvalue: if 1: create it, if 0: require its existence*/
local_varT *find_local_array_name(local_varT *head, char name[], int lvalue, int print_flag);
local_varT *create_local_array(local_varT *head, char array_name[], int new_last_index);
local_varT *realloc_local_array(local_varT *head, local_varT *current, char array_name[], int old_last_index, int new_last_index);

void modify_local_node(local_varT *head, char name[], int new_value, int print_flag);
int read_local_node(local_varT *head, char name[], int print_flag);

/****************************** globals ***************************************/
global_varT* init_global_list();
void destroy_global_list(global_varT *head, int print_flag);
void abort_global_function(global_varT *head);

global_varT* add_global_node(global_varT *head, global_varT *current, char *new_name, int new_value);

void print_global_contents(global_varT *head);

/*if 1: create it, if 0: require its existence*/
global_varT *find_global_name(global_varT *head, char name[], int lvalue, int print_flag);

/* name: e.g. "argv[3]", lvalue: if 1: create it, if 0: require its existence*/
global_varT *find_global_array_name(global_varT *head, char name[], int lvalue, int print_flag);
global_varT *find_global_head_prev(global_varT *head);
global_varT *create_global_array(global_varT *head, char array_name[], int new_last_index);
global_varT *realloc_global_array(global_varT *head, global_varT *current, char array_name[], int old_last_index, int new_last_index);

void modify_global_node(global_varT *head, char name[], int new_value, int print_flag);
int read_global_node(global_varT *head, char name[], int print_flag);

/******************************************************************************/

#endif
