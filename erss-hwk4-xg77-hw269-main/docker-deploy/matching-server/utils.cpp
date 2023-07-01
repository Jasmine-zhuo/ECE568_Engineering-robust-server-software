#include "utils.hpp"
#define SERVER_BACKLOG 10000

int setup_socket(void){//this function returns a socket fd corresponding to our server
  signal(SIGPIPE, SIG_IGN);

  int sockfd;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  cout << "create a socket successfully" << endl;
  if (sockfd < 0){
    cout << "ERROR opening socket" << endl;
    exit(EXIT_FAILURE);
  }
  //bind port number and socket
  struct sockaddr_in serv_addr;
  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(12345);
  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
    cout << "ERROR on binding" << endl;
    exit(EXIT_FAILURE);
  }
  //listen to socket
  listen(sockfd, SERVER_BACKLOG);
  string port = "12345";
  cout << "Waiting for connection on port " << port << endl;
  return sockfd;
}


void run(int sockfd){//keep accepting new client processes and spawn new threads to deal with it
  while(true){
      try{
          int client_connection_fd;
          struct sockaddr_in cli_addr;
          socklen_t clilen;
          clilen = sizeof(cli_addr);
          client_connection_fd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
          cout << "Accepted a client! FD is " << client_connection_fd << endl;

          if (client_connection_fd == -1) {
              throw server_exception("Error: cannot accept connection on socket");
          }  //if
	  /*
          while (true){
	                  if(ThreadPool::GetInstance()->DoTaskWait(100, (TPTask)deal_with_one_client,(TPTaskArg)client_connection_fd)){
                  break;
              }
          }
	    */
          std::thread new_thread(deal_with_one_client,
                                 client_connection_fd);
          new_thread.detach();
      }
      catch(server_exception & e){
          cerr << e.what() << endl;
      }
  }
}

void deal_with_one_client(int client_fd){//can we change pointer to reference??
    try{
      // cout<<"keep connecting with client whose fd is "<< client_fd <<" !"<<endl;
        StockDatabase my_database;
        fd_set socket_set;
        while (true) {
            FD_ZERO(&socket_set);
            FD_SET(client_fd, &socket_set);
            int n = select(1024, &socket_set, NULL, NULL, NULL);  //FD_SETSIZE = 1024

            if (n < 0) {
                throw server_exception("'select()' function failed!");
            }

            if (FD_ISSET(client_fd, &socket_set)) {
                int status = handle_requests(client_fd, &my_database);

                if(status == 0){
		  cout<<"tunnel with client_fd "<<client_fd<<" closed!\n";
                    break; //jump out of the while loop
                }

                // error occurs in handle_request
                if(status == -1){
                    break;
                }
            }
        }//end of while loop
        close(client_fd);
    }
    catch (server_exception & e){
        cerr << e.what() << endl;
        close(client_fd);
    }

}

int handle_requests(int client_fd, StockDatabase * my_database){
    try{
      int n=0;
      int buffer_size = 4096;//IMPORTANT:No single request will be longer than this number
      vector<char> buffer(buffer_size,'\0');

        n = read(client_fd, buffer.data(), buffer_size);
	//        cout<<"-------------"<< n <<"bytes received--------------\n\n" << endl;
        if(n < 0){
            throw server_exception("'read()' function received failure!\n ");
        }
        // client closes the connection tunnel
        if(n == 0){
            return 0;
        }
	int i=0; //we read all the way from buffer[0] to buffer[n-1]

    for(auto c:buffer){
	  i++;
	  if(c=='\n'){
            break;
	  }
	}

	vector<char> real_xml;
	while(buffer[i]!='\0'){
          real_xml.push_back(buffer[i]);
	  i++;
	}
	real_xml.push_back('\0');
    parse_one_request(client_fd,my_database,real_xml);
 	 
    return 1; //status = 1 means everything is fine. Tunnel not closed yet!
          
     }
    catch(server_exception & e){
        cerr << e.what() << endl;
        return -1;
    }
      
}

void parse_one_request(int client_fd,StockDatabase * my_database,vector<char> & buffer){
       // let's begin parsing!!!!
        xml_document<> doc;
        xml_node<> * root_node = NULL;
	    doc.parse<0>(&buffer[0]); // throw what type exception if format is wrong ???
	    //  cout << "The root name is :"<< doc.first_node()->name() <<endl;
        stringstream ss;
        ss << doc.first_node()->name();
        string request_type = ss.str();

        // process <create> request
        if(request_type == "create"){
            cout << ">-----------dealing with a 'create' request-------------<\n";
            deal_with_one_create_request(client_fd,my_database,doc);
        }

        // process <transaction> request
        if(request_type == "transactions"){
            cout << ">------------dealing with a 'transaction' request-----------<\n";
	        deal_with_one_transaction_request(client_fd,my_database,doc);
        }
}

