#include "Utils.hpp"

string vector_char_to_string(vector<char> & vec) {
  string s;
  for (char c : vec) {
    s.push_back(c);
  }
  return s;
}

vector<string> split(string s, string delimiter) {
  size_t pos_start = 0, pos_end, delim_len = delimiter.length();
  string token;
  vector<string> res;

  while ((pos_end = s.find(delimiter, pos_start)) != string::npos) {
    token = s.substr(pos_start, pos_end - pos_start);
    pos_start = pos_end + delim_len;
    res.push_back(token);
  }
  res.push_back(s.substr(pos_start));
  return res;
}

string toLower(string data) {
  transform(data.begin(), data.end(), data.begin(), [](unsigned char c) {
    return std::tolower(c);
  });
  return data;
}

//remove all whitespaces before characters
std::string ltrim(const std::string & s) {
  size_t start = s.find_first_not_of(" ");
  return (start == std::string::npos) ? "" : s.substr(start);
}

bool isHeadEnd(vector<char> & buffer) {
  int headLength = buffer.size();
  if (headLength < 4) {
    return false;
  }
  if (buffer[headLength - 1] == '\n' && buffer[headLength - 2] == '\r' &&
      buffer[headLength - 3] == '\n' && buffer[headLength - 4] == '\r') {
    return true;
  }
  return false;
}

time_t strToTime(string strTime) {
  struct tm time;
  strptime(strTime.c_str(), "%a, %d %b %Y %H:%M:%S %Z", &time);
  return mktime(&time);
}
