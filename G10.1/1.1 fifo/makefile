all: fifo_tester.o fifo.o fifo.h
	gcc fifo_tester.o fifo.o -o fifo_tester -lpthread

fifo_tester: fifo_tester.o fifo.o fifo.h
	gcc fifo_tester.o fifo.o -o fifo_tester -lpthread

fifo_tester.o: fifo_tester.c fifo.h
	gcc -Wall -c fifo_tester.c -lpthread

fifo.o: fifo.c fifo.h
	gcc -Wall -c fifo.c

clean:
	rm -vf fifo.o fifo_tester.o fifo_tester
