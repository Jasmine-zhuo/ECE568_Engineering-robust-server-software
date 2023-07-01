#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <ctime>
#include <chrono>
#include <sys/time.h>

using namespace std;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;
long first_send_time = 0;
long last_read_time = 0;
bool is_running=false;
int failed_client=0;
void error(const char * msg) {
  cout<<"error happened!! msg was:"<<msg<<endl;
  failed_client++;
  cout<<"renewed failed client number: "<<failed_client<<endl;
  //  return;
  exit(EXIT_SUCCESS);
}

void build_one_connection(char * host_name,char * xml_file_name){
  while(!is_running){

  }
 //create socket
  int sockfd;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  printf("create socket\n");
  printf("the socket number is %d\n",sockfd);
  if (sockfd < 0)
    error("ERROR opening socket");
  //get host name from HTTP request(from proxy service)  ??
  struct hostent * server;
  server = gethostbyname(host_name);
  printf("gethostbyname\n");
  if (server == NULL) {
    fprintf(stderr, "ERROR, no such host\n");
    return;
  }

  //connect port
  struct sockaddr_in serv_addr;
  int portno;

  //portno was phrase from http  ??
  portno = 12345;
  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(portno);
  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
      error("ERROR connecting");
  }
  cout<<"connected to server successfully!\n";
  int counts = 0;
  vector<char> buffer;
  string my_string;
  // fstream my_xml("sample.xml");
  fstream my_xml(xml_file_name);
  string length_added_s;
  if(my_xml.is_open()){
    stringstream ss;
    ss << my_xml.rdbuf();
    my_string = ss.str();
  }
  else{
    error("cannot open XML file!\n");
  }
  cout<<"sending:"<<endl;
  length_added_s = to_string(my_string.length())+"\n"+my_string;
  cout << length_added_s<<endl;

  /*
  time_t send_time = time(0);
  tm * s_time = localtime(&send_time);
  */

  auto millisec_since_epoch_1 = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
cout<<"send time is "<<millisec_since_epoch_1<<endl;
 stringstream ss;
 ss<<millisec_since_epoch_1;
 string send_time = ss.str();
 if(first_send_time ==0){ 
   first_send_time = atol(send_time.c_str());
   cout<<"*****************recorded first send time: "<<first_send_time<<endl;
 }
  int n =   write(sockfd,length_added_s.c_str(),length_added_s.length());
  if(n < 0 ){
    cout << "'write()' function failed!\n";
    return ;
  }

  
  //waiting to get response or tunnel close information from server
  
         fd_set socket_set;
        while (true) {
            FD_ZERO(&socket_set);
            FD_SET(sockfd, &socket_set);
            int n = select(1024, &socket_set, NULL, NULL, NULL);  //FD_SETSIZE = 1024
            if (n < 0) {
	      //      throw server_exception("'select()' function failed!");
	      cout<<"select failed!!!!!!!!!!!!!!!!!!!!!"<<endl;
            }
            if (FD_ISSET(sockfd, &socket_set)) {
	      vector<char> read_buffer(4096,'\0');

	      int status = read(sockfd,read_buffer.data(),4096);
	      /*
	      time_t read_time = time(0);
	      tm* r_time = localtime(&read_time);
	      time_t diff_time = difftime(read_time,send_time);
	      */


	        auto millisec_since_epoch_2 = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
cout<<"read time is "<<millisec_since_epoch_2<<endl;
 auto diff_time = millisec_since_epoch_2 - millisec_since_epoch_1;
 cout<<"diff time is "<<diff_time<<"milli seconds !\n";
 stringstream ss;                                                                               
 ss<<millisec_since_epoch_2;                                                                 
 string read_time = ss.str();
 long read_t = atol(read_time.c_str());
 if(last_read_time < read_t){
   last_read_time = read_t;
      cout<<"*****************recorded first send time: "<<first_send_time<<endl; 
 cout<<"**********renewed last read time:"<<last_read_time<<endl;
 cout<<"*********renewed total execution time for all clients:"<<last_read_time-first_send_time<<endl;
 }
                if(status == 0){
                    close(sockfd);
		    cout<<"tunnel closed!";
		    return;
		}
                if(status == -1){
		  cout<<"Didn't get response!\n";
		  close(sockfd);
		  return;
                }
		//status >0

	    
		string length_str;
                int i=0;
		for(auto c:read_buffer){
		  i++;
		  if(c=='\n'){
		    break;
		  }
		  length_str.push_back(c);
		}
		int length = atol(length_str.c_str());
		cout<<"received XML length:"<<length<<endl;
                string response_str;
		for(int j=i;j<i+length;j++){
		  response_str.push_back(read_buffer[j]);
		}
                cout<<"The response received is :\n";
		cout<<response_str<<endl;
		break;
	       }	      
	}//end of while loop
        close(sockfd);
        cout<<"This thread is closing!!\n";
        return;
}


int main(int argc,char ** argv) {
  if(argc!=4){
    cout<<"Usage: ./client <hostname> <input xml filename> <num of threads>"<<endl;
    return EXIT_SUCCESS;
  }
  char * hostname = argv[1];
  char * xml_file_name = argv[2];
  char * num_threads_char = argv[3];
  int thread_num = atol(num_threads_char);
  for(int i=0;i<thread_num;i++){
    
    thread new_thread(build_one_connection,hostname,xml_file_name);
    new_thread.detach();
    
  }

  is_running = true;

  while(1){
    //  cout<<"1111"<<endl;
  }
  /*
  cout<<"first send time and last read time is :"<<first_send_time<<", "<<last_read_time<<endl;
  cout<<"*********** avg execution time for a request: "<<(last_read_time-first_send_time)/thread_num<<endl;
  */
   return EXIT_SUCCESS;
}



