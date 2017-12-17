#ifndef __PROGRAM_HANDLER_H__
#define __PROGRAM_HANDLER_H__

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define INIT_PID -1

#define NOT_RUNNING 0
#define RUNNING 1

#define PRINT_REPORT 1

typedef volatile struct program_struct{
  char *name;
  pthread_t pid; // se poio core anhkei
  int id;
  int running;

  volatile struct program_struct *prev;
  volatile struct program_struct *next;
}programT;

programT* init_program_list();
void add_program(programT *head, char *new_name, pthread_t new_pid, int new_id);
programT* search_program_id(programT *head, int id, int print_flag); //psaxnei me to onoma

void kill_all(programT *head, int print_flag);
int kill_program(programT *head, int id, int print_flag);

void print_programs(programT *head);


#endif