void deal_with_one_create_request(int client_fd,StockDatabase * my_database,xml_document<> & doc){

  xml_node<> * root_node = doc.first_node(); // tag <create>
  xml_node<> * cur_node = NULL;

  XMLResponse my_response;//there is one response for one 'create' request
  my_response.socket_fd = client_fd;
  xml_node<>* root_response = my_response.doc.allocate_node(node_element,"results");
  my_response.doc.append_node(root_response);
  my_response.root = root_response;

   cur_node = root_node->first_node("account");
  while(cur_node){//we can have 0 or more 'account' tags in a 'create' tag
    string id_str = cur_node->first_attribute("id")->value();
    string balance_str = cur_node->first_attribute("balance")->value();
    //    cout<< "id is:"<<id_str<<" , and balance is "<<balance_str<<endl;

    my_database->createAccount(id_str,balance_str,my_response);  
    cur_node = cur_node->next_sibling("account");
  }
  //then deal with the <symbol> tags
  cur_node = root_node->first_node("symbol");
  while(cur_node){
    string sym_str = cur_node->first_attribute("sym")->value();
    vector<pair<string,string> > sym_info_list;
    xml_node<> * cur_account_node = cur_node->first_node("account");

    while(cur_account_node){//a 'sym' tag has 1 or more 'id' children
    string id = cur_account_node->first_attribute("id")->value();
    string shares = cur_account_node->value();
    sym_info_list.push_back(make_pair(id,shares));
    cur_account_node = cur_account_node->next_sibling("account");
    }
    
    my_database->placeSymbol(sym_str,sym_info_list,my_response);
    cur_node = cur_node->next_sibling("symbol");
  }

   my_response.send_back_to_client(); 
}

void deal_with_one_transaction_request(int client_fd,StockDatabase * my_database,xml_document<> & doc){
  xml_node<> * root_node = doc.first_node(); // tag <transactions>
  xml_node<> * cur_node = NULL;
  string account_id_str = root_node->first_attribute("id")->value();
  
  XMLResponse my_response;
  my_response.socket_fd = client_fd;
  xml_node<>* root_response = my_response.doc.allocate_node(node_element,"results");
  my_response.doc.append_node(root_response);
  my_response.root = root_response;

  cur_node = root_node->first_node("order");
  while(cur_node){//we can have 0 or more <order> tags in a 'transactions' tag
    string sym_str = cur_node->first_attribute("sym")->value();
    string amount_str = cur_node->first_attribute("amount")->value();
    string limit_str = cur_node->first_attribute("limit")->value();
    //  cout<<"The information for this order tag is :\n";
    // cout<<"sym,amount,limit = "<<sym_str<<" , "<<amount_str<<" , "<<limit_str<<endl;
    my_database->processOrder(account_id_str,sym_str,amount_str,limit_str,my_response);
    cur_node = cur_node->next_sibling("order");
  }
  
  //then deal with the <query> tags
  cur_node = root_node->first_node("query");
  while(cur_node){
    string transaction_id_str = cur_node->first_attribute("id")->value();

    xml_node<> * status_node = my_response.doc.allocate_node(node_element, "status");
    char * id = my_response.doc.allocate_string(transaction_id_str.c_str());
    status_node->append_attribute(my_response.doc.allocate_attribute("id",id));
    my_response.cur_root_node = status_node;

    my_database->processQuery(account_id_str,transaction_id_str,my_response);
    my_response.root->append_node(status_node);

    cur_node = cur_node->next_sibling("query");
  }


  cur_node = root_node->first_node("cancel");
  while(cur_node){
    string transaction_id_str = cur_node->first_attribute("id")->value();

    xml_node<> * canceled_node = my_response.doc.allocate_node(node_element, "canceled");
    char * id = my_response.doc.allocate_string(transaction_id_str.c_str());
    canceled_node->append_attribute(my_response.doc.allocate_attribute("id",id));
    my_response.cur_root_node = canceled_node;
    
    my_database->processCancel(account_id_str,transaction_id_str,my_response);
    my_response.root->append_node(canceled_node);
    
    cur_node = cur_node->next_sibling("cancel");
  }
  my_response.send_back_to_client();
}

