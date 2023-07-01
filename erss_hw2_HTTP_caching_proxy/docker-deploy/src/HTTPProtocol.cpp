#include "HTTPProtocol.hpp"

HTTPProtocol::HTTPProtocol(int socket) {
  int n = 0;
  vector<char> buffer;
  int bodyLength = 0;
  do {
    char tempBuffer;
    n = read(socket, &tempBuffer, 1);
    buffer.push_back(tempBuffer);
    if (tempBuffer == '\n') {
      if (isHeadEnd(buffer)) {
        string strBuffer(buffer.begin(), buffer.end());
        //creat HTTPRequest or HTTPResponse by passing headers
        createHTTPProtocol(strBuffer);
        if (hasHeadKey("content-length")) {
          string strLoopCount = getHeadValueByKey("content-length");
          bodyLength = stoi(strLoopCount);
        }
        else {
          bodyLength = 0;
        }
        break;
      }
    }
    //erro handle here
  } while (n != 0);
  //set HTTPProtocol body
  if (bodyLength != 0) {
    vector<char> newBody(bodyLength);
    read(socket, newBody.data(), bodyLength);
    setBody(newBody);
  } else if (hasHeadKey("transfer-encoding")) {
    //cout << "test !!" << getHeadValueByKey("transfer-encoding")<<"::" << endl;
    if (getHeadValueByKey("transfer-encoding") == string("chunked")) {
      storeBodyWhenChunk(socket);
    }
  }
  //set rawData
  if (body.size() == 0) {
    this->rawData = buffer;
  }
  else {
    this->rawData.reserve(buffer.size() + body.size());
    this->rawData.insert(rawData.end(), buffer.begin(), buffer.end());
    this->rawData.insert(rawData.end(), body.begin(), body.end());
  }
}

void HTTPProtocol::storeBodyWhenChunk(int socket) {
  int n = 0;
  vector<char> buffer;
  int contentLength = 1;
  vector<char> lengthLine;
  //cout << "test !!storeBodyWhenChunk::" << endl;
  do {
    char tempBuffer;
    n = read(socket, &tempBuffer, 1);
    buffer.push_back(tempBuffer);
    lengthLine.push_back(tempBuffer);
    int bufferLength = buffer.size();
    //cout << tempBuffer << endl;
    if (tempBuffer == '\n' && buffer[bufferLength - 2] == '\r') {
      string lenStr(lengthLine.begin(), lengthLine.end());
      contentLength = strtol(lenStr.c_str(), NULL, 16);
      //cout << "test !!read one part::" << contentLength << endl;
      int  count = contentLength + 2;
      char temp;
      while (count > 0) {
        n = read(socket, &temp, 1);
        buffer.push_back(temp);
        count--;
      }
      lengthLine.clear();
    }
  } while (contentLength != 0);
  setBody(buffer);
}


void HTTPProtocol::createHTTPProtocol(string strHTTPProtocol) {
  vector<string> headersStrings = split(strHTTPProtocol, "\r\n");
  int lengthHeaders = headersStrings.size();
  this->firstLine = headersStrings[0];
  for (int i = 1; i < lengthHeaders; i++) {
    string head = toLower(headersStrings[i]);
    int index = head.find(':');
    if (index == string::npos) {
      continue;
    }
    string key = head.substr(0, index);
    headers[key] = ltrim(head.substr(index + 1));
  }
}

void HTTPProtocol::setBody(vector<char> reBody) {
  this->body = reBody;
}

string HTTPProtocol::getHeadValueByKey(string key) {
  if (hasHeadKey(toLower(key))) {
    return headers[toLower(key)];
  }
  return "";
}

bool HTTPProtocol::hasHeadKey(string str) {
  unordered_map<string, string>::const_iterator it = this->headers.find(toLower(str));
  return it != this->headers.end();
}

vector<char> HTTPProtocol::getRawData() {
  return this->rawData;
}
void HTTPProtocol::reviseRawdata(vector<char> & vec) {
  this->rawData = vec;
}

string HTTPProtocol::getFirstLine() {
  return this->firstLine;
}

