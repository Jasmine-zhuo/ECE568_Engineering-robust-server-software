//
// Created by Xinyu Guo on 3/22/22.
//

#include "XMLResponse.h"

  //member functs for 'create' type requests
void XMLResponse::created_id(string account_id_str){
    xml_node<>* child = doc.allocate_node(node_element,"created");
    char * account_id = doc.allocate_string(account_id_str.c_str());
    child->append_attribute(doc.allocate_attribute("id",account_id));
    root->append_node(child);
}

void XMLResponse::created_sym(string sym_name_str, string account_id_str){
    xml_node<>* child = doc.allocate_node(node_element,"created");
    char * sym_name = doc.allocate_string(sym_name_str.c_str());
    char * account_id = doc.allocate_string(account_id_str.c_str());
    child->append_attribute(doc.allocate_attribute("sym",sym_name));
    child->append_attribute(doc.allocate_attribute("id",account_id));
    root->append_node(child);
}

void XMLResponse::error_id(string account_id_str, string error_msg){
    char * msg = doc.allocate_string(error_msg.c_str());
    xml_node<>* child = doc.allocate_node(node_element,"error",msg);
    char * account_id = doc.allocate_string(account_id_str.c_str());
    child->append_attribute(doc.allocate_attribute("id",account_id));
    root->append_node(child);
}

void XMLResponse::error_sym(string sym_name_str,string account_id_str, string error_msg){
    char * msg = doc.allocate_string(error_msg.c_str());
    xml_node<>* child = doc.allocate_node(node_element,"error",msg);
    char * sym_name = doc.allocate_string(sym_name_str.c_str());
    child->append_attribute(doc.allocate_attribute("sym",sym_name));
    char * account_id = doc.allocate_string(account_id_str.c_str());
    child->append_attribute(doc.allocate_attribute("id",account_id));
    root->append_node(child);  
}

//member functs for 'transaction' type requests
void XMLResponse::opened_order(string sym_name_str,string amount_str, string limit_str, string transaction_id_str){
    xml_node<>* child = doc.allocate_node(node_element,"opened");
    char * sym_name = doc.allocate_string(sym_name_str.c_str());
    char * amount = doc.allocate_string(amount_str.c_str());
    char * limit = doc.allocate_string(limit_str.c_str());
    char * transaction_id = doc.allocate_string(transaction_id_str.c_str()); 
    child->append_attribute(doc.allocate_attribute("sym",sym_name));
    child->append_attribute(doc.allocate_attribute("amount",amount));
    child->append_attribute(doc.allocate_attribute("limit",limit));
    child->append_attribute(doc.allocate_attribute("id",transaction_id));
    root->append_node(child);
}

void XMLResponse::error_order(string sym_name_str,string amount_str, string limit_str,string error_msg){
     char * msg = doc.allocate_string(error_msg.c_str());
     xml_node<>* child = doc.allocate_node(node_element,"error",msg);
     char * sym_name = doc.allocate_string(sym_name_str.c_str());
     char * amount = doc.allocate_string(amount_str.c_str());
     char * limit = doc.allocate_string(limit_str.c_str());
     child->append_attribute(doc.allocate_attribute("sym",sym_name));
     child->append_attribute(doc.allocate_attribute("amount",amount));
     child->append_attribute(doc.allocate_attribute("limit",limit));
     root->append_node(child);
}

void XMLResponse::error_query_or_cancel(string transaction_id_str,string error_msg){
    char * msg = doc.allocate_string(error_msg.c_str());
     xml_node<>* child = doc.allocate_node(node_element,"error",msg);
     char * trans_id = doc.allocate_string(transaction_id_str.c_str());
     
     child->append_attribute(doc.allocate_attribute("transaction_id",trans_id));
     root->append_node(child);
}


void XMLResponse::open_shares(string shares_str){
    xml_node<>* child = doc.allocate_node(node_element,"open");
    char * shares = doc.allocate_string(shares_str.c_str());
    child->append_attribute(doc.allocate_attribute("shares",shares));
    cur_root_node->append_node(child);
}

void XMLResponse::canceled_shares(string shares_str,string time_str){
    xml_node<>* child = doc.allocate_node(node_element,"canceled");
    char * shares = doc.allocate_string(shares_str.c_str());
    child->append_attribute(doc.allocate_attribute("shares",shares));
    char * times = doc.allocate_string(time_str.c_str());
    child->append_attribute(doc.allocate_attribute("time",times));
    cur_root_node->append_node(child);
}

void XMLResponse::exec_shares(string shares_str,string price_str,string time_str){
    xml_node<>* child = doc.allocate_node(node_element,"executed");
    char * shares = doc.allocate_string(shares_str.c_str());
    char * price = doc.allocate_string(price_str.c_str());
    char * time = doc.allocate_string(time_str.c_str());
    child->append_attribute(doc.allocate_attribute("shares",shares));
    child->append_attribute(doc.allocate_attribute("price",price));
    child->append_attribute(doc.allocate_attribute("time",time));
    cur_root_node->append_node(child);
}

void XMLResponse::send_back_to_client(){
       //send the entire response to client
      string response_str;
      rapidxml::print(back_inserter(response_str),doc);
     
      
      string length_added_s = to_string(response_str.length())+"\n"+response_str;
      int n= write(socket_fd,length_added_s.c_str(),length_added_s.length());
    cout<<"the response we are about to send back is :"<<endl;
    cout<<length_added_s<<endl;
      if(n<0){
        cout << "send back response to socket_fd "<< socket_fd <<" failed!\n";
      }
}
