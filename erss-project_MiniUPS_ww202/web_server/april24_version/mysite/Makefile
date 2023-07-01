CC=g++
CFLAGS=-O3
EXTRAFLAGS=-lpqxx -lpq

all: test

test: test.cpp
	$(CC) $(CFLAGS) -o test test.cpp $(EXTRAFLAGS)

clean:
	rm -f *~ *.o *# test

clobber:
	rm -f *~ *.o *#
