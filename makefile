CC=gcc
LDLIBS=-lm -lrt -pthread
CFLAGS=--std=c11 -Wall -g -O -pthread

pagerank: main.o calc.o graph.o error.o
	$(CC) $^ $(LDLIBS) -o $@

main.o: main.c error/error.h core/graph.h core/calc.h core/declarations.h
	$(CC) $(CFLAGS) -c $<

graph.o: core/graph.c core/graph.h
	$(CC) $(CFLAGS) -c $<

calc.o: core/calc.c core/calc.h
	$(CC) $(CFLAGS) -c $<

error.o: error/error.c error/error.h
	$(CC) $(CFLAGS) -c $<

clean: 
	rm -f *.o error/*.o core/*.o