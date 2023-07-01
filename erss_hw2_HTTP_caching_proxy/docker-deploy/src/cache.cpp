#include "cache.hpp"

#include <time.h>

#include <cstdlib>
//transfer request to a key
string Cache::generateKeyFromRequest(HTTPRequest request) {
  return "1" + request.getMethod() + "2" + request.getURL() + "3" +
         request.getHostName() + to_string(request.getPort());
}
//check whether there is cache by request
bool Cache::findCache(HTTPRequest request) {
  string key = generateKeyFromRequest(request);
  return cachemap.find(key) != cachemap.end();
}
void Cache::addCache(HTTPRequest request, HTTPResponse response) {
  string key = generateKeyFromRequest(request);
  cachemap[key] = response;
}
// check if the string has given word
bool Cache::hasWord(string str, string word) {
  return str.find(word) != string::npos;
}
//find value matched with word in string
int Cache::findWordValue(string str, string word) {
  mylock.lock();
  size_t indexOfWord = str.find(word);
  size_t indexOfValue = indexOfWord + word.length() + 1;
  string valueStr = str.substr(indexOfValue);
  int value = atoi(valueStr.c_str());
  mylock.unlock();
  return value;
}
//check if time is out
bool Cache::isValidTime(time_t date, int timeAge) {
  return timeAge + date > time(NULL);
}
//transfer string to time_t
time_t Cache::strToTime(string strTime) {
  struct tm time;
  strptime(strTime.c_str(), "%a, %d %b %Y %H:%M:%S %Z", &time);
  return mktime(&time);
}

//check whether response is valide,  check time is expired
bool Cache::isValidResponse(HTTPResponse response) {
  if (response.hasHeadKey("cache-control")) {
    string catch_control = response.getHeadValueByKey("cache-control");
    if (hasWord(catch_control, "no-cache")) {
      return false;
    }
    if (hasWord(catch_control, "s-maxage")) {
      int timeAge = findWordValue(catch_control, "s-maxage");
      time_t date = response.getResponseDate();
      return isValidTime(date, timeAge);
    }
    if (hasWord(catch_control, "max-age")) {
      int timeAge = findWordValue(catch_control, "max-age");
      time_t date = response.getResponseDate();
      return isValidTime(date, timeAge);
    }
  }
  if (response.hasHeadKey("expires")) {
    string expireTime = response.getHeadValueByKey("expires");
    time_t expireTime_t = strToTime(expireTime);
    return expireTime_t > time(NULL);
  }
  return false;
}

void Cache::modifyCache(HTTPRequest request, HTTPResponse response) {
  mylock.lock();
  addCache(request, response);
  mylock.unlock();
}

void Cache::deleteCache(HTTPRequest request, HTTPResponse response) {
  string key = generateKeyFromRequest(request);
  cachemap.erase(key);
}
//get response by request
HTTPResponse Cache::getResponse(HTTPRequest request) {
  string key = generateKeyFromRequest(request);
  return cachemap[key];
}
void Cache::renewDateofResponse(HTTPRequest request) {
  mylock.lock();
  string key = generateKeyFromRequest(request);
  time_t newdate = time(NULL);
  cachemap[key].setDate(newdate);
  mylock.unlock();
}
