#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "program_handler.h"
#include "program_parser.h"
#include "my_sema.h"

#define COMMAND_S 20
#define PROGRAM_NAME_S 20

programT *volatile programs;

volatile int volatile *assigned_arguments;
volatile char volatile *assigned_progr_name;
volatile my_bsem job_assingned;

volatile int program_killed;

void print_menu(){
  printf("\n***** MENU *****\n");
  printf("run <program name> <arguments>\nlist\nkill <program id>\nexit");
  printf("\n****************\n\n");
}

void *core (void * arg){
  int i;
  programT *current;

  // printf("IN core:\n");
  // for(i = 0; i < args[1]; i++){
  //   printf("args[%d] = %d\n", i+2, args[i+2]);
  // }

  printf("goinh to block\n");

  my_bsem_down((my_bsem*)&job_assingned, __LINE__);
  if(program_killed){
    pthread_exit();
  }
  printf("A job has been assigned to me.\n");
  printf("with assigned_progr_name = %s ", assigned_progr_name);
  printf("and ID = %d\n", assigned_arguments[0]);

  for(i = 0; i < assigned_arguments[1]; i++){
    printf("args[%d] = %d\n", i + 2, assigned_arguments[i+2]);
  }

  main_program((int)assigned_arguments[0], (int)assigned_arguments[1],
               (int*) assigned_arguments + 2, (char*)assigned_progr_name);

  current = search_program_id(programs, (int) assigned_arguments[0], PRINT_REPORT);
  current->running = NOT_RUNNING;

  printf("In core: program returned. Everything is ok!\n");
  return NULL;
}

void *user_inter_func (void * arg){
  int i, j, program_id;
  int *args;
  char format_str_c[20], format_str_p[20], byte;
  char command_input[COMMAND_S], program_name[PROGRAM_NAME_S];

  // INITIALISATIONS
  program_id = 11;
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
          printf("end of arguments\n");
          break;
        }
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

      // ASSIGN STO CORE *************************************************************
      assigned_arguments = (volatile int *) malloc(sizeof(int) * (i + 1));
      if(assigned_arguments == NULL){
        fprintf(stderr, "error with malloc for the volatile\n");
        exit(1);
      }
      for(j = 0; j < i + 2; j++){
        assigned_arguments[j] = args[j];
      }
      printf("assigned_arguments[0] = %d\n", assigned_arguments[0]);
      assigned_progr_name = (volatile char*) strdup(program_name);
      if(assigned_progr_name == NULL){
        fprintf(stderr, "Sth wrong with strdup.\n");
        exit(1);
      }
      printf("Usr interf: arg name(volatile): %s\n", assigned_progr_name);
      printf("Usr interf: args[1] - argc(volatile): %d\n", assigned_arguments[1]);

      if(my_bsem_up((my_bsem*)&job_assingned, __LINE__)){
        fprintf(stderr, "htan hdh up. sth is wrong\n");
        exit(1);
      }

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
      program_killed = 1;
      if(my_bsem_up((my_bsem*)&job_assingned, __LINE__)){
        fprintf(stderr, "htan hdh up. sth is wrong\n");
        exit(1);
      }
      pthread_exit(NULL);
    }
    else{

    }
  }
}

int main (int argc, char *argv[]){
  pthread_t user_interractionT, coreT;
  int check;

  program_killed = 0;
  programs = init_program_list();
  my_bsem_init((my_bsem*)&job_assingned, 0, __LINE__);

  // create core thread.
  check = pthread_create(&coreT, NULL, &core, NULL);
  if (check == 1){
    printf("%s\n", "pthread_create for core went wrong\n");
  }

  // create user interface thread.
  check = pthread_create(&user_interractionT, NULL, &user_inter_func, NULL);
  if (check == 1){
    printf("%s\n", "pthread_create for user interface went wrong\n");
  }

  // create cores


  pthread_join(user_interractionT, NULL);
  pthread_join(coreT, NULL);

  kill_all(programs, PRINT_REPORT);
  my_bsem_destroy((my_bsem*)&job_assingned, __LINE__);

  printf("main terminating\n");

  return 0;
}
