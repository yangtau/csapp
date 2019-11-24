# Makefile for Proxy Lab 
#
# You may modify this file any way you like (except for the handin
# rule). You instructor will type "make" on your specific Makefile to
# build your proxy from sources.

CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -lpthread

all: proxy

csapp.o: csapp.c csapp.h
	$(CC) $(CFLAGS) -c csapp.c

queue.o: queue.c queue.h csapp.h
	$(CC) $(CFLAGS) -c queue.c

proxy.o: proxy.c csapp.h
	$(CC) $(CFLAGS) -c proxy.c

http.o: http.c http.h csapp.h
	$(CC) $(CFLAGS) -c http.c

proxy: proxy.o csapp.o queue.o http.o
	$(CC) $(CFLAGS) proxy.o csapp.o queue.o http.o -o proxy $(LDFLAGS)

test_queue.o: queue.h test_queue.c
	$(CC) $(CFLAGS) -c test_queue.c

test_queue: test_queue.o queue.o csapp.o
	$(CC) $(CFLAGS) test_queue.o queue.o csapp.o -o test_queue $(LDFLAGS)

clean:
	rm -f *~ *.o proxy core *.tar *.zip *.gzip *.bzip *.gz test_queue
