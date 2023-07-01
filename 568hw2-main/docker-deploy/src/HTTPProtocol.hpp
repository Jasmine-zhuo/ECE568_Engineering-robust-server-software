#ifndef __XINXIN_HTTP_PROTOCOL__
#define __XINXIN_HTTP_PROTOCOL__
#include <netdb.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <functional>
#include <iostream>
#include <unordered_map>

#include "Utils.hpp"
using namespace std;

class HTTPProtocol {
 protected:
  unordered_map<string, string> headers;
  vector<char> body;
  vector<char> rawData;
  string firstLine;

 public:
  HTTPProtocol(){};
  HTTPProtocol(int socket);
  void createHTTPProtocol(string strHTTPProtocol);
  void setBody(vector<char> reBody);
  string getHeadValueByKey(string key);
  bool hasHeadKey(string str);
  vector<char> getRawData();
  string getFirstLine();
  void reviseRawdata(vector<char> & vec);
  void storeBodyWhenChunk(int socket);
};

class HTTPRequest : public HTTPProtocol {
 private:
  string hostName;
  int port;
  string method;
  string requestTarget;
  string HTTPVersion;

 public:
  HTTPRequest() : HTTPProtocol(){};
  HTTPRequest(int socket);
  //copy constructor.
  HTTPRequest(const HTTPRequest & r);
  // = operator
  HTTPRequest & operator=(const HTTPRequest & r);
  string getURL();
  string getHostName();
  int getPort();
  // return GET / POST / CONNECT
  string getMethod();
};

class HTTPResponse : public HTTPProtocol {
 private:
  string statusCode;
  time_t date;

 public:
  HTTPResponse() : HTTPProtocol(){};
  HTTPResponse(int socket);
  //copy constructor.
  HTTPResponse(const HTTPResponse & r);
  // = operator
  HTTPResponse & operator=(const HTTPResponse & r);
  time_t getResponseDate();
  string getStatusCode();
  void setDate(time_t newDate);
  bool has_Etag();
  bool has_last_modified();
};

#endif
