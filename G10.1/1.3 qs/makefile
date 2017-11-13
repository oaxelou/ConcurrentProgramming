all: qs_tester.o qs.o qs.h
	gcc qs_tester.o qs.o -o qs_tester -lpthread

qs_tester: qs_tester.o qs.o qs.h
	gcc qs_tester.o qs.o -o qs_tester -lpthread

qs_tester.o: qs_tester.c qs.h
	gcc -Wall -c qs_tester.c -lpthread

qs.o: qs.c qs.h
	gcc -Wall -c qs.c

clean:
	rm -vf qs qs.o qs_tester qs_tester.o

