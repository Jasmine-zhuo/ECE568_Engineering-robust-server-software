#include <vector>
#include <cstdlib>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <cstring>
#include <iomanip>
#include <ctime>
#include <thread>
#include <signal.h>
#include <sstream>
#include <thread>

#include "rapidxml.hpp"
#include "XMLResponse.h"
#include "myException.h"
#include "database.h"
#include "rapidxml_print.hpp"

using namespace std;
using namespace rapidxml;

int setup_socket(void);
void run(int sockfd);
void deal_with_one_client(int client_fd);
int handle_requests(int client_fd, StockDatabase * my_database);
void parse_one_request(int client_fd, StockDatabase * my_database,vector<char>& buffer);
void deal_with_one_create_request(int client_fd, StockDatabase * my_database,xml_document<> & doc);
void deal_with_one_transaction_request(int client_fd, StockDatabase * my_database,xml_document<> & doc);
