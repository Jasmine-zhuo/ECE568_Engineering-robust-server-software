CC=g++ -std=c++11
CFLAGS=-ggdb3
EXTRAFLAGS=-lpqxx -lpq -lpthread

all: client 


client:client.cpp
	$(CC) $(CFLAGS) -o client client.cpp $(EXTRAFLAGS)

clean:
	rm -f *~ *.o   client

clobber:
	rm -f *~ *.o
