//This is the header file for the proxy log!
#include <fstream>   //for writing to file!
#include <ios>       //for using the append mode when opening the log file
#include <iostream>  //for cout
#include <mutex>     //for lock!
#include <string>

using namespace std;
class log {
  std::fstream log_file;
  mutex mylock;  //a log object only has one lock!So this guarantees thread-safe
 public:

  void initial_log(int id,
                   string REQUEST,
                   string IP_FROM,
                   string TIME);  //open the log file and write the first line to it

  void GET_result_log(

      int id,
      int result,
      string EXPIREDTIME);
  //if the request is a GET request, check the cache and print to log based on the result
  //result=1 for not in cache, =2 for cached yet expired, =3 for in cache & needs validation, =4 for in cache & valid

  void request_to_origin_server_log(int id, string REQUEST, string SERVER);
  void response_from_origin_server_log(int id, string RESPONSE, string SERVER);

  void GET_OK_log(int id, int result, string REASON, string EXPIRES);

  //result = 1 for not cachable because of some reason, = 2 for cached and expires at some expire time, =3 for cached but requires re-validation

  void response_to_client_log(int id, string RESPONSE);

  void tunnel_close_log(int id);

  void note_log(int id, string MESSAGE);

  void warning_log(int id, string MESSAGE);

  void error_log(int id, string MESSAGE);
};
