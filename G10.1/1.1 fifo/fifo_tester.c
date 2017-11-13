/* Axelou Olympia, 2161
 * Tsitsopoulou Eirini, 2203
 *
 * This is the tester program for fifo
 * Creates two threads, assigns jobs to each one, waits for them to end
 *
 * Inputs: pathnames of input and output files
 */

#include "fifo.h"
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#define EXIT_FAILURE 1
#define FAILURE -1

#define SIZE 10

volatile int write_join, read_join;

/* opens an existing text file and returns its fd
 * in case the pathname cannot be found, the program exits
 */
int my_open(const char *pathname,int flags,mode_t mode,int line){
	int fd;

	fd = open(pathname,flags,mode);

	if(fd == FAILURE ){
		printf("open error on line %d: %s\n", line, strerror(errno));
		exit(EXIT_FAILURE);
	}

    return(fd);
}

/* opens a text file and returns its fd
 * if it already exists, the user can truncate it
 * if he chooses not to, the program exits
 */
int my_open_export(const char *pathname, int line){

	int fd_file;
	char choice;

	fd_file = open(pathname,O_RDWR | O_CREAT | O_EXCL, S_IRWXU);

	if(fd_file == FAILURE ){

		if(errno == EEXIST){

			printf("File already exists\n");
			printf("Do you want to truncate it? (y/n) : ");
			scanf(" %c", &choice);

			while(choice != 'n' && choice != 'y'){
				printf ("Invalid entry.\n");

				printf("File already exists\n");
				printf("Do you want to truncate it? (y/n)");
				scanf(" %c", &choice);
			}

			if(choice == 'n')
				exit(EXIT_FAILURE);

			//o xristis epelekse truncate
			fd_file = open(pathname, O_RDWR | O_TRUNC , S_IRWXU);
			return (fd_file);
		}
		else{
			printf("open error on line %d: %s\n", line, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	return(fd_file);
}

/* returns 1 if the byte is successfully written
 * or terminates the program in case of failure
 */
int my_write(int fd, char temp_char,int line){

	int bytes_written;

	bytes_written = (int) write(fd, &temp_char, (size_t) 1);

	if(bytes_written == FAILURE){
		printf("write error on line %d: %s\n", line, strerror(errno));
		close(fd);
		exit(EXIT_FAILURE);
	}
	else
		return(bytes_written);
}

/* returns the bytes read in case of successful system call
 * or terminates the program in case of failure
 */
int my_read(int fd, char *temp_char, int line){

	int bytes_read;

	bytes_read = (int) read(fd, temp_char, (size_t) 1);

	if(bytes_read == FAILURE){
		printf("read error on line %d: %s\n", line, strerror(errno));
		close(fd);
		exit(EXIT_FAILURE);
	}
	else{
		if(!bytes_read){
			return(0);
		}
		else
			return(1);
	}
}


/* function executed by the write&close thread
 * reads from file (until EOF) and writes in pipe
 * byte-by-byte, and then closes the pipe
 *
 * input: fd of the file from which it reads
 */
void *write_close_func(void *arg){
  char c;
  int fd_input;

  fd_input = *(int*) arg;

  while(my_read(fd_input, &c, __LINE__) == 1){
    pipe_write(c);
	}

  pipe_close();

	write_join = 0;
  return NULL;
}


/* function executed by the read thread
 * reads from pipe and writes in file byte-by-byte
 *
 * input: fd of the file in which it writes
 */
void *read_func(void *arg){
  char c;
  int fd_output;

  fd_output = *(int*) arg;

	while (1) {
    if(!pipe_read(&c)){
      break;
    }
    my_write(fd_output, c, __LINE__);
  }

	read_join = 0;
  return NULL;
}

int main(int argc,char *argv[]){

  int size = SIZE;
  int fd_input, fd_output, check_value;
  pthread_t t_write, t_read;

  // checks the arguments
  if(argc != 3){
    printf("Wrong number of arguments\n");
    return 1;
  }
  fd_input = my_open(argv[1],O_RDONLY ,S_IRWXU, __LINE__);
  fd_output = my_open_export(argv[2], __LINE__);

  pipe_init(size);

  // creates write&close thread
  write_join = 1;
  check_value = pthread_create(&t_write, NULL, write_close_func, &fd_input);
  if (check_value){
    printf("error at write\n");
  }

	// creates read thread
  read_join = 1;
  check_value = pthread_create(&t_read, NULL, read_func, &fd_output);
  if (check_value){
    printf("error at read\n");
  }

  // waits for both threads to terminate
  while(write_join || read_join){}

  return 0;
}
