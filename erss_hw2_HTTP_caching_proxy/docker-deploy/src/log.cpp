//This is the source file for the proxy log!
#include "log.hpp"

#include <fstream>   //for writing to file!
#include <ios>       //for using the append mode when opening the log file
#include <iostream>  //for cout
#include <mutex>
#include <string>
using namespace std;
std::string log_directory = "/var/log/erss/proxy.log";

void log::initial_log(int id,
                      string REQUEST,
                      string IP_FROM,
                      string TIME) {  //open the log file and write the first line to it
  mylock.lock();
  log_file.open(
      log_directory,
      fstream::
          app);  //append mode: add output to end of existing file; prevents overwriting!
  //!!!!!!!!need to add error handling code!
  log_file << id << ": ";
  log_file << "\"" << REQUEST << "\""
           << " from ";
  /////
  log_file << IP_FROM << " @ ";
  log_file << TIME;
  log_file << endl;
  log_file.close();
  mylock.unlock();
}
void log::GET_result_log(
    int id,
    int result,
    string
        EXPIREDTIME) {  //if the request is a GET request, check the cache and print to log based on the result
  //result=1 for not in cache, =2 for cached yet expired, =3 for in cache & needs validation, =4 for in cache & valid
    mylock.lock();
  log_file.open(log_directory, fstream::app);
  //need to add error handling code!!!
  if (result < 1 || result > 4) {  //invalid result identifier! throw exception!!!!!!!!!
    ////*******************need to add error handle code***************
  }
  log_file << id << ": ";  //all four kinds of results need this!
  switch (result) {
    case 1:
      log_file << "not in cache" << endl;
      break;
    case 2:
      log_file << "in cache, but expired at " << EXPIREDTIME << endl;
      break;
    case 3:
      log_file << "in cache, requires validation" << endl;
      break;
    case 4:
      log_file << "in cache, valid" << endl;
  }
  log_file.close();
    mylock.unlock();
}

void log::request_to_origin_server_log(int id, string REQUEST, string SERVER) {
  mylock.lock();
  log_file.open(log_directory, fstream::app);
  //add error handling code!
  log_file << id << ": Requesting "
           << "\"" << REQUEST << "\""
           << " from " << SERVER << endl;

  log_file.close();
    mylock.unlock();
}
void log::response_from_origin_server_log(int id, string RESPONSE, string SERVER) {
  mylock.lock();
  log_file.open(log_directory, fstream::app);
  log_file << id << ": Received "
           << "\"" << RESPONSE << "\""
           << " from " << SERVER << endl;
  log_file.close();
    mylock.unlock();
}
void log::GET_OK_log(int id, int result, string REASON, string EXPIRES) {
  //result = 1 for not cachable because of some reason, = 2 for cached and expires at some expire time, =3 for cached but requires re-validation
    mylock.lock();
  log_file.open(log_directory, fstream::app);
  //add error handling code!
  log_file << id << ": ";
  if (result < 1 || result > 3) {
    //error handling: invalid result!
  }
  switch (result) {
    case 1:
      log_file << "not cacheable because " << REASON << endl;
      break;
    case 2:
      log_file << "cached, expires at " << EXPIRES << endl;
      break;
    case 3:
      log_file << "cached, but requires re-validation" << endl;
  }
  log_file.close();
    mylock.unlock();
}
void log::response_to_client_log(int id, string RESPONSE) {
  mylock.lock();
  log_file.open(log_directory, fstream::app);
  //add error handling code
  log_file << id << ": Responding "
           << "\"" << RESPONSE << "\"" << endl;
  log_file.close();
    mylock.unlock();
}
void log::tunnel_close_log(int id) {
  mylock.lock();
  log_file.open(log_directory, fstream::app);
  //add error handling code
  log_file << id << ": Tunnel closed" << endl;
  log_file.close();
  mylock.unlock();  
}
void log::note_log(int id, string MESSAGE) {
  mylock.lock();
  log_file.open(log_directory, fstream::app);
  //add error handling code
  log_file << id << ": NOTE " << MESSAGE << endl;
  log_file.close();
  mylock.unlock();
}
void log::warning_log(int id, string MESSAGE) {
  mylock.lock();
  log_file.open(log_directory, fstream::app);
  //add error handling code
  log_file << id << ": WARNING " << MESSAGE << endl;
  log_file.close();
  mylock.unlock();
}
void log::error_log(int id, string MESSAGE) {
  mylock.lock();
  log_file.open(log_directory, fstream::app);
  //add error handling code
  log_file << id << ": ERROR " << MESSAGE << endl;
  log_file.close();
  mylock.unlock();
}

/*
int main(void) {  //////////**************JUST FOR TEST USE************************////
/*
log mylog;
mylog.initial_log(104,
                  "GET www.bbc.co.uk/ HTTP/1.1",
                  "1.2.3.4",
                  "Sun Jan 1 22:58:17 2022");
mylog.GET_result_log(105, 1, "Sun Jan 1 22:58:17 2022");
mylog.GET_result_log(106, 2, "Sun Jan 1 22:58:17 2022");
mylog.GET_result_log(107, 3, "Sun Jan 1 22:58:17 2022");
mylog.GET_result_log(108, 4, "Sun Jan 1 22:58:17 2022");
mylog.request_to_origin_server_log(109, "GET www.bbc.co.uk/ HTTP/1.1", "www.bbc.co.uk");
mylog.response_from_origin_server_log(110, "HTTP/1.1 200 OK", "www.bbc.co.uk");
mylog.GET_OK_log(111, 1, "some not cacheable reason", "");
mylog.GET_OK_log(112, 2, "", "Sun Jan 1 22:58:10 2023");
mylog.GET_OK_log(113, 3, "", "");
mylog.response_to_client_log(114, "HTTP/1.1 200 OK");
mylog.tunnel_close_log(115);
mylog.note_log(116, "Cache-Control:must revalidate");
mylog.warning_log(117, "This is a warining");
mylog.error_log(118, "This is an error");
return EXIT_SUCCESS;
}
* /


*/
