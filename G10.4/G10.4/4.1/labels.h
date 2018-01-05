#ifndef __LABELS_H__
#define __LABELS_H__

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define INIT_OFFSET -2
#define INVALID_OFFSET -1
#define PRINT_REPORT 1

typedef struct label_struct{
  char *name;
  off_t offset;
  struct label_struct *prev;
  struct label_struct *next;
}labelsT;

labelsT* init_labels();
void add_label(labelsT *head, char *new_name, off_t new_offset);
off_t search_label(labelsT *head, char name[], int print_flag);
void destroy_labels(labelsT *head, int print_flag);
void print_labels(labelsT *head);

#endif
