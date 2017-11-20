/*
 * This is a tester for macros' arguments.
 */
#include <stdio.h>

#define PRINT_NAME(name, age) printf(name);\
                         printf("!\n");\
                         printf(age"\n");

#define VAR(label) volatile int var1[label];
#define VAR1 static int var2 = 10;

#define EXECUTIE(body) body

int main(int argc,char *argv[]){

  EXECUTIE(int a = 12;printf("%d\n", a);printf("\n"););

  PRINT_NAME("hi", "world");
  VAR(2);
  VAR1;

  var1[0] = 123;
  // var2 = 1234;

  printf("%d\n", var1[0]);
  printf("%d\n", ++var2);


  return 0;
}
