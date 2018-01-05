#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "program_handler2.h"
#include "parser.h"
#include "mtx_sema.h"

#define COMMAND_S 20
#define VAR_SIZE 100
#define PROGRAM_NAME_S 100

#define TIME_TO_SWITCH 3

programT *volatile programs;
volatile int nofcores;
volatile my_bsem *sem;
pthread_mutex_t program_mtx;

volatile int exit_given;

void print_menu(){
  printf("\n***** MENU *****\n");
  printf("run <program name> <arguments>\nlist\nkill <program id>\nexit");
  printf("\n****************\n\n");
}

int check_tag(int fd){
  char input_buffer[100];
  int i;
  my_read(fd, input_buffer, __LINE__);
  if (input_buffer[0] != '#'){
    fprintf(stderr, "Program Tag does not start with '#'\n");
    close(fd);
    return 1;
  }

  discard_spaces(fd, input_buffer, ALLOW_N_LINE_CHAR);
  i = read_island(fd, input_buffer);
  input_buffer[i] = '\0';

  if (strcmp(input_buffer, PROGRAM_TAG)!=0){
    fprintf(stderr, "Incorrect Program Tag\n");
    return 1;
  }
  return 0;
}

void *core(void * arg){
  int my_no, program_counter;
  programT *current_program  = programs;

  /* take arguments*/
  my_no = *(int *)arg;
  my_bsem_up((my_bsem*)&(sem[my_no]), __LINE__);

  program_counter = 0;
  while(1){
    // sleep(1);
    if(exit_given){
      break;
    }

    mtx_lock(&program_mtx, __LINE__);
    if(program_counter == TIME_TO_SWITCH || current_program->core != my_no){
      program_counter = 0;
      current_program = current_program->next;
    }

    if(current_program->core == my_no){
      program_counter++;
      if(current_program->running){
        if(!current_program->blocked){
          current_program->running_now = 1;
          mtx_unlock(&program_mtx, __LINE__);
          main_program(current_program, programs, &program_mtx);
          mtx_lock(&program_mtx, __LINE__);
          current_program->running_now = 0;
          if(current_program->running == 0){
            printf("\n"ANSI_COLOR_GREEN"PROGRAM RETURNED SUCCESSFULLY !"ANSI_COLOR_RESET"\n\n");
            
            destroy_labels(current_program->labels, !PRINT_REPORT);
            destroy_list(current_program->locals, !PRINT_REPORT);
            current_program->core = INIT_CORE;
            close(current_program->fd);
	    #ifdef debug
              printf("before rebalancing\n"); print_programs(programs);
            #endif
            rebalance_cores(programs, nofcores);
            #ifdef debug
              printf("after rebalancing\n"); print_programs(programs);
            #endif
            current_program = programs;
          }
        }
        else if(current_program->blocked){
          if(current_program->sleep_start >= 0){
            if(time(NULL) - current_program->sleep_start >= current_program->sleep_time){
              current_program->blocked = 0;
              current_program->sleep_start = -1;
              current_program->sleep_time = -1;
              // printf("woke up\n");
            } // else: sleeping...
          }
          else if(current_program->down_sem != NULL){
            if(current_program->woke_up){  //sb woke me up!
              current_program->blocked = 0;
              current_program->down_sem = NULL;
              current_program->woke_up = 0;
            }
          }
          else{
            fprintf(stderr, "Blocked but neither with sleep nor with down (should never appear)\n");
            exit(1);
          }
        }
        else{
          fprintf(stderr, "error with programT field values (blocking). should never appear\n");exit(1);
        }
      }
      else{ //gia thn periptwsh kill
        destroy_labels(current_program->labels, !PRINT_REPORT);
        destroy_list(current_program->locals, !PRINT_REPORT);
        current_program->core = INIT_CORE;
        current_program->blocked = 0;
        close(current_program->fd);

        #ifdef debug
          printf("before rebalancing\n"); print_programs(programs);
        #endif
        rebalance_cores(programs, nofcores);
        #ifdef debug
          printf("after rebalancing\n"); print_programs(programs);
        #endif
        current_program = programs;
      }
    }
    mtx_unlock(&program_mtx, __LINE__);
  }

  return NULL;
}

