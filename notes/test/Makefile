CC = gcc
CFLAGS = -Wall -O1 -pg

all: poly1 poly2

poly1: poly1.c test.c
	$(CC) $(CFLAGS) -o poly1.o test.c poly1.c

poly2: poly2.c test.c
	$(CC) $(CFLAGS) -o poly2.o test.c poly2.c

test1: poly1.o
	./poly1.o data-10000000.txt 10000000
test2: poly2.o
	./poly2.o data-10000000.txt 10000000

clean:
	rm -rf *.o *.out