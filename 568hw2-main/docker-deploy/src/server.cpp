#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <thread>
#include <signal.h>
#include "HTTPClient.hpp"
#include "HTTPProtocol.hpp"
#include "Utils.hpp"
#include "cache.hpp"
#include "log.hpp"
#include <sstream>

using namespace std;

void deal_with_one_request(int client_fd,
                           int request_id,
                           Cache * my_cache,
                           log * my_log) {
  // cout << "I am thread No." << request_id << "and I am logging!" << endl;
  try{  
  //  my_log->initial_log(
  //  request_id, "GET vcm-181.duke.edu", "faluse IP", "2022-02-14 11:16");
  
  HTTPRequest my_request(client_fd);  //
  //we will have request.port  request.host request.method
  //log request
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
    auto str = oss.str();
  my_log->initial_log(request_id,my_request.getFirstLine(),my_request.getHostName(),str);
  if (my_request.getMethod() == "POST") {
    //do the job: communicate with the real server-->send(fd)-->receive response(recv(fd)
    cout<<"dealing with a "<<"\""<<"POST"<<"\""<<"request!**********"<<endl;
    HTTPClient my_client(my_request);
    HTTPResponse my_response = my_client.send(my_request);
    my_log->request_to_origin_server_log(request_id,my_request.getFirstLine(),my_request.getHostName());
    my_log->response_from_origin_server_log(request_id,my_response.getFirstLine(),my_request.getHostName());
    vector<char> response_rawdata = my_response.getRawData();  //maybe uncorrect
    std::string status_code = my_response.getStatusCode();     //'200' in "GET 200 OK"

    if (response_rawdata.size() == 0) {  //special case handling
      my_log->warning_log(request_id, "Got Empty Response from the HOST!");
    }

    std::string first_line =
        my_response
            .getFirstLine();  //the entire resonse line except from the new-line character --- "HTTP/1.1 200 OK"
    //    cout << "The proxy got the first line of the response, it is:\n" << first_line;
    int first_bit_of_response_code = status_code[0] - '0';  //could be 1,2,3,4 or 5

    switch (first_bit_of_response_code) {
      case 1:
        my_log->note_log(request_id,
                         "Request received, server is continuing the process!");
        break;
      case 2:
        my_log->response_to_client_log(request_id, first_line);
        //send->back
        break;
      case 3:
        my_log->note_log(
            request_id, "Further action must be taken in order to complete the request.");
        break;
      case 4:
        my_log->error_log(request_id,
                          " The request contains bad syntax or cannot be understood.");
        break;
      case 5:
        my_log->error_log(request_id,
                          "The Server failed to fullfill an apparently valid request!");
        break;
      default:;  //do nothing
    }
    write(client_fd, response_rawdata.data(), response_rawdata.size());

    //send data back to client!
    //send(fd,response_rawdata)
    close(client_fd);
    my_client.closeClient();
  }
  else if (my_request.getMethod() == "CONNECT") {
    //*****firt build a socket using connect with origin server but do not send any request to the origin server!
    cout<<"dealing with a CONNECT request!**********************"<<endl;
    HTTPClient my_client(my_request);
    if (my_client.isClosed()) {
      //add error message :"connet to origin server failed"
      close(client_fd);
      return;
    }
    //    cout<<"test point 1"<<endl;
    string connect_success_message = "HTTP/1.1 200 OK\r\n\r\n";
    int length = connect_success_message.length();
    int status = send(client_fd, connect_success_message.c_str(), length, 0);
    my_log->response_to_client_log(request_id,connect_success_message);              
         if (status == -1) {
      //throw exception! ("Error: cannot send back connect success message to client!")
       throw proxy_exception("Send back to client failed!");
	 }
    int proxy_as_client_fd = my_client.getSocketFD();
    fd_set socket_set;
   
    while (true) {
      FD_ZERO(&socket_set);
      FD_SET(client_fd, &socket_set);
      FD_SET(proxy_as_client_fd, &socket_set);
      int n = select(1024, &socket_set, NULL, NULL, NULL);  //FD_SETSIZE = 1024
      //  cout<<"test point 3"<<endl;
      if (n < 0) {
        // throw ConnectionErrorException();	
       throw proxy_exception("Select failed!");
      }
      if (FD_ISSET(proxy_as_client_fd, &socket_set)) {
        std::vector<char> buf(BUFSIZ);
        int status;
        int n = recv(proxy_as_client_fd, buf.data(), BUFSIZ, 0);
        if (n == 0) {
          status = 0;
        }
        if (n < 0) {
          status = -1;
        }
        status = send(client_fd, buf.data(), n, 0);
        if (status < 0) {
          status = -1;
        }
      }
      // cout<<"test point 4"<<endl;
      if (FD_ISSET(client_fd, &socket_set)) {
        std::vector<char> buf(BUFSIZ);
        int status;
        int n = recv(client_fd, buf.data(), BUFSIZ, 0);
        if (n == 0) {
          status = 0;
        }
        if (n < 0) {
          status = -1;
        }
        status = send(proxy_as_client_fd, buf.data(), n, 0);
        if (status < 0) {
          status = -1;
        }
      }
      if (status < 0) {
        //    throw ConnectionErrorException();
              throw proxy_exception("Connection failed!");
	break;
      }
      if (status == 0) {  //sending an empty message means the end of connection!
	cout<<"tunnel closed!\n";
	my_client.closeClient();
        close(client_fd);
        my_log->tunnel_close_log(request_id);
	break;
      }
    }
  }
  else if (my_request.getMethod() == "GET") {
    cout<<"delaing with a GET request!***************************"<<endl;
    HTTPClient my_client(my_request);
    if (!my_cache->findCache(
            my_request)) {  //cannot found corresponding response in cache,maybe the first time we see this request
      my_log->GET_result_log(request_id,1,"");
      HTTPResponse my_response = my_client.send(my_request);
    my_log->request_to_origin_server_log(request_id,my_request.getFirstLine(),my_request.getHostName());              
    my_log->response_from_origin_server_log(request_id,my_response.getFirstLine(),my_request.getHostName());               vector<char> response_raw_data = my_response.getRawData();
      write(client_fd,
            response_raw_data.data(),
            response_raw_data.size());  //send the response back to the user
      string status_code = my_response.getStatusCode();
      std::string first_line =
          my_response
              .getFirstLine();  //the entire resonse line except from the new-line character --- "HTTP/1.1 200 OK"
      int first_bit_of_response_code = status_code[0] - '0';  //could be 1,2,3,4 or 5

      string cache_control_directive;
      if (my_response.hasHeadKey("cache_control") ||
          my_response.hasHeadKey(
              "private")) {  //in these two conditions, we are not allowed to store this response into our cache (which is s shared-cache instead of a private cache)
        cache_control_directive = my_response.getHeadValueByKey("cache_control");
      }

      switch (first_bit_of_response_code) {
        case 1:
          my_log->note_log(request_id,
                           "Request received, server is continuing the process!");
          break;
        case 2:
          my_log->response_to_client_log(request_id, first_line);
          //find out if we should cache this one
          if (cache_control_directive !=
              "no_store") {  // save the current response to cache
            my_cache->addCache(my_request, my_response);
	  my_log->GET_OK_log(request_id,3,"","");
	  }
	  else{
	    my_log->GET_OK_log(request_id,1,"REASON","");
	  }
          break;
        case 3:
          my_log->note_log(
              request_id,
              "Further action must be taken in order to complete the request.");
          break;
        case 4:
          my_log->error_log(request_id,
                            " The request contains bad syntax or cannot be understood.");
          break;
        case 5:
          my_log->error_log(request_id,
                            "The Server failed to fullfill an apparently valid request!");
          break;
        default:;  //do nothing
      }

      my_client.closeClient();
      close(client_fd);
    }
    else {  //my_cache->findcache == TRUE
      //we need to figure out if we can use directly or we have to revalidate!
      //***************
      //first lets's find out if currently the response stored in our cache has been out-dated!
      vector<char> send_to_client_vec;  //declare the send back vector here
      vector<char>
          response_rawdata;  //if we need to get a new response, then this is its rawdata
      bool renew_flag =
          false;  //default situation is that we cannot use the cached response
      string cache_control_directive;
      HTTPResponse cached_response = my_cache->getResponse(my_request);
      if (cached_response.hasHeadKey("cache_control")) {
        cache_control_directive = cached_response.getHeadValueByKey("cache_control");
      }

      bool is_valid = my_cache->isValidResponse(cached_response);

      bool scenario1 = (cache_control_directive == "must-revalidate") && (!is_valid);
      bool scenario2 = (cache_control_directive == "no-cache") && (!is_valid);
      bool scenario3 = (cache_control_directive == "proxy-revalidate") && (!is_valid);
      if (scenario1 || scenario2 || scenario3) {  //needs revaidation!
	my_log->GET_result_log(request_id,3,"");
        //Step1: add 'If-None-Match' line and/or If-Modified-Since line to request header lines
        //if the originally cached response contains e-tag or last_modiofied, we need to revise the current request;else,we can just send the request without modifying
        HTTPResponse my_response;  //this is the name of the respose we are about to get
        if (cached_response.has_Etag() || cached_response.has_last_modified()) {
          vector<char> rawdata_my_request = my_request.getRawData();
          string rawdata_my_request_string = vector_char_to_string(rawdata_my_request);

          //Step2: send revised request to origin server
          int insert_position =
              rawdata_my_request_string.find("\r\n");  //insert after request line
          string Line_if_none_match = NULL;            //initialize inserting line1
          string Line_if_modified_since = NULL;        //initialize inserting line2
          if (cached_response.has_Etag()) {
            Line_if_none_match =
                "If-None-Match: " + cached_response.getHeadValueByKey("etag") + "\r\n";
          }
          if (cached_response.has_last_modified()) {
            Line_if_modified_since = "If-Modified-Since: " +
                                     cached_response.getHeadValueByKey("last-modified") +
                                     "\r\n";
          }
          string insertLines = Line_if_modified_since + Line_if_none_match;
          rawdata_my_request_string.insert(insert_position, insertLines);
          //Step2 : send revised request to original server
          vector<char> revised_raw_data(rawdata_my_request_string.begin(),
                                        rawdata_my_request_string.end());
          my_request.reviseRawdata(revised_raw_data);
          my_response = my_client.send(my_request);
        }
        else {  //no need to revise, nothing to insert to my_request,send directly
          my_response = my_client.send(my_request);

    my_log->request_to_origin_server_log(request_id,my_request.getFirstLine(),my_request.getHostName());              
    my_log->response_from_origin_server_log(request_id,my_response.getFirstLine(),my_request.getHostName());       	  
        }
        //Step3: parse the response and possibly change the response we stored in cache
        //For step3: if receive a 304-->update response and reuse;if receive a full response-->replace in cache and send back new response; if 5xx-->error message and still send back to client this 5xx response
        response_rawdata = my_response.getRawData();            //maybe uncorrect
        std::string status_code = my_response.getStatusCode();  //'200' in "GET 200 OK"

        if (response_rawdata.size() == 0) {  //special case handling
          my_log->warning_log(request_id, "Got Empty Response from the HOST!");
        }

        if (status_code == "304") {  //renew in cache

          renew_flag = true;
        }

        std::string first_line =
            my_response
                .getFirstLine();  //the entire resonse line except from the new-line character --- "HTTP/1.1 200 OK"
        cout << "The proxy got the first line of the response, it is:\n" << first_line;
        int first_bit_of_response_code = status_code[0] - '0';  //could be 1,2,3,4 or 5

        switch (first_bit_of_response_code) {
          case 1:
            my_log->note_log(request_id,
                             "Request received, server is continuing the process!");
            break;
          case 2:
            my_log->response_to_client_log(request_id, first_line);
            break;
          case 4:
            my_log->error_log(
                request_id, " The request contains bad syntax or cannot be understood.");
            break;
          case 5:
            my_log->error_log(
                request_id, "The Server failed to fullfill an apparently valid request!");
            break;
          default:;  //do nothing
        }
      }
      else {  //do not need revalidation!
              //step1: Renew the time/date information os the response stored in the cache
	my_log->GET_result_log(request_id,4,"");
	renew_flag = true;
      }

      if (renew_flag == true) {
        my_cache->renewDateofResponse(my_request);
        //directly send back to client using this response we cached(and renewed) in the cache
        HTTPResponse renewed_response = my_cache->getResponse(my_request);
        vector<char> rawdata_new_response = renewed_response.getRawData();
        //log the response!
        //use the renewed response (in cache)
        send_to_client_vec = rawdata_new_response;
      }
      else {  //use the newly recieved response
        send_to_client_vec = response_rawdata;
      }
      //finally  send back to client
      int n = write(client_fd, send_to_client_vec.data(), send_to_client_vec.size());
      
      if (n < 0) {
        //raise exception: writing to socket failed
	throw proxy_exception("Writing to socket failed!");
      }
    }  //end of "in cache situation"
    close(client_fd);
    my_client.closeClient();

  }  //end of "GET" request
  }//end of "Try"

  catch (proxy_exception & e){
    cout<<"An error happend! "<<e.what()<<endl;
    close(client_fd);
  }
  return;
}

