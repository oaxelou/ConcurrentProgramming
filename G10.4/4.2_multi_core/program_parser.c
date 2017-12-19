#include "program_parser.h"
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

//parameters: w_nline: to check for either ' ' (and '\t')or a '\n'
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
  }while(input_buffer[0] == ' ' || input_buffer[0] == '\t' || input_buffer[0] == '\n');

  //printf("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");

  if(w_nline == BLOCK_N_LINE_CHAR){
    if(input_buffer[0] == '\n'){
      fprintf(stderr, "Unexpected end of line inside instruction. Terminating\n");
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
  }while(input_buffer[i] != ' ' && input_buffer[i] != '\t' && input_buffer[i] != '\n');

  return i;
}

int check_varval(int fd, local_varT *locals, char input_buffer[], char *temp_char, char command[]){
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
        fprintf(stderr, "%s: Syntax error: expected a value\n", command);
        exit(1);
      }
      j++;
    }
    printVal = atoi(input_buffer);

    // printf(ANSI_COLOR_BLUE"%d "ANSI_COLOR_RESET, printVal);
    return printVal;
  }
  else if(input_buffer[0] == '$'){
    i = read_island(fd, input_buffer);

    *temp_char = input_buffer[i];
    input_buffer[i] = '\0';

    fprintf(stderr,"var given: %s\n", input_buffer);

    //check that first char is a letter
    if(isalpha(input_buffer[1]) == 0){
      fprintf(stderr, "%s: Syntax error. Not right name of variable\n", command);
      exit(1);
    }

    pos_temp = strchr(input_buffer, '[');
    //check if variable
    if(pos_temp == NULL){
      printf("%s: simple var\n", input_buffer);
      printVar = read_local_node(locals, input_buffer, PRINT_REPORT);
    }
    else{
      pos = input_buffer + 1;
      pos_temp = strchr(pos, '$');
      // check if array
      if(pos_temp == NULL){
        printf("%s: simple array\n", input_buffer);
        printVar = read_local_node(locals, input_buffer, PRINT_REPORT);
      }
      else{
        pos = pos_temp + 1;
        input_buffer[strlen(input_buffer) - 1] = '\0';
        // printf("if %s = 8\n", pos - 1);

        ext_array_pos = read_local_node(locals, pos - 1, PRINT_REPORT); // read value; !CREATE_PERMISSION
        pos = (char *)malloc(sizeof(char) * NODIGITS(ext_array_pos) + 1);
        sprintf(pos, "%d", ext_array_pos);

        strcpy(pos_temp, pos);
        strcat(pos_temp, "]");

        printf("double array became: %s\n", input_buffer);
        free(pos);
        printVar = read_local_node(locals, input_buffer, PRINT_REPORT);

      }
    }
    printf(ANSI_COLOR_BLUE"(string)%s (int)%d\n"ANSI_COLOR_RESET,input_buffer, printVar);
    return printVar;
  }
  else{
    fprintf(stderr, "%s: Syntax error: Not a varval\n", command);
    exit(1);
  }
}

void check_var(int fd, local_varT *locals, char input_buffer[], char *temp_char, char command[]){
  int i, ext_array_pos;
  char* pos, *pos_temp;

  if(input_buffer[0] == '$'){
    i = read_island(fd, input_buffer);

    *temp_char = input_buffer[i];
    input_buffer[i] = '\0';

    fprintf(stderr,"var given: %s\n", input_buffer);

    // printf("===%s\n", input_buffer);
    //check that first char is a letter
    if(isalpha(input_buffer[1]) == 0){
      // printf("===%s\n", input_buffer+1);
      fprintf(stderr, "%s: Syntax error. Not right name of variable\n", command);
      exit(1);
    }

    pos_temp = strchr(input_buffer, '[');
    //check if variable
    if(pos_temp == NULL){
      printf("%s: simple var\n", input_buffer);
      // EDW HTAN TO modify_node
    }
    else{
      pos = input_buffer + 1;
      pos_temp = strchr(pos, '$');
      // check if array
      if(pos_temp == NULL){
        printf("%s: simple array\n", input_buffer);
        // EDW HTAN TO modify_node
      }
      else{
        pos = pos_temp + 1;
        input_buffer[strlen(input_buffer) - 1] = '\0';

        ext_array_pos = read_local_node(locals, pos - 1, PRINT_REPORT);
        pos = (char *)malloc(sizeof(char) * NODIGITS(ext_array_pos) + 1);
        sprintf(pos, "%d", ext_array_pos);

        strcpy(pos_temp, pos);
        strcat(pos_temp, "]");

        printf("double array became: %s\n", input_buffer);
        free(pos);
      }
    }
  }
  else{
    fprintf(stderr, "%s: Syntax error: expected $\n", command);
    exit(1);
  }
}

