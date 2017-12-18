#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "program_handler.h"

#define COMMAND_S 20
#define PROGRAM_NAME_S 20

programT *volatile programs;

void print_menu(){
  printf("\n***** MENU *****\n");
  printf("run\nlist\nkill\nexit");
  printf("\n****************\n\n");
}

void *thr_func (void * arg){
  int i;
  int *args = (int *)arg;
  printf("IN FOO:\n");
  for(i = 0; i < args[1]; i++){
    printf("args[%d] = %d\n", i+2, args[i+2]);
  }

  return NULL;
}

void *user_inter_func (void * arg){
  int i, program_id;
  int *args;
  char format_str_c[20], format_str_p[20], byte;
  char command_input[COMMAND_S], program_name[PROGRAM_NAME_S];

  // INITIALISATIONS
  program_id = 0;
  sprintf(format_str_c, "%%%ds", COMMAND_S - 1);
  sprintf(format_str_p, "%%%ds", PROGRAM_NAME_S - 1);

  print_menu();

  while(1){

    /* READ WHAT THE USER WANTS */
    scanf(format_str_c, command_input);
    printf("command given: %s\n", command_input);

    /***************************** RUN COMMAND GIVEN **************************/
    if(strcmp(command_input, "run") == 0){
      scanf(format_str_p, program_name);
      printf("program_name given: %s\n", program_name);

      args = (int *)malloc(3 * sizeof(int));
      if(args == NULL){
        fprintf(stderr, "Error with malloc\n");
        exit(1);
      }
      args[0] = program_id;
      args[1] = 0; //argc
      i = 2;

      while(1){
        byte = getchar();

        if(byte == '\n'){
          printf("INDEED\n");
          break;
        }
        printf("wrong assumption. byte = %c\n", byte);
        scanf("%d", args + i);

        printf("byte = %c & args[%d] = %d\n", byte, i, args[i]);
        args[1]++;
        args = (int *) realloc(args, (i + 2) * sizeof(int));
        if(args == NULL){
          fprintf(stderr, "Realloc couldn't realloc\n");
        }

        i++;
      }
      // exei parei ta args

      for(i = 0; i < args[1] ; i++){
        printf("args[%d] = %d\n", i + 2, args[i + 2]);
      }
      //sleep(1);

      add_program(programs, program_name, 1234, program_id);
      program_id++;
    }
    else if(strcmp(command_input, "list") == 0){
      print_programs(programs);
    }
    else if(strcmp(command_input, "kill") == 0){
      // diavasma tou anagnwristikou pou dinei o xrhsths
      // search to anagnwristiko pou dinei o xrhsths
      // an den to vrei ektupwnei mhnuma kai continue

      // an to vrei, psaxnei to core sto opoio ekteleitai to programma
      // stelei shma me kapoio tropo sto core na stamathsei na to ektelei

      // (genika) tha mporouse meta apo kathe entolh(arxh h telos ths while) na
      // tsekarei ena flag an tou exei dothei timh pou shmainei "kill yourself"
      // kai na kanei break & free tis topikes tou metablhtes
    }
    else if(strcmp(command_input, "exit") == 0){

      // free ta panta apo pantou kai terminate ta panta
      pthread_exit(NULL);
    }
    else{

    }
  }
}

int main (int argc, char *argv[]){
  pthread_t user_interractionT;
  int check, i;

  programs = init_program_list();


  // create user interface thread.
  check = pthread_create(&user_interractionT, NULL, &user_inter_func, NULL);
  if (check == 1){
    printf("%s\n", "pthread_create for user interface went wrong\n");
  }

  // create cores


  pthread_join(user_interractionT, NULL);
  kill_all(programs, PRINT_REPORT);

  printf("main terminating\n");

  return 0;
}