void *user_inter_func (void * arg){
  int i, program_id, fd;
  int *args;
  char format_str_c[20], format_str_p[20], byte;
  char command_input[COMMAND_S], program_name[PROGRAM_NAME_S], var_name[VAR_SIZE];
  labelsT *labels;
  varT *locals;
  programT *current;

  // INITIALISATIONS
  program_id = 0;
  sprintf(format_str_c, "%%%ds", COMMAND_S - 1);
  sprintf(format_str_p, "%%%ds", PROGRAM_NAME_S - 1);
  exit_given = 0;

  print_menu();

  while(1){

    /* READ WHAT THE USER WANTS */
    scanf(format_str_c, command_input);

    /***************************** RUN COMMAND GIVEN **************************/
    if(strcmp(command_input, "run") == 0){
      scanf(format_str_p, program_name);

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
          break;
        }
        scanf("%d", args + i);

        args[1]++;
        args = (int *) realloc(args, (i + 2) * sizeof(int));
        if(args == NULL){
          fprintf(stderr, "Realloc couldn't realloc\n");
        }
        i++;
      }
      // exei parei ta args

      /********************** ARXIKOPOIHSEIS **********************************/
      labels = init_labels();
      locals = init_list();

      add_node(locals, locals->prev, "$argv[0]", program_id);
      for(i = 0; i < args[1]; i++){
        sprintf(var_name , "$argv[%d]", i + 1);
        add_node(locals, locals->prev, var_name, args[i + 2]);
      }
      add_node(locals, locals->prev, "$argc", args[1] + 1);

      fd = open(program_name, O_RDWR, S_IRWXU);
    	if(fd == -1){
    		perror("open");
        continue;
    		// exit(1);
      }
      /************************* PROGRAM TAG CHECK ****************************/
      if(check_tag(fd)){continue;}

      mtx_lock(&program_mtx, __LINE__);
      printf("\n"ANSI_COLOR_GREEN"PROGRAM JUST STARTED !"ANSI_COLOR_RESET"\n\n");
      add_program(programs, program_name, find_less_busy_core(programs, nofcores), program_id, locals, labels, fd);
      mtx_unlock(&program_mtx, __LINE__);

      program_id++;
    }
    else if(strcmp(command_input, "list") == 0){
      mtx_lock(&program_mtx, __LINE__);
      print_programs(programs);
      mtx_unlock(&program_mtx, __LINE__);
    }
    else if(strcmp(command_input, "kill") == 0){
      scanf("%d", &program_id);

      mtx_lock(&program_mtx, __LINE__);
      for(current = programs->next; current != programs; current = current->next){
        if(current->id == program_id){
          current->running = !RUNNING;
          break;
        }
      }
      if(current == programs){
        printf("Didn't find program with id %d\n", program_id);
      }
      mtx_unlock(&program_mtx, __LINE__);
    }
    else if(strcmp(command_input, "exit") == 0){ // free ta panta apo pantou kai terminate ta panta
      exit_given = 1;
      pthread_exit(NULL);
    }
  }
}

int main (int argc, char *argv[]){
  pthread_t user_interractionT;
  pthread_t *coreT;
  int check, i;

  programs = init_program_list();
  globals = init_list();
  mtx_init(&program_mtx, __LINE__);


  mtx_init(&glob_mtx, __LINE__);
  /****************************************************************************/
  printf("Number of cores: ");
  scanf("%d", &nofcores);

  //allocating sem & pthread_t
  sem = (volatile my_bsem *)malloc(sizeof(my_bsem)*nofcores);
  if(sem == NULL){fprintf(stderr, "error malloc sem\n");exit(1);}
  coreT = (pthread_t *)malloc(sizeof(pthread_t)*nofcores);
  if(coreT == NULL){fprintf(stderr, "error malloc pthread_t\n");exit(1);}

  // initialising semaphores
  for (i=0; i<nofcores; i++){
      my_bsem_init((my_bsem*)&(sem[i]), 0, __LINE__);
  }

  // create core threads.
  for (i=0; i<nofcores; i++){
      if (pthread_create(&coreT[i], NULL, core, (void*)&i)){
        printf("error at pthread_create no: %d\n", i);
        exit(1);
      }
      // waits for the thread to take its argument
      // before continuing with the next one
      my_bsem_down((my_bsem*)&(sem[i]), __LINE__);
  }

  // create user interface thread.
  check = pthread_create(&user_interractionT, NULL, &user_inter_func, NULL);
  if (check == 1){
    printf("%s\n", "pthread_create for user interface went wrong\n");
  }

  /****************************************************************************/

  pthread_join(user_interractionT, NULL);

  for(i = 0; i < nofcores; i++){
    pthread_join(coreT[i], NULL);
    my_bsem_destroy((my_bsem*)sem + i, __LINE__);
  }
  free(coreT);
  free((my_bsem*)sem);

  #ifdef debug
    destroy_programs(programs, PRINT_REPORT);
  #else
    destroy_programs(programs, !PRINT_REPORT);
  #endif

  #ifdef debug
    printf("\nGlobals:\n");destroy_list(globals, PRINT_REPORT);
  #else
    destroy_list(globals, !PRINT_REPORT);
  #endif

  mtx_destroy(&glob_mtx, __LINE__);
  mtx_destroy(&program_mtx, __LINE__);

  printf("Deallocated memory.\nmain terminating\n");

  return 0;
}