// exactly the same as check_var but for globals
void check_Globalvar(int fd, global_varT *globals, char input_buffer[], char *temp_char, char command[]){
  int i, ext_array_pos;
  char* pos, *pos_temp;

  if(input_buffer[0] == '$'){
    i = read_island(fd, input_buffer);

    *temp_char = input_buffer[i];
    input_buffer[i] = '\0';

    fprintf(stderr,"var given: %s\n", input_buffer);

    // printf("===%s\n", input_buffer);
    //check that first char is a letter
    if(isalpha(input_buffer[1]) == 0){
      // printf("===%s\n", input_buffer+1);
      fprintf(stderr, "%s: Syntax error. Not right name of variable\n", command);
      exit(1);
    }

    pos_temp = strchr(input_buffer, '[');
    //check if variable
    if(pos_temp == NULL){
      printf("%s: simple var\n", input_buffer);
      // EDW HTAN TO modify_node
    }
    else{
      pos = input_buffer + 1;
      pos_temp = strchr(pos, '$');
      // check if array
      if(pos_temp == NULL){
        printf("%s: simple array\n", input_buffer);
        // EDW HTAN TO modify_node
      }
      else{
        pos = pos_temp + 1;
        input_buffer[strlen(input_buffer) - 1] = '\0';

        ext_array_pos = read_global_node(globals, pos - 1, PRINT_REPORT);
        pos = (char *)malloc(sizeof(char) * NODIGITS(ext_array_pos) + 1);
        sprintf(pos, "%d", ext_array_pos);

        strcpy(pos_temp, pos);
        strcat(pos_temp, "]");

        printf("double array became: %s\n", input_buffer);
        free(pos);
      }
    }
  }
  else{
    fprintf(stderr, "%s: Syntax error: expected $\n", command);
    exit(1);
  }
}

int check_varGlobal(int fd, global_varT *globals, char input_buffer[], char *temp_char, char command[]){
  int i, ext_array_pos;
  char* pos, *pos_temp;

  if(input_buffer[0] == '$'){
    i = read_island(fd, input_buffer);

    *temp_char = input_buffer[i];
    input_buffer[i] = '\0';

    fprintf(stderr,"var given: %s\n", input_buffer);

    // printf("===%s\n", input_buffer);
    //check that first char is a letter
    if(isalpha(input_buffer[1]) == 0){
      // printf("===%s\n", input_buffer+1);
      fprintf(stderr, "%s: Syntax error. Not right name of variable\n", command);
      exit(1);
    }

    pos_temp = strchr(input_buffer, '[');
    //check if variable
    if(pos_temp == NULL){
      printf("%s: simple var\n", input_buffer);
      return read_global_node(globals, input_buffer, PRINT_REPORT);
    }
    else{
      pos = input_buffer + 1;
      pos_temp = strchr(pos, '$');
      // check if array
      if(pos_temp == NULL){
        printf("%s: simple array\n", input_buffer);
        return read_global_node(globals, input_buffer, PRINT_REPORT);
      }
      else{
        pos = pos_temp + 1;
        input_buffer[strlen(input_buffer) - 1] = '\0';
        // printf("if $%s = 8\n", pos);

        ext_array_pos = read_global_node(globals, pos - 1, PRINT_REPORT);
        pos = (char *)malloc(sizeof(char) * NODIGITS(ext_array_pos) + 1);
        sprintf(pos, "%d", ext_array_pos);

        strcpy(pos_temp, pos);
        strcat(pos_temp, "]");

        printf("Global: array became: %s\n", input_buffer);
        free(pos);
        return read_global_node(globals, input_buffer, PRINT_REPORT);
      }
    }
    // printf(ANSI_COLOR_BLUE"(string)%s (int)%d "ANSI_COLOR_RESET,input_buffer, printVar);
  }
  else{
    fprintf(stderr, "%s: Syntax error: expected $\n", command);
    exit(1);
  }
}