int main(int argc, char * argv[]) {
  
  // sigaction(SIGPIPE, &(struct sigaction){SIG_IGN}, NULL);
  signal(SIGPIPE,SIG_IGN);
  Cache my_cache;
  log my_log;
  int sockfd;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  printf("create a socket successfully\n");
  if (sockfd < 0)
    cout << "ERROR opening socket" << endl;

  //bind port number and socket
  struct sockaddr_in serv_addr;
  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(12345);
  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    cout << "ERROR on binding" << endl;

  //listen to socket
  listen(sockfd, 100);
  string port = "12345";
  cout << "Waiting for connection on port " << port << endl;

  int request_id_count = 101;  //We use this to give every request an unique ID
  while (1) {
    /*    struct sockaddr_storage socket_addr;
    socklen_t socket_addr_len = sizeof(socket_addr);
    int client_connection_fd;
      client_connection_fd =
      accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    */

    int client_connection_fd;
    struct sockaddr_in cli_addr;
    socklen_t clilen;
    clilen = sizeof(cli_addr);
    client_connection_fd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    printf("create a newsockfd successfully\n");
    cout << "Accepted a client! FD is " << client_connection_fd << endl;
    if (client_connection_fd == -1) {
      cerr << "Error: cannot accept connection on socket" << endl;
      return -1;
    }  //if

    std::thread new_thread(deal_with_one_request,
                           client_connection_fd,
                           request_id_count,
                           &my_cache,
                           &my_log);

    new_thread.detach();
    //   new_thread.join();
    request_id_count++;
  }

  //we may never reach the code below because we are in a never-ending while-loop
  // freeaddrinfo(host_info_list);  //free the linedk-list
  // close(socket_fd);
  return 0;
}
