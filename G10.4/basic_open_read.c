#include "var_storage.h"

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

#define ALLOW_N_LINE_CHAR 0
#define BLOCK_N_LINE_CHAR 1
#define SEARCH_FOR_N_LINE 2

#define NODIGITS(x) x>0?((int)floor(log10((double)abs(x))) + 1):1

/*
 * returns number of group of the command if it's a command
 * returns 0 if not command
 */
int is_command(char command[]){
  //balta se kathgories gia na ta diaxeiristoume kalutera meta
  if(strcmp(command, "LOAD") == 0){
    return 1;
  }
  else if(strcmp(command, "STORE") == 0){
    return 2;
  }
  else if(strcmp(command, "SET") == 0){
    return 3;
  }
  else if(strcmp(command, "ADD") == 0 || \
          strcmp(command, "SUB") == 0 || \
          strcmp(command, "MUL") == 0 || \
          strcmp(command, "DIV") == 0 || \
          strcmp(command, "MOD") == 0 ){
            return 4;
  }
  else if(strcmp(command, "BRGT") == 0 || \
          strcmp(command, "BRGE") == 0 || \
          strcmp(command, "BRLT") == 0 || \
          strcmp(command, "BRLE") == 0 || \
          strcmp(command, "BREQ") == 0 ){
            return 5;
  }
  else if(strcmp(command, "BRA") == 0){
    return 6;
  }
  else if(strcmp(command, "DOWN") == 0 || \
          strcmp(command, "UP") == 0){
            return 7;
  }
  else if(strcmp(command, "SLEEP") == 0){
    return 8;
  }
  else if(strcmp(command, "PRINT") == 0){
    return 9;
  }
  else if(strcmp(command, "RETURN") == 0){
    return 10;
  }
  else{
    return 0;
  }
  return 1;
}

int my_read(int fd, char *buffer, int line){
  int byte_read;

  byte_read = read(fd, buffer, 1);
  if(byte_read == -1){
    fprintf(stderr, "line %d, ", line);
    perror("read");
    exit(1);
  }
  return byte_read;
}

//parameters: w_nline: to check for either ' ' or a '\n'
void discard_spaces(int fd, char input_buffer[], int w_nline){
  ssize_t byte_read;

  do{
    byte_read = my_read(fd, input_buffer, __LINE__);
    if(byte_read == 0){
      fprintf(stderr, "EOF terminating.\n");
      exit(1);
    }
    if ((w_nline == BLOCK_N_LINE_CHAR || w_nline == SEARCH_FOR_N_LINE) && input_buffer[0] == '\n') {
      break;
    }
  //  printf("input_buffer[0] = %c\n", input_buffer[0]);
  }while(input_buffer[0] == ' ' || input_buffer[0] == '\n');

  //printf("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");

  if(w_nline == BLOCK_N_LINE_CHAR){
    if(input_buffer[0] == '\n'){
      fprintf(stderr, "Unexpected end of line after label. Terminating\n");
      exit(1);
    }
  }
}

int read_island(int fd, char input_buffer[]){
  ssize_t byte_read;
  int i = 0;

  do{
    i++;
    byte_read = my_read(fd, input_buffer + i, __LINE__);
    if(byte_read == 0){
      fprintf(stderr, "Syntax error: Hasn't reached eoline and nothing to read. Terminating.\n");
      exit(1);
    }
    // printf("input_buffer[i = %d] = %c\n", i, input_buffer[i]);
  }while(input_buffer[i] != ' ' && input_buffer[i] != '\n');

  return i;
}