void search_label_downwards(int fd, char temp_char, char label_to_find[]){
  int i, bytes_read;
  char input_buffer[LABEL_SIZE];

  while (1) {
    if(temp_char == '\n'){
      if (lseek(fd, -1, SEEK_CUR) == -1){
        fprintf(stderr, "Error with lseek in search_label_downwards\n");
        exit(1);
      }
    }
    bytes_read = my_read(fd, input_buffer, __LINE__);
    if (bytes_read == 0){
      fprintf(stderr, "Error: label %s not found\n", label_to_find);
      exit(1);
    }
    else if(input_buffer[0] == '\n'){
      discard_spaces(fd, input_buffer, ALLOW_N_LINE_CHAR);
      i = read_island(fd, input_buffer);
      temp_char = input_buffer[i];
      input_buffer[i] = '\0';

      if (strcmp(label_to_find, input_buffer)){
        continue; //go to next line
      }
      //label_to_find has been found
      //main will jump to label_to_find and read label from file
      if (lseek(fd, -strlen(label_to_find)-1, SEEK_CUR) == -1){
        fprintf(stderr, "Error with lseek in search_label_downwards\n");
        exit(1);
      }
      break;
    }
  }
}

//*******************************************************************************************

// KATW APO KATHE KLHSH PREPEI NA UPARXEI H ANTHETHESH input_buffer[0] = temp_char;
void check_inner_space(int fd, char *temp_char, char error_string[], char command[]){
  if(*temp_char == ' ' || *temp_char == '\t'){
    discard_spaces(fd, temp_char, BLOCK_N_LINE_CHAR);
  }
  else if(*temp_char == '\n') {
    fprintf(stderr, "%s: Syntax error: Expected %s but found new line.\n", command, error_string);
    exit(1);
  }
  else{
    printf("sth terribly wrong\n");
    exit(1);
  }
}

void read_label(int fd, char *temp_char, char buffer[], char command[]){
  int i;

  check_inner_space(fd, temp_char, "label", command);
  buffer[0] = *temp_char;

  i = read_island(fd, buffer);

  *temp_char = buffer[i];

  buffer[i] = '\0';
  if(buffer[0] != 'L'){
    fprintf(stderr, "Syntax error: Not a label.\n");
    exit(1);
  }
}

int read_varval(int fd, local_varT *locals,char *temp_char, char command[]){
  int varval;char buffer[LABEL_SIZE];

  if(*temp_char == ' ' || *temp_char == '\t'){
    discard_spaces(fd, buffer, BLOCK_N_LINE_CHAR);
  }
  else if(*temp_char == '\n') {
    fprintf(stderr, "%s: Syntax error: Expected var/val but found new line.\n", command);
    exit(1);
  }
  else{
    fprintf(stderr, "sth terribly wrong\n");
    exit(1);
  }

  varval = check_varval(fd, locals, buffer, temp_char, command);
  return varval;
}

void read_var(int fd, local_varT *locals, char *temp_char, char var_op[], char command[]){
  char input_buffer[LABEL_SIZE];

  check_inner_space(fd, temp_char, "var", command);
  input_buffer[0] = *temp_char;

  var_op[0] = input_buffer[0];
  check_var(fd, locals, var_op, temp_char, command);
}

