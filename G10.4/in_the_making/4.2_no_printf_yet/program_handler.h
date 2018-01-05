#ifndef __PROGRAM_HANDLER_H__
#define __PROGRAM_HANDLER_H__

#include "var_storage.h"
#include "labels_commands_list.h"


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define INIT_CORE -1

#define NOT_RUNNING 0
#define RUNNING 1

#define PRINT_REPORT 1

typedef volatile struct program_struct{
  char *name;
  int core; // se poio core anhkei
  int id;
  int running;
  int blocked;
  time_t sleep_start;
  int sleep_time;
  int fd;
  varT *locals;
  labelsT *labels;
  varT *down_sem;
  int woke_up;
  int running_now;

  volatile struct program_struct *next;
  volatile struct program_struct *prev;
}programT;

programT* init_program_list();
programT* add_program(programT *head, char *new_name, int new_core, int new_id, varT *locals, labelsT *labels, int fd);
programT* search_program_id(programT *head, int id, int print_flag); //psaxnei me to id

void destroy_programs(programT *head, int print_flag);
int remove_program(programT *current);

void print_programs(programT *head);
int find_less_busy_core(programT *head, int nofcores);
void rebalance_cores(programT *head, int nofcores);

#endif
