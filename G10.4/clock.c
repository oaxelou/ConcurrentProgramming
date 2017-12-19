#include <stdio.h>
#include <time.h>
#include <unistd.h>

int main () {
   time_t seconds1;//, seconds2;
   int seconds_to_sleep = 4;

   seconds1 = time(NULL);
   printf("Seconds since January 1, 1970 = %ld, %d\n", seconds1, (int)seconds1);
   //sleep(4);
  //  seconds2 = time(NULL);

  while(time(NULL) - seconds1 < seconds_to_sleep){} // perimenoume 4 seconds (isodunamei me to sleep(4);)
   printf("Seconds sleeping = %ld\n", /*seconds2*/ time(NULL)- seconds1);

   return(0);
}