/******************************************************************************/

void print_command(int fd, local_varT *locals, char *temp_char){
  char varval_input[LABEL_SIZE];
  ssize_t bytes_read;
  int varval1;

  if(*temp_char == ' ' || *temp_char == '\t'){
    discard_spaces(fd, temp_char, BLOCK_N_LINE_CHAR);
  }
  else if(*temp_char == '\n') {
    fprintf(stderr, "PRINT: Syntax error: Expected string but found new line.\n");
    exit(1);
  }
  else{
    printf("sth terribly wrong\n");
    exit(1);
  }

  if(*temp_char != '"'){
    fprintf(stderr, "PRINT: Syntax error: Expected quotes for string.\n");
    exit(1);
  }

  do{
    bytes_read = my_read(fd, temp_char, __LINE__);
    if(bytes_read == 0){
      fprintf(stderr, "Syntax error: Hasn't reached eoline and nothing to read. Terminating.\n");
      exit(1);
    }
    else if(*temp_char == '"'){
      break;
    }
    printf(ANSI_COLOR_BLUE"%c"ANSI_COLOR_RESET, *temp_char);
  }while(1);

  if(*temp_char != '"'){
    fprintf(stderr, "PRINT: Syntax error: Expected ending quotes for string.\n");
    exit(1);
  }

  bytes_read = my_read(fd, temp_char, __LINE__);
  if(bytes_read == 0){
    fprintf(stderr, "PRINT: Syntax error: Hasn't reached eoline and nothing to read. Terminating.\n");
    exit(1);
  }

  /***************************** VARVAL ***********************************/
  while (*temp_char != '\n'){
    if(*temp_char == ' ' || *temp_char == '\t'){
      discard_spaces(fd, varval_input, SEARCH_FOR_N_LINE);
      if(varval_input[0] == '\n'){
        printf("\n");break;
      }
    }
    else{
      fprintf(stderr, "sth terribly wrong\n");
      exit(1);
    }

    varval1 = check_varval(fd, locals, varval_input, temp_char, "PRINT");
    printf(ANSI_COLOR_BLUE"%d "ANSI_COLOR_RESET, varval1);
  }
  printf("\n");
}

void load_command(int fd, local_varT *locals, global_varT *globals, char *temp_char){
  char var_op[LABEL_SIZE], input_buffer[LABEL_SIZE];
  int varval1;

  //Perimenei Var kai GlobalVar
  if(*temp_char == ' ' || *temp_char == '\t'){
    discard_spaces(fd, input_buffer, BLOCK_N_LINE_CHAR);
  }
  else if(*temp_char == '\n') {
    fprintf(stderr, "LOAD: Syntax error: Expected var but found new line.\n");
    exit(1);
  }
  else{
    printf("sth terribly wrong\n");
    exit(1);
  }

  var_op[0] = input_buffer[0];
  check_var(fd, locals, var_op, temp_char, "LOAD");

  if(*temp_char == ' ' || *temp_char == '\t'){
    discard_spaces(fd, input_buffer, BLOCK_N_LINE_CHAR);
  }
  else if(*temp_char == '\n') {
    fprintf(stderr, "LOAD: Syntax error: Expected var but found new line.\n");
    exit(1);
  }
  else{
    fprintf(stderr, "sth terribly wrong\n");
    exit(1);
  }
  varval1 = check_varGlobal(fd, globals, input_buffer, temp_char, "LOAD");

  modify_local_node(locals, var_op, varval1, !PRINT_REPORT);
}

