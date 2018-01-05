#ifndef __PARSER_H__
#define __PARSER_H__

#include "var_storage.h"
#include "labels.h"
#include "program_handler1.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>
#include <time.h>

#define COMMAND_SIZE 7
#define STANDARD_SIZE 200
#define PROGRAM_TAG "PROGRAM"

#define ALLOW_N_LINE_CHAR 0
#define BLOCK_N_LINE_CHAR 1
#define SEARCH_FOR_N_LINE 2

#define NODIGITS(x) x>0?((int)floor(log10((double)abs(x))) + 1):1

varT volatile *globals;

/******************************************************************************/

int is_command(char command[]);
int my_read(int fd, char *buffer, int line);
void discard_spaces(int fd, char input_buffer[], int w_nline);
int read_island(int fd, char input_buffer[]);
int check_varval(int fd, varT *locals, char input_buffer[], char *temp_char, char command[]);
void check_var(int fd, varT *locals, char input_buffer[], char *temp_char, char command[]);
int check_varGlobal(int fd, varT *globals, varT *locals, char input_buffer[], char *temp_char, char command[]);
void search_label_downwards(int fd, char temp_char, char label_to_find[], labelsT *labels);
void check_inner_space(int fd, char *temp_char, char error_string[], char command[]);
void read_label(int fd, char *temp_char, char buffer[], char command[]);
int read_varval(int fd, varT *locals,char *temp_char, char command[]);
void read_var(int fd, varT *locals, char *temp_char, char var_op[], char command[]);
void print_command(int fd, varT *locals, char *temp_char);
void load_command(int fd, varT *locals, varT *globals, char *temp_char);
int read_label_command(int fd, char *temp_char, labelsT* labels, char label[], char command[], int *command_group);

int main_program(programT *program);

#endif
