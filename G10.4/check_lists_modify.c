#include "var_storage.h"

int main(int argc, char *argv[]) {
  localVar *head;
  // int i, value;
  // char temp[10];

  head = init_list();

  print_contents(head);

  add_node(head, head->prev, "name[0]", 12);
  add_node(head, head->prev, "name[1]", 10);
  add_node(head, head->prev, "temp", 9);
  print_contents(head);


  modify_node(head, "name[6]", 5, !PRINT_REPORT);
  print_contents(head);
  // printf("%s = %d\n", "name[6]", read_node(head, "name[6]", !PRINT_REPORT));



  // print_contents(head);

  // current = find_array_name(head, "name[2]", CREATE_PERMISSION, PRINT_REPORT);
  // print_contents(head);

  // for(i = 0; i < 3; i++){
  //   printf("Enter int to modify: ");
  //   scanf("%9s %d", temp, &value);
  //
  //   modify_node(head, temp, value, PRINT_REPORT);
  //   print_contents(head);
  // }

  destroy_list(head, PRINT_REPORT);

  return 0;
}