//RETURN VALUE: 1 if labelGiven and 0 elsewise
int read_label_command(int fd, char *temp_char, labelsT* labels, char label[], char command[], int *command_group){
  int i, labelGiven;
  char input_buffer[LABEL_SIZE];

  labelGiven = i = 0;
  discard_spaces(fd, input_buffer, ALLOW_N_LINE_CHAR);
  i = read_island(fd, input_buffer);

  *temp_char = input_buffer[i];
  input_buffer[i] = '\0';
  *command_group = is_command(input_buffer);
  if(*command_group){
    strcpy(command, input_buffer); //dynamika??? to command
  }
  else if(input_buffer[0] == 'L'){
    strcpy(label, input_buffer); //dynamika??? to label
    labelGiven = 1;
    add_label(labels, label, lseek(fd, 0, SEEK_CUR) - strlen(label) - 1); //current offset - strlen(label_name)

  }else{
    fprintf(stderr, "Syntax error. Neither a label nor a command\n");
    exit(1);
  }

  /*************** IF LABEL IS GIVEN ***************/
  if(labelGiven == 1){
    if(*temp_char == '\n'){
      fprintf(stderr, "Unexpected end of line after label. Terminating\n");
      exit(1);
    }

    i = 0;
    discard_spaces(fd, input_buffer, BLOCK_N_LINE_CHAR);
    i = read_island(fd, input_buffer);

    *temp_char = input_buffer[i];

    input_buffer[i] = '\0';
    *command_group = is_command(input_buffer);
    if(*command_group){
      printf("It's a command!\n");
      strcpy(command, input_buffer); //dynamika??? to command
    }
    else{
      fprintf(stderr, "Syntax error. No command after label\n");
      exit(1);
    }
  }

  return labelGiven;
}

/******************************************************************************/