int check_varval(int fd, localVar *locals, char input_buffer[], char *temp_char){
  int i, j, printVal, printVar = 0;
  char* pos, *pos_temp;
  int ext_array_pos;

  if(isdigit(input_buffer[0])){
    i = read_island(fd, input_buffer);

    *temp_char = input_buffer[i];
    input_buffer[i] = '\0';
    j = 1;
    while(input_buffer[j] != '\0'){
      if(isdigit(input_buffer[j]) == 0){
        fprintf(stderr, "Syntax error: expected a value\n");
        exit(1);
      }
      j++;
    }
    printVal = atoi(input_buffer);

    printf(ANSI_COLOR_BLUE"%d "ANSI_COLOR_RESET, printVal);
    return printVal;
  }
  else if(input_buffer[0] == '$'){
    i = read_island(fd, input_buffer);

    *temp_char = input_buffer[i];
    input_buffer[i] = '\0';

    fprintf(stderr,"var given: %s\n", input_buffer);

    //check that first char is a letter
    if(isalpha(input_buffer[1]) == 0){
      fprintf(stderr, "Syntax error. Not right name of variable\n");
      exit(1);
    }

    pos_temp = strchr(input_buffer, '[');
    //check if variable
    if(pos_temp == NULL){
      printf("%s: simple var\n", input_buffer);
      printVar = read_node(locals, input_buffer, PRINT_REPORT);
    }
    else{
      pos = input_buffer + 1;
      pos_temp = strchr(pos, '$');
      // check if array
      if(pos_temp == NULL){
        printf("%s: simple array\n", input_buffer);
        printVar = read_node(locals, input_buffer, PRINT_REPORT);
      }
      else{
        pos = pos_temp + 1;
        input_buffer[strlen(input_buffer) - 1] = '\0';
        printf("if %s = 8\n", pos - 1);

        ext_array_pos = read_node(locals, pos - 1, PRINT_REPORT); // read value; !CREATE_PERMISSION
        pos = (char *)malloc(sizeof(char) * NODIGITS(ext_array_pos) + 1);
        sprintf(pos, "%d", ext_array_pos);

        strcpy(pos_temp, pos);
        strcat(pos_temp, "]");

        printf("double array became: %s\n", input_buffer);
        free(pos);
        printVar = read_node(locals, input_buffer, PRINT_REPORT);

      }
    }
    printf(ANSI_COLOR_BLUE"(string)%s (int)%d "ANSI_COLOR_RESET,input_buffer, printVar);
    return printVar;
  }
  else{
    fprintf(stderr, "Syntax error: Not a varval\n");
    exit(1);
  }
}

void check_var(int fd, localVar *locals, char input_buffer[], char *temp_char /*, int operation_result*/){
  int i;
  char* pos, *pos_temp;

  if(input_buffer[0] == '$'){
    i = read_island(fd, input_buffer);

    *temp_char = input_buffer[i];
    input_buffer[i] = '\0';

    fprintf(stderr,"var given: %s\n", input_buffer);

    //check that first char is a letter
    if(isalpha(input_buffer[1]) == 0){
      fprintf(stderr, "Syntax error. Not right name of variable\n");
      exit(1);
    }

    pos_temp = strchr(input_buffer, '[');
    //check if variable
    if(pos_temp == NULL){
      printf("%s: simple var\n", input_buffer);
      if (modify_node(locals, input_buffer, 0/*operation_result*/ ,PRINT_REPORT)){
        fprintf(stderr, "Error with modify_node (should never appear)\n");
        exit(1);
      }
    }
    else{
      pos = input_buffer + 1;
      pos_temp = strchr(pos, '$');
      // check if array
      if(pos_temp == NULL){
        printf("%s: simple array\n", input_buffer);
        if (modify_node(locals, input_buffer, 0/*operation_result*/ ,PRINT_REPORT)){
          fprintf(stderr, "Error with modify_node (should never appear)\n");
          exit(1);
        }
      }
      // else{
      //   pos = pos_temp + 1;
      //   input_buffer[strlen(input_buffer) - 1] = '\0';
      //   printf("if $%s = 8\n", pos);
      //   ext_array_pos = 8; // read value; !CREATE_PERMISSION
      //   printVar = read_node(locals, input_buffer + 1, PRINT_REPORT);
      //   pos = (char *)malloc(sizeof(char) * NODIGITS(ext_array_pos) + 1);
      //   sprintf(pos, "%d", ext_array_pos);
      //
      //   strcpy(pos_temp, pos);
      //   strcat(pos_temp, "]");
      //
      //   printf("double array became: %s\n", input_buffer);
      //   free(pos);
      //   if (modify_node(locals, input_buffer + 1, 0/*operation_result*/ ,PRINT_REPORT)){
      //     fprintf(stderr, "Error with modify_node (should never appear)\n");
      //     exit(1);
      //   }
      // }
    }
    // printf(ANSI_COLOR_BLUE"(string)%s (int)%d "ANSI_COLOR_RESET,input_buffer, printVar);
  }
}

