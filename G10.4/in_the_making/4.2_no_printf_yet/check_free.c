#include "program_handler.h"

int main(int argc, char const *argv[]) {
  programT *programs, *current; int running_core, blocked_core, nofcores = 2; running_core = blocked_core = 0;

  programs = init_program_list();

  print_programs(programs);
  add_program(programs, "name", 1, 10, NULL, NULL, 10);

  current = search_program_id(programs, 10, PRINT_REPORT); //psaxnei me to id
  current->running = 0;
  print_programs(programs);
  // rebalance_cores(programs, 2);
  // printf("current->name = %s\n", current->next->name);

  // for(current = programs->next; current != programs; current = current->next){
  //   if(current->running){
  //     if(current->blocked){
  //       current->core = blocked_core % nofcores;
  //       blocked_core++;
  //     }
  //     else{
  //       current->core = running_core % nofcores;
  //       running_core++;
  //     }
  //   }
  //   else{
  //     // remove_program(current);
  //   }
  // }
  current->prev->next = current->next;
  current->next->prev = current->prev;

  free(current->name);      printf("current->name = %s\n", current->name);
  free((struct program_struct *)current);
  current = current->next;
  printf("current->name = %s\n", current->prev->name);sleep(1);

  printf(ANSI_COLOR_RED"counted %d blocked programs"ANSI_COLOR_RESET"\n", blocked_core);
  printf(ANSI_COLOR_RED"counted %d running programs"ANSI_COLOR_RESET"\n", running_core);

  print_programs(programs);
  destroy_programs(programs, PRINT_REPORT);

  return 0;
}