int main(/*int id, */int argc,char *argv[]/*, char *filename*/){
  int fd, labelGiven, command_group, varValue, varval1, varval2;
  char input_buffer[LABEL_SIZE] = "";
  char var_op[LABEL_SIZE], varval1_op[LABEL_SIZE];
  char temp_char, label[LABEL_SIZE] ="", command[COMMAND_SIZE]="";
  off_t temp_offset;
  int i;

  local_varT *locals;
  global_varT *globals;
  labelsT *labels;

  locals = init_local_list();
  globals = init_global_list();
  labels = init_labels();


  add_local_node(locals, locals->prev, "$argc", argc - 1);
  add_local_node(locals, locals->prev, "$argv[0]", 0);
  // for(i = 0; i < argc; i++){
  //   sprintf(var_op , "$argv[%d]", i + 1);
  //   add_local_node(locals, locals->prev, var_op,atoi( argv[i]));
  // }
  sprintf(var_op, "$argv[%d]", 1);
  add_local_node(locals, locals->prev, var_op,atoi( argv[2]));

  print_local_contents(locals);

  fd = open(argv[1], O_RDWR, S_IRWXU);
	if(fd == -1){
		perror("open");
		exit(1);
  }

  /************************* PROGRAM TAG CHECK ********************************/
  my_read(fd, input_buffer, __LINE__);
  if (input_buffer[0] != '#'){
    fprintf(stderr, "Main: Program Tag does not start with '#'\n");
    exit(1);
  }

  discard_spaces(fd, input_buffer, ALLOW_N_LINE_CHAR);
  i = read_island(fd, input_buffer);
  temp_char = input_buffer[i];
  input_buffer[i] = '\0';

  if (strcmp(input_buffer, PROGRAM_TAG)!=0){
    fprintf(stderr, "Main: Incorrect Program Tag\n");
    exit(1);
  }
/******************************************************************************/

  while(1){
    labelGiven = read_label_command(fd, &temp_char, labels, label, command, &command_group);

    if(labelGiven){
      printf(ANSI_COLOR_BLUE"%s "ANSI_COLOR_RESET, label);
    }
    printf(ANSI_COLOR_BLUE"%s "ANSI_COLOR_RESET, command);

    if(command_group == 10){ // RETURN
      if(temp_char == ' ' || temp_char == '\t'){
        discard_spaces(fd, input_buffer, SEARCH_FOR_N_LINE);
      }
      else{
        input_buffer[0] = temp_char;
      }

      if(input_buffer[0] != '\n'){
        fprintf(stderr, "Main: Syntax error: Nothing expected after return\n");
        exit(1);
      }
      break;
    }
    else if(command_group == 9){ // PRINT
      print_command(fd, locals, &temp_char);
    }
    else if(command_group == 1){ // LOAD
      load_command(fd, locals, globals, &temp_char);
    }
    else if(command_group == 2){ // STORE
      //Perimenei GlobalVar kai Var

      check_inner_space(fd, &temp_char, "var", command);
      input_buffer[0] = temp_char;

      fprintf(stderr, "STORE: var = %c\n", input_buffer[0]);
      var_op[0] = input_buffer[0];
      check_Globalvar(fd, globals, var_op, &temp_char, command);

      /***************************** VARVAL ***********************************/
      varval1 = read_varval(fd, locals, &temp_char, command);

      modify_global_node(globals, var_op, varval1, !PRINT_REPORT);
    }
    else if(command_group == 3){ // SET
      //Perimenei Var kai VarVal

      read_var(fd, locals, &temp_char, var_op, command);

      /***************************** VARVAL ***********************************/
      varval1 = read_varval(fd, locals, &temp_char, command);

      modify_local_node(locals, var_op, varval1, !PRINT_REPORT);
    }
    else if(command_group == 4){ // ADD, SUB, MUL, DIV, MOD
      //Perimenei Var, VarVal kai VarVal

      /****************************** VAR *************************************/
      check_inner_space(fd, &temp_char, "var", command);
      input_buffer[0] = temp_char;

      var_op[0] = input_buffer[0];
      check_var(fd, locals, var_op, &temp_char, command);

      /**************************** VARVAL1 ***********************************/
      varval1 = read_varval(fd, locals, &temp_char, command);

      /**************************** VARVAL2 ***********************************/
      varval2 = read_varval(fd, locals, &temp_char, command);

      // ADD, SUB, MUL, DIV, MOD
      if(strcmp(command, "ADD") == 0){
        varValue = varval1 + varval2;
      }
      else if(strcmp(command, "SUB") == 0){
        varValue = varval1 - varval2;
      }
      else if(strcmp(command, "MUL") == 0){
        varValue = varval1 * varval2;
      }
      else if(strcmp(command, "DIV") == 0){
        varValue = varval1 / varval2;
      }
      else if(strcmp(command, "MOD") == 0){
        varValue = varval1 % varval2;
      }
      else{
        fprintf(stderr, "%s: not a command. This should not appear.\n", command);
        exit(1);
      }

      modify_local_node(locals, var_op, varValue, !PRINT_REPORT);

    }
    else if(command_group == 7){ // DOWN, UP
      //Perimenei GlobalVar
      // if(temp_char == ' '){
      //   discard_spaces(fd, input_buffer, BLOCK_N_LINE_CHAR);
      // }
      // else if(temp_char == '\n') {
      //   fprintf(stderr, "Syntax error: Expected GlobalVar but found new line.\n");
      //   exit(1);
      // }
      // else{
      //   printf("sth terribly wrong\n");
      //   exit(1);
      // }

      check_inner_space(fd, &temp_char, "GlobalVar", command);
      input_buffer[0] = temp_char;

      varval1_op[0] = input_buffer[0];
      varval1 = check_varGlobal(fd, globals, varval1_op, &temp_char, command);

      if(strcmp(command, "DOWN") == 0){
        varval1 = varval1 - 1;
      }
      else if(strcmp(command, "UP") == 0){
        varval1 = varval1 + 1;
      }

      modify_global_node(globals, varval1_op, varval1, !PRINT_REPORT);


    }
    else if(command_group == 8){ // SLEEP
      //Perimenei VarVal
      check_inner_space(fd, &temp_char, "var", command);
      input_buffer[0] = temp_char;

      fprintf(stderr, "SLEEP: var = %c\n", input_buffer[0]);
      varval1_op[0] = input_buffer[0];

      varval1 = check_varval(fd, locals, varval1_op, &temp_char, command);
      sleep(varval1);
    }
    else if(command_group == 5){ // BRGT, BRGE, BRLT, BRLE, BREQ
      //Perimenei VarVal, VarVal kai Label

      /**************************** VARVAL1 ***********************************/
      varval1 = read_varval(fd, locals, &temp_char, command);
      /**************************** VARVAL2 ***********************************/
      varval2 = read_varval(fd, locals, &temp_char, command);
      /****************************** LABEL ***********************************/
      read_label(fd, &temp_char, input_buffer, command);
      /******************** CHECK CONDITION ***********************************/
      //BRGT, BRGE, BRLT, BRLE, BREQ
      if(strcmp(command, "BRGT") == 0){
        varValue = (varval1 > varval2) ? 1 : 0;
      }
      else if(strcmp(command, "BRGE") == 0){
        varValue = (varval1 >= varval2) ? 1 : 0;
      }
      else if(strcmp(command, "BRLT") == 0){
        varValue = (varval1 < varval2) ? 1 : 0;
      }
      else if(strcmp(command, "BRLE") == 0){
        varValue = (varval1 <= varval2) ? 1 : 0;
      }
      else if(strcmp(command, "BREQ") == 0){
        varValue = (varval1 == varval2) ? 1 : 0;
      }
      else{
        fprintf(stderr, "%s: not a command. This should not appear.\n", command);
        exit(1);
      }
      printf("%s: varValue = %d\n", command, varValue);

      /******************** CHECK LABEL ***************************************/
      if (varValue){
        temp_offset = search_label(labels, input_buffer, PRINT_REPORT);

        if (temp_offset == INVALID_OFFSET){
          /******************** IF STATEMENT ************************************/
          fprintf(stderr, "%s: Label with name %s not in labels list\n", command, input_buffer);
          search_label_downwards(fd, temp_char, input_buffer);
        }
        else {
          /*********************** IN THE LOOP **********************************/
          if (lseek(fd, temp_offset, SEEK_SET) == -1){
            fprintf(stderr, "%s: Error with lseek\n", command);
            exit(1);
          }
        }
      }
      printf(ANSI_COLOR_BLUE"%d %d %s "ANSI_COLOR_RESET"\n", varval1, varval2, input_buffer);
    }
    else if(command_group == 6){ // BRA
      //Perimenei Label

      read_label(fd, &temp_char, input_buffer, command);
      /******************** CHECK LABEL ***************************************/
      temp_offset = search_label(labels, input_buffer, PRINT_REPORT);

      if (temp_offset == INVALID_OFFSET){
        /******************** IF STATEMENT ************************************/
        fprintf(stderr, "BRA: Label with name %s not in labels list\n", input_buffer);
        search_label_downwards(fd, temp_char, input_buffer);

      }
      else {
        /*********************** IN THE LOOP **********************************/
        if (lseek(fd, temp_offset, SEEK_SET) == -1){
          fprintf(stderr, "BRA: Error with lseek\n");
          exit(1);
        }
      }
      printf(ANSI_COLOR_BLUE"%s "ANSI_COLOR_RESET"\n", input_buffer);
    }
    else{
      fprintf(stderr, "EDW DEN PROKEITAI POTE NA FTASEI\n");
    }

    //edw ftanei (AKA h teleutaia trypa ths flogeras)

    printf(ANSI_COLOR_BLUE"\n"ANSI_COLOR_RESET);
    printf(ANSI_COLOR_GREEN"\nlocals: "); print_local_contents(locals);
    printf("\nglobals: ");                print_global_contents(globals);
    printf("\nlabels: ");                 print_labels(labels);
    printf(ANSI_COLOR_RESET);
  }

  //den ftanei edw pote alla tha mpei se mia "terminating function"
  printf("\n"ANSI_COLOR_GREEN"PROGRAM RETURNED SUCCESSFULLY !"ANSI_COLOR_RESET"\n\n");
  destroy_local_list(locals, PRINT_REPORT);
  destroy_global_list(globals, PRINT_REPORT);
  destroy_labels(labels, PRINT_REPORT);
  close(fd);

  return 0;
}
