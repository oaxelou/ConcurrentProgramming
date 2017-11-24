/*
 * This is a tester for macros' arguments.
 */
#include <stdio.h>

#define defineVar int v2
#define PRINT_NAME(name, age) while(1){\
                         printf("%s %d?!\n", #name, v##name);\
                         printf(age"\n");break;\
                       }

#define VAR(label) volatile int var1[label];
#define VAR1 static int var2 = 10;

#define EXECUTIE(body) body

int main(int argc,char *argv[]){

  EXECUTIE(int a = 12;printf("%d\n", a);printf("\n"););


  printf("********************\n");
  defineVar;
  v2 = 123456;
  PRINT_NAME(2, "world");
  printf("********************\n");

  VAR(2);
  VAR1;

  var1[0] = 123;
  // var2 = 1234;

  printf("%d\n", var1[0]);
  printf("%d\n", ++var2);


  return 0;
}
