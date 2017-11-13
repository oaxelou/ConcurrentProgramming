#include "fifo.h"

// pipe initialization function
void pipe_init(int size){

  buffer = (char *)malloc(sizeof(char) * size);

  size_vol = size;
  empty = 1;
  full = 0;
  reading = writing = closing = 0;
  write_index = read_index = 0;
}

void pipe_write(char c){

  //in write waiting either for the pipe not to be full
  //or for the other thread to not be reading
  while(reading || full){
  }

  writing = 1;
  buffer[write_index] = c;
  write_index = (write_index + 1) % size_vol;

  if(empty)
    empty = 0;

  if(read_index == write_index){ //the pipe just got full
    full = 1;
  }

  writing = 0;
}

int pipe_read(char *c){

  // in read waiting for the pipe to
  // not be empty, or for the other thread not to be writing
  // or for the closing flag to be 1
  while (!closing &&(writing || empty)) {
  }

  if(closing && empty){
    free((int*)buffer);
	  return 0;
  }

  reading = 1;
  *c = buffer[read_index];
  read_index = (read_index + 1) % size_vol;

  if(full)
    full = 0;

  if(read_index == write_index){ //the pipe just got empty
    empty = 1;
  }

  reading = 0;

  return 1;
}

void pipe_close(){
  closing = 1;
}
