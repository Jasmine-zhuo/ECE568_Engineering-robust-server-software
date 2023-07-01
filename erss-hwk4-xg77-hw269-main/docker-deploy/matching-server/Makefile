CC=g++ -std=c++11
CFLAGS=-ggdb3
EXTRAFLAGS=-lpqxx -lpq -lpthread

all: server

server: server.cpp utils.cpp database.cpp XMLResponse.cpp
	$(CC) $(CFLAGS) -o server server.cpp utils.cpp database.cpp XMLResponse.cpp $(EXTRAFLAGS)
clean:
	rm -f *~ *.o  server client

clobber:
	rm -f *~ *.o