HTTPRequest::HTTPRequest(int socket) : HTTPProtocol(socket) {
  vector<string> startLine = split(firstLine, " ");
  if(startLine.size()>=3){//hao added! 02/23
  this->method = startLine[0];
  this->requestTarget = startLine[1];
  this->HTTPVersion = startLine[2];
  }//hao added! 02/23
  
  //set hostName and port number
  //if has host key
  if (hasHeadKey("host")) {
    string host = this->getHeadValueByKey("host");
    if (host.find(':') == string::npos) {
      this->hostName = host;
      this->port = 0;
    }
    else {
      int indexSeperateHost = host.find(':');
      this->hostName = host.substr(0, indexSeperateHost);
      string portStr = host.substr(indexSeperateHost + 1);
      this->port = stoi(portStr);
    }
  }  //if headers don't have host, get hostName from request target(isn't http)
  else if (this->requestTarget[0] != 'h') {
    //requestTarget don't have port number
    if (requestTarget.find(':') == string::npos) {
      int spaceSeperate = requestTarget.find(' ');
      this->hostName = requestTarget.substr(0, spaceSeperate);
      this->port = 0;
    }
    else {
      int colonSeperate = requestTarget.find(':');
      this->hostName = requestTarget.substr(0, colonSeperate);
      int spaceSeperate = requestTarget.find(' ');
      string portStr = requestTarget.substr(colonSeperate + 1, spaceSeperate);
      this->port = stoi(portStr);
    }
  }  // if headers don't have host, get hostName from request target(is http)
  else {
    string reTargetWithoutH = this->requestTarget.substr(7);
    int slashSeperate = reTargetWithoutH.find('/');
    if (reTargetWithoutH.find(':') == string::npos) {
      this->hostName = reTargetWithoutH.substr(0, slashSeperate);
      this->port = 0;
    }
    else {
      int colonSeperate = requestTarget.find(':');
      this->hostName = reTargetWithoutH.substr(0, colonSeperate);
      this->port = stoi(reTargetWithoutH.substr(colonSeperate + 1, slashSeperate));
    }
  }
  if (this->port == 0) {
    if (this->method == "connect") {
      this->port = 443;
    }
    else {
      this->port = 80;
    }
  }
}

HTTPRequest::HTTPRequest(const HTTPRequest & r) : HTTPProtocol(r) {
  this->hostName = r.hostName;
  this->port = r.port;
  this->method = r.method;
  this->requestTarget = r.requestTarget;
  this->HTTPVersion = r.HTTPVersion;
  this->headers = r.headers;
  this->body = r.body;
  this->rawData = r.rawData;
}

HTTPRequest & HTTPRequest::operator=(const HTTPRequest & r) {
  this->hostName = r.hostName;
  this->port = r.port;
  this->method = r.method;
  this->requestTarget = r.requestTarget;
  this->HTTPVersion = r.HTTPVersion;
  this->headers = r.headers;
  this->body = r.body;
  this->rawData = r.rawData;
  return *this;
}

string HTTPRequest::getURL() {
  string reTarget = this->requestTarget;
  if (reTarget[0] == '/') {
    string host = headers["host"];
    return host + reTarget;
  }
  return reTarget;
}

string HTTPRequest::getHostName() {
  return this->hostName;
}

int HTTPRequest::getPort() {
  return this->port;
}

// return GET / POST / CONNECT
string HTTPRequest::getMethod() {
  return this->method;
}

HTTPResponse::HTTPResponse(int socket) : HTTPProtocol(socket) {
  vector<string> headersStrings = split(firstLine, " ");
  this->statusCode = headersStrings[1];
  if (hasHeadKey("date")) {
    string strDate = headers["date"];
    this->date = strToTime(strDate);
  }
  else {
    this->date = time(NULL);
  }
}

//copy constructor.
HTTPResponse::HTTPResponse(const HTTPResponse & r) : HTTPProtocol(r) {
  this->headers = r.headers;
  this->body = r.body;
  this->rawData = r.rawData;
  this->firstLine = r.firstLine;
  this->statusCode = r.statusCode;
  this->date = r.date;
}
// = operator
HTTPResponse & HTTPResponse::operator=(const HTTPResponse & r) {
  this->headers = r.headers;
  this->body = r.body;
  this->rawData = r.rawData;
  this->firstLine = r.firstLine;
  this->statusCode = r.statusCode;
  this->date = r.date;
  return *this;
}
time_t HTTPResponse::getResponseDate() {
  return this->date;
}
string HTTPResponse::getStatusCode() {
  return statusCode;
}

void HTTPResponse::setDate(time_t newDate) {
  //haven't finished!
  this->date = newDate;
}

bool HTTPResponse::has_Etag() {
  return headers.find("etag") != headers.end();
}

bool HTTPResponse::has_last_modified() {
  return headers.find("last-modified") != headers.end();
}
