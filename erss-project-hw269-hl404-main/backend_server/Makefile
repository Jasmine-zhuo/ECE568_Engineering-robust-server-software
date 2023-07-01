GDB = -ggdb3
THREAD = -pthread
PQXX = -lpqxx -lpq
GPB = `pkg-config --cflags --libs protobuf`
PROTO = AUW_protocol.pb.cc
All:  miniUPS
miniUPS: main.cpp server.cpp client.cpp AUW_protocol.pb.cc Common.hpp database.hpp client.hpp server.hpp
	g++ -std=c++11 main.cpp server.cpp client.cpp database.cpp  $(PROTO) $(PQXX) $(GDB) $(THREAD) $(GPB)  -o miniUPS



server.o: server.cpp server.hpp
	g++ -std=c++11 server.cpp -c $(GDB) 
client.o: client.cpp client.hpp
	g++ -std=c++11 client.cpp -c $(GDB)	
database.o: database.cpp database.hpp Common.hpp
	g++ -std=c++11 database.cpp -c $(GDB) $(PQXX)
clean:
	rm -f *~ client.o server.o database.o miniUPS
