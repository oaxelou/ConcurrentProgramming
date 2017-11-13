#ifndef __FIFO_H_
#define __FIFO_H_

#include "fifo.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//flags
volatile int writing, reading, closing;
volatile int full, empty;

//circular buffer pointers
volatile int read_index, write_index;

//buffer (pipe) pointer & size
volatile char *buffer;
volatile int size_vol;

void pipe_init(int size);
void pipe_write(char c);
int pipe_read(char *c);
void pipe_close();

#endif
