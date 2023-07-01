//
// Created by Xinyu Guo on 3/22/22.
//

#ifndef HW4_XMLRESPONSE_H
#define HW4_XMLRESPONSE_H

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include "rapidxml.hpp"
#include "rapidxml_print.hpp"
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
using namespace std;
using namespace rapidxml;

class XMLResponse {
public:
      xml_node<> * cur_root_node;
      xml_node<> * root;
      xml_document<> doc;
      int socket_fd;

    //member functs for 'create' type requests
    void  created_id(string account_id_str);
    void  created_sym(string sym_name_str, string account_id_str);
    void  error_id(string account_id_str, string error_msg);
    void  error_sym(string sym_name_str,string account_id_str, string error_msg);

    //member functs for 'transaction' type requests
    void  opened_order(string sym_name_str,string amount_str, string limit_str, string transaction_id_str);
    void  error_order(string sym_name_str,string amount_str, string limit_str,string error_msg);
    void  error_query_or_cancel(string transaction_id_str,string error_msg);//transaction id invalid!

    void open_shares(string shares_str);
    void canceled_shares(string shares_str,string time_str);
    void exec_shares(string shares_str, string price_str, string time_str);
    void send_back_to_client();
};


#endif //HW4_XMLRESPONSE_H
