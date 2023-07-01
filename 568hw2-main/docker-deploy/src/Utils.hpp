#ifndef __UTILS__
#define __UTILS__
#include <ctime>
//#include <unordered_map>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <vector>
#include <exception>
#include <string>
using namespace std;


class proxy_exception : public std::exception{

  const char * err_msg;
   public:
  proxy_exception(const char * msg) : err_msg(msg) {}
  proxy_exception(const std::string & msg) : err_msg(msg.c_str()) {}
  const char * what() const throw() { return err_msg; }
};


vector<string> split(string s, string delimiter);
string toLower(string data);
//remove all whitespaces before characters
std::string ltrim(const std::string & s);
bool isHeadEnd(vector<char> & buffer);
time_t strToTime(string strTime);
string vector_char_to_string(vector<char> & vec);
#endif
