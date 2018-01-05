#ifndef __PROGRAM_HANDLER1_H__
#define __PROGRAM_HANDLER1_H__

#include "var_storage.h"
#include "labels.h"

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
  int fd, running;
  varT *locals;
  labelsT *labels;
}programT;

#endif