/******************************************************************************/

int main(int argc,char *argv[]){
  int fd, i, labelGiven, command_group;
  char input_buffer[(LABEL_SIZE > COMMAND_SIZE ? LABEL_SIZE : COMMAND_SIZE)] = "";
  char temp_char, label[LABEL_SIZE] ="", command[COMMAND_SIZE]="";
  char printString[LABEL_SIZE];

  localVar *locals;

  locals = init_list();

  add_node(locals, locals->prev, "$temp", 2);
  print_contents(locals);
  // add_node(locals, locals->prev, "$array[2]", 7);
  modify_node(locals, "$array[2]", 7, !PRINT_REPORT);
  print_contents(locals);
  printf("\n\nPress enter to continue: "); getchar();

  // pare ta args
  if(argc < 2){
    fprintf(stderr, "Wrong number of arguments\n");
    exit(1);
  }

  fd = open(argv[1], O_RDWR, S_IRWXU);
	if(fd == -1){
		perror("open");
		exit(1);
  }

  // edw prepei na tsekarei to PROGRAM tag

  while(1){
    labelGiven = 0;

    /********************* 1o island **********************/
    i = 0;
    discard_spaces(fd, input_buffer, ALLOW_N_LINE_CHAR);
    printf("###################################################\n");
    i = read_island(fd, input_buffer);

    temp_char = input_buffer[i];

    input_buffer[i] = '\0';
    command_group = is_command(input_buffer);
    if(command_group){
      printf("It's a command!\n");
      strcpy(command, input_buffer); //dynamika??? to command
    }
    else if(input_buffer[0] == 'L'){
      printf("It's a label!\n");
      strcpy(label, input_buffer); //dynamika??? to label
      labelGiven = 1;
    }else{
      fprintf(stderr, "Syntax error. Neither a label nor a command\n");
      exit(1);
    }

    /*************** IF LABEL IS GIVEN ***************/
    if(labelGiven == 1){
      if(temp_char == '\n'){
        fprintf(stderr, "Unexpected end of line after label. Terminating\n");
        exit(1);
      }

      i = 0;
      discard_spaces(fd, input_buffer, BLOCK_N_LINE_CHAR);
      i = read_island(fd, input_buffer);

      temp_char = input_buffer[i];

      input_buffer[i] = '\0';
      command_group = is_command(input_buffer);
      if(command_group){
        printf("It's a command!\n");
        strcpy(command, input_buffer); //dynamika??? to command
      }
      else{
        fprintf(stderr, "Syntax error. No command after label\n");
        exit(1);
      }
    }

    // edw einai pou ginetai o elegxos gia tis metavlhtes
    // mia megalh switch wste na parei ta swsta

    // se auth th fash exei diavasei ton epomeno xarakthra apo to command
    // prepei na kanei discard_spaces

    if(command_group == 10){ // RETURN
      printf("I know it's return, now let's check it\n");
      if(temp_char == ' '){
        discard_spaces(fd, input_buffer, SEARCH_FOR_N_LINE);
      }
      else{
        input_buffer[0] = temp_char;
      }

      if(input_buffer[0] == '\n'){
        //ola mia xara. einai to RETURN
        printf("RETURN command spotted!\n");
      }
      else{
        fprintf(stderr, "Syntax error: Nothing expected after return\n");
        exit(1);
      }

      if(labelGiven){
        printf(ANSI_COLOR_BLUE"%s "ANSI_COLOR_RESET, label);
      }
      printf(ANSI_COLOR_BLUE"%s"ANSI_COLOR_RESET"\n", command);
    }
    else if(command_group == 9){ // PRINT
      /***************************** STRING ***********************************/
      if(temp_char == ' '){
        discard_spaces(fd, input_buffer, BLOCK_N_LINE_CHAR);
      }
      else if(temp_char == '\n') {
        fprintf(stderr, "Syntax error: Expected string but found new line.\n");
        exit(1);
      }
      else{
        printf("sth terribly wrong\n");
        exit(1);
      }

      printf("input_buffer[0] = '%c'\n", input_buffer[0]);
      if(input_buffer[0] != '"'){
        fprintf(stderr, "Syntax error: Expected quotes for string.\n");
        exit(1);
      }

      // i = read_island(fd, input_buffer);
      i = 0;
      char byte_read;
      do{
        i++;
        byte_read = my_read(fd, input_buffer + i, __LINE__);
        if(byte_read == 0){
          fprintf(stderr, "Syntax error: Hasn't reached eoline and nothing to read. Terminating.\n");
          exit(1);
        }
      }while(input_buffer[i] != '"');

      temp_char = input_buffer[i];
      input_buffer[i] = '\0';
      if(temp_char != '"'){
        fprintf(stderr, "Syntax error: Expected ending quotes for string.\n");
        exit(1);
      }

      strcpy(printString, input_buffer + 1);
      printf("printString = %s\n", printString);
      // printf("input_buffer = %s\n", input_buffer + 1);

      byte_read = my_read(fd, &temp_char, __LINE__);
      if(byte_read == 0){
        fprintf(stderr, "Syntax error: Hasn't reached eoline and nothing to read. Terminating.\n");
        exit(1);
      }

      /***************************** VARVAL ***********************************/
      if(labelGiven){
        printf(ANSI_COLOR_BLUE"%s "ANSI_COLOR_RESET, label);
      }
      printf(ANSI_COLOR_BLUE"%s %s "ANSI_COLOR_RESET, command, printString);

      // printf("input_buffer = %s\n");

      while (temp_char != '\n'){
        if(temp_char == ' '){
          discard_spaces(fd, input_buffer, SEARCH_FOR_N_LINE);
          if(input_buffer[0] == '\n'){
            printf("\n");break;
          }
        }
        else{
          fprintf(stderr, "sth terribly wrong\n");
          exit(1);
        }

        fprintf(stderr, "input_buffer[0] = %c\n", input_buffer[0]);
        /************************* IN A FUNCTION ******************************/
        check_varval(fd, locals, input_buffer, &temp_char);

        /************************ </IN A FUNCTION> ****************************/

      }
      printf("\n");
      continue;
    }
    else if(command_group == 1){ // LOAD
      //Perimenei Var kai GlobalVar
    }
    else if(command_group == 2){ // STORE
      //Perimenei GlobalVar kai Var
    }
    else if(command_group == 3){ // SET
      //Perimenei Var kai VarVal
    }
    else if(command_group == 4){ // ADD, SUB, MUL, DIV, MOD
      //Perimenei Var, VarVal kai VarVal
    }
    else if(command_group == 7){ // DOWN, UP
      //Perimenei GlobalVar
    }
    else if(command_group == 8){ // SLEEP
      //Perimenei VarVal
    }
    else if(command_group == 5){ // BRGT, BRGE, BRLT, BRLE, BREQ
      //Perimenei VarVal, VarVal kai Label
    }
    else if(command_group == 6){ // BRA
      //Perimenei Label
    }
    else{
      fprintf(stderr, "EDW DEN PROKEITAI POTE NA FTASEI\n");
    }

  }

  //
  // if(i >= COMMAND_SIZE || /*check if NOT a command function*/){
  //   // it's a label
  //
  //   // do the same shit once more for the command
  //   //read
  //   // check if it's a command. If not: syntax error
  // }
  //
  // //it's a command for sure

  //den ftanei edw pote alla tha mpei se mia "terminating function"
  close(fd);

  return 0;
}
