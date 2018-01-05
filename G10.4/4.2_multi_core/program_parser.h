#ifndef __PROGRAM_PARSER_H__
#define __PROGRAM_PARSER_H__

#include "var_storage1.h"
#include "labels_commands_list.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <math.h>

#define COMMAND_SIZE 7
#define LABEL_SIZE 100
#define PROGRAM_TAG "PROGRAM"

#define ALLOW_N_LINE_CHAR 0
#define BLOCK_N_LINE_CHAR 1
#define SEARCH_FOR_N_LINE 2

#define NODIGITS(x) x>0?((int)floor(log10((double)abs(x))) + 1):1

/******************************************************************************/

int is_command(char command[]);
int my_read(int fd, char *buffer, int line);
void discard_spaces(int fd, char input_buffer[], int w_nline);
int read_island(int fd, char input_buffer[]);
int check_varval(int fd, local_varT *locals, char input_buffer[], char *temp_char, char command[]);
void check_var(int fd, local_varT *locals, char input_buffer[], char *temp_char, char command[]);
// exactly the same as check_var but for globals
void check_Globalvar(int fd, global_varT *globals, char input_buffer[], char *temp_char, char command[]);
int check_varGlobal(int fd, global_varT *globals, char input_buffer[], char *temp_char, char command[]);
void search_label_downwards(int fd, char temp_char, char label_to_find[]);
void check_inner_space(int fd, char *temp_char, char error_string[], char command[]);
void read_label(int fd, char *temp_char, char buffer[], char command[]);
int read_varval(int fd, local_varT *locals,char *temp_char, char command[]);
void read_var(int fd, local_varT *locals, char *temp_char, char var_op[], char command[]);
void print_command(int fd, local_varT *locals, char *temp_char);
void load_command(int fd, local_varT *locals, global_varT *globals, char *temp_char);
int read_label_command(int fd, char *temp_char, labelsT* labels, char label[], char command[], int *command_group);

int main_program(int id, int argc,int *argv, char *filename);

#endif