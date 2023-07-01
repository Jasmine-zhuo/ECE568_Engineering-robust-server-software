#ifndef HW4_DATABASE_H
#define HW4_DATABASE_H
#include <iostream>
#include <pqxx/pqxx>
#include <string>
#include <ctime>
#include <fstream>
#include <utility>
#include <cassert>
#include "XMLResponse.h"
#include "myException.h"

using namespace std;
using namespace pqxx;

typedef struct execution_info_tag{
    string sym_name;
    double execution_amount;
    double execution_price;
}execution_info_t;

typedef struct order_info_tag{
    string ID ;
    string transaction_id;
    string account_id;
    string type;
    string sym_name;
    double limit_price;
    double amount;
}order_info_t;

class StockDatabase{
private:
    connection *C;
    const static pqxx::isolation_level isolation = pqxx::isolation_level::serializable;

    // setup database
    void executeSQL(const string & sql);
    void deleteAllTables();
    void createTables();
    void initializeDatabase();
    void setupDatabase();

    // insert functions
    void insertIntoAccounts(string account_id, string balance, transaction<isolation> &T);
    void insertIntoPosition(string account_id, string sym_name, string position_amount, transaction<isolation> &T);
    void insertNewOrder(string account_id, string sym_name, string order_amount, string limit_price, order_info_t &insert_order_info, transaction<isolation> &T);
    void insertSplitOrder(string trans_id, string account_id, string sym_name, string order_amount, string limit_price, string order_type, transaction<isolation> &T);

    // update functions
    void updatePositionAmount(string account_id, string sym_name, double position_amount, string operation, transaction<isolation> &T);
    void updateOrderAmount(string order_id, double amount, string operation, transaction<isolation> &T);
    void updateOrderStatus(string order_id, string status, double limit_price, transaction<isolation> &T);
    void updateAccountBalance(string account_id, double money, string operation, transaction<isolation> &T);
    void deleteOrder(string order_id, transaction<isolation> &T);

    // query functions
    bool isAccountExist(string account_id, transaction<isolation> &T);
    bool isSymbolExistForId(int account_id, string sym_name, transaction<isolation> &T);
    bool isSellOrderValid(string account_id, string sym_name, double order_num, transaction<isolation> &T);
    bool isBuyOrderValid(double balance, double amount, double limit);

    // get function
    result getTransInfo(string account_id_str, string transaction_id);
    double getAccountBalance(string account_id, transaction<isolation> &T);

    // match function
    void matchOrders(string sym_name, order_info_t curr_order_info, transaction<isolation> &T);
    void updateTwoMatchedOrders(execution_info_t execution_info, order_info_t partially_executed_order_info, order_info_t completely_executed_order_info, bool executed_partially,
                                               transaction<isolation> &T);
    void executeOrder(execution_info_t execution_info, order_info_t partially_executed_order_info, order_info_t completely_executed_order_info, bool executed_partially,
                                     transaction<isolation> &T);
    // cancel function
    bool isCancelOperationValid(string account_id_str, string transaction_id, order_info_t &valid_order_info, transaction<isolation> &T);
    void performCancelOperation(order_info_t cancelled_order_info, transaction<isolation> &T);

public:
    StockDatabase();
    StockDatabase(bool isInitializeDB);
    ~StockDatabase();
    // API for <create> tag
    void createAccount(string account_id_str, string balance_str, XMLResponse &response);
    void placeSymbol(string sym_name_str, vector<pair<string, string>> sym_info_list, XMLResponse &response);
    // API for <transaction> tag
    void processOrder(string account_id_str, string sym_name_str,
                      string order_amount_str, string limit_price_str, XMLResponse &response);
    void processQuery(string account_id_str, string transaction_id_str, XMLResponse & response);
    void processCancel(string account_id_str, string transaction_id_str, XMLResponse & response);
};


#endif //HW4_DATABASE_H
