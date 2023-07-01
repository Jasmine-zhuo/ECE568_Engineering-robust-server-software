#ifndef __XINXIN_CACHE__
#define __XINXIN_CACHE__
#include <ctime>
#include <string>
#include <unordered_map>
#include <mutex>
#include "HTTPProtocol.hpp"
using namespace std;

class Cache {
 private:
  mutex mylock;
  unordered_map<string, HTTPResponse> cachemap;

 public:
  //transfer request to a key
  string generateKeyFromRequest(HTTPRequest request);
  //check whether there is cache by request
  bool findCache(HTTPRequest request);
  void addCache(HTTPRequest request, HTTPResponse response);
  // check if the string has given word
  bool hasWord(string str, string word);
  //find value matched with word in string
  int findWordValue(string str, string word);
  //check if time is out
  bool isValidTime(time_t date, int timeAge);
  //transfer string to time_t
  time_t strToTime(string strTime);
  //check whether response is valide,  check time is expired
  bool isValidResponse(HTTPResponse response);
  void modifyCache(HTTPRequest request, HTTPResponse response);
  void deleteCache(HTTPRequest request, HTTPResponse response);
  //get response by request
  HTTPResponse getResponse(HTTPRequest request);
  void renewDateofResponse(HTTPRequest request);
};

#endif
