#ifndef HW4_MYEXCEPTION_H
#define HW4_MYEXCEPTION_H

class server_exception : public std::exception{
private:
    const char * err_msg;
public:
    server_exception(const char * msg) : err_msg(msg){}
    server_exception(const std::string & msg) : err_msg(msg.c_str()){}
    const char * what() const throw() {return err_msg;}
};

class order_exception : public std::exception{
private:
    const char * err_msg;
public:
    order_exception(const char * msg) : err_msg(msg){}
    order_exception(const std::string & msg) : err_msg(msg.c_str()){}
    const char * what() const throw() {return err_msg;}
};
#endif //HW4_MYEXCEPTION_H
