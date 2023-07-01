all:proxy_server

proxy_server: server.cpp 
	g++ -std=c++11 server.cpp cache.cpp HTTPProtocol.cpp HTTPClient.cpp log.cpp Utils.cpp -lpthread -o haoserver

.PHONY: clean
clean:
	rm haoserver
