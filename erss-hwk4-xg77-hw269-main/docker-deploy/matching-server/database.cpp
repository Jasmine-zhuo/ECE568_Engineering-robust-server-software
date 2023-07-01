#include "database.h"

void StockDatabase::executeSQL(const string & sql){
    try{
        work W(*this->C);
        W.exec(sql);
        W.commit();
    }
    catch (const std::exception &e){
        cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
}


void StockDatabase::deleteAllTables(){
    try{
        string delete_tbs_sql = "DROP TABLE IF EXISTS ACCOUNTS, POSITIONS CASCADE;";
        delete_tbs_sql += "DROP TYPE IF EXISTS STATUS_T CASCADE;";
        delete_tbs_sql += "DROP TYPE IF EXISTS TYPE_T CASCADE;";
        delete_tbs_sql += "DROP TABLE IF EXISTS ORDERS;";
        executeSQL(delete_tbs_sql);
        cout << "Delete existed tables successfully" << endl;
    }
    catch (const std::exception &e){
        cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
}


void StockDatabase::createTables(){
    // create ACCOUNT table
    string create_account_sql = "CREATE TABLE ACCOUNTS("
                              "ACCOUNT_ID INT PRIMARY KEY NOT NULL,"
                              "BALANCE REAL NOT NULL CHECK (BALANCE > 0));"; // positive
    executeSQL(create_account_sql);

    // create POSITION table
    string create_position_sql = "CREATE TABLE POSITIONS("
                              "ACCOUNT_ID INT NOT NULL,"
                              "SYM_NAME VARCHAR(256) NOT NULL,"
                              "POSITION_AMOUNT REAL NOT NULL CHECK (POSITION_AMOUNT > 0)," // positive
                              "CONSTRAINT POSITION_PK PRIMARY KEY (ACCOUNT_ID, SYM_NAME));";
    executeSQL(create_position_sql);

    // create order status type
    string create_status_type_sql = "CREATE type STATUS_T AS ENUM('open', 'executed', 'cancelled')";
    executeSQL(create_status_type_sql);
    string create_type_type_sql = "CREATE type TYPE_T AS ENUM('sell', 'buy');";
    executeSQL(create_type_type_sql);


    // create ORDERS table
    string create_order_sql = "CREATE TABLE ORDERS("
                             "ID SERIAL PRIMARY KEY,"
                             "TRANS_ID SERIAL NOT NULL,"
                             "ACCOUNT_ID INT NOT NULL,"
                             "SYM_NAME VARCHAR(256) NOT NULL,"
                             "ORDER_AMOUNT REAL NOT NULL," // positive: buy, negative: sell
                             "LIMIT_PRICE REAL NOT NULL,"
                             "TYPE TYPE_T NOT NULL,"
                             "STATUS STATUS_T NOT NULL DEFAULT 'open',"
                             "TIME TIMESTAMP NOT NULL DEFAULT now(),"
                             "CONSTRAINT CHECK_STATUS CHECK (STATUS IN ('open', 'executed', 'cancelled')),"
                             "CONSTRAINT CHECK_TYPE CHECK (TYPE IN ('sell', 'buy')));";
    executeSQL(create_order_sql);

    cout << "creat all tables successfully" << endl;
}


void StockDatabase::initializeDatabase(){
    deleteAllTables();
    createTables();
}


void StockDatabase::setupDatabase(){
    try{
        //Establish a connection to the database
        this->C = new connection("dbname=stock user=postgres password=passw0rd host=stock_db port=5432");
        if (C->is_open()) {
            cout << "Opened database successfully: " << C->dbname() << endl;
        } else {
            std::cout << "Can't open database" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    catch (const std::exception &e){
        cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }

}

/**********************************************************************************
 *                            Insert/update/delete Functions                      *
 **********************************************************************************/
void StockDatabase::insertIntoAccounts(string account_id, string balance, transaction<isolation> &T) {
    string sql = "INSERT INTO ACCOUNTS(ACCOUNT_ID, BALANCE) "
                 "VALUES (" + account_id + ", " + balance + ");";
    T.exec(sql);
}

void StockDatabase::insertIntoPosition(string account_id, string sym_name, string position_amount,
                                       transaction<isolation> &T){
    string sql = "INSERT INTO POSITIONS(ACCOUNT_ID, SYM_NAME, POSITION_AMOUNT) "
                 "VALUES (" + account_id + ", '" + sym_name + + "', " + position_amount + ") " +
                 "ON CONFLICT ON CONSTRAINT POSITION_PK DO UPDATE SET POSITION_AMOUNT = "
                 "POSITIONS.POSITION_AMOUNT + EXCLUDED.POSITION_AMOUNT;";
    T.exec(sql);
}

void StockDatabase::insertNewOrder(string account_id, string sym_name, string order_amount, string limit_price,
                                   order_info_t &insert_order_info, transaction<isolation> &T){
    string insert_sql = "INSERT INTO ORDERS(ACCOUNT_ID, SYM_NAME, ORDER_AMOUNT, LIMIT_PRICE, TYPE, STATUS) "
                 "VALUES (" + account_id + ", '" + sym_name + + "', " + order_amount + ", "
                 + limit_price + ", 'buy', 'open') RETURNING ID;";
    result R_insert(T.exec(insert_sql));

    string db_order_id = R_insert[0]["ID"].as<string>();
    string return_sql = "SELECT * FROM ORDERS WHERE ID = " + db_order_id + ";";
    result R(T.exec(return_sql));

    insert_order_info.ID = R[0]["ID"].as<string>();
    insert_order_info.transaction_id = R[0]["TRANS_ID"].as<string>();
    insert_order_info.account_id = R[0]["ACCOUNT_ID"].as<string>();
    insert_order_info.sym_name = R[0]["SYM_NAME"].as<string>();
    insert_order_info.amount = R[0]["ORDER_AMOUNT"].as<double>();
    insert_order_info.limit_price = R[0]["LIMIT_PRICE"].as<double>();
    insert_order_info.type = R[0]["TYPE"].as<string>();

    // update order_amount to abs value and set the order type
    if(insert_order_info.amount < 0){
        // update local order information
        insert_order_info.amount = abs(insert_order_info.amount);
        insert_order_info.type = "sell";

        // update database order information
        string update_sql = "UPDATE ORDERS SET ORDER_AMOUNT = " + to_string(insert_order_info.amount)
                            + ", TYPE = 'sell' WHERE ID = " + insert_order_info.ID + ";";
        T.exec(update_sql);
    }
    if(insert_order_info.amount == 0){
        throw order_exception("the amount of order is zero");
    }
}

void StockDatabase::insertSplitOrder(string trans_id, string account_id, string sym_name, string order_amount,
                                     string limit_price, string order_type, transaction<isolation> &T){
    string insert_sql = "INSERT INTO ORDERS(TRANS_ID, ACCOUNT_ID, SYM_NAME, ORDER_AMOUNT, LIMIT_PRICE, TYPE, STATUS) "
                        "VALUES (" + trans_id + ", " + account_id + ", '" + sym_name + + "', " + order_amount + ", "
                        + limit_price + ", '" + order_type + "', 'executed');";
    result R(T.exec(insert_sql));
}

void StockDatabase::updatePositionAmount(string account_id, string sym_name, double position_amount,
                                         string operation, transaction<isolation> &T){
    string sql = "UPDATE POSITIONS SET POSITION_AMOUNT = POSITION_AMOUNT " +  operation + " "
                + to_string(position_amount) + " WHERE ACCOUNT_ID = "
                + account_id + " AND SYM_NAME = '" + sym_name + "';";
    T.exec(sql);
}

void StockDatabase::updateOrderAmount(string order_id, double amount, string operation, transaction<isolation> &T){
    string sql = "UPDATE ORDERS "
                 "SET ORDER_AMOUNT = ORDER_AMOUNT " +  operation + " " + to_string(amount) + " WHERE ID = " + order_id + ";";
    T.exec(sql);
}


void StockDatabase::updateOrderStatus(string order_id, string status, double limit_price, transaction<isolation> &T){
    string sql = "UPDATE ORDERS "
                 "SET STATUS = '" + status + "', LIMIT_PRICE = " + to_string(limit_price) + ", TIME = now() " +
                 "WHERE ID = " + order_id + ";";
    T.exec(sql);
}

void StockDatabase::updateAccountBalance(string account_id, double money, string operation, transaction<isolation> &T){
    string sql = "UPDATE ACCOUNTS SET BALANCE = BALANCE"
                 " " + operation + " " + to_string(money) + " WHERE ACCOUNT_ID = " + account_id + " ;";
    T.exec(sql);
}

void StockDatabase::deleteOrder(string order_id, transaction<isolation> &T){
    string sql = "DELETE FROM ORDERS WHERE ID = " + order_id + ";";
    T.exec(sql);
}


/**********************************************************************************
 *                                Query Functions                                 *
 **********************************************************************************/
bool StockDatabase::isAccountExist(string account_id, transaction<isolation> &T){
    string sql = "SELECT ACCOUNT_ID "
                 "FROM ACCOUNTS "
                 "WHERE ACCOUNTS.ACCOUNT_ID = " + account_id + ";";
    result R(T.exec(sql));
    return !R.empty();
}

bool StockDatabase::isSymbolExistForId(int account_id, string sym_name, transaction<isolation> &T){
    string sql = "SELECT ACCOUNT_ID, SYM_NAME "
                 "FROM POSITIONS "
                 "WHERE POSITIONS.ACCOUNT_ID = " + to_string(account_id)
                 + " AND POSITIONS.SYM_NAME = '" + sym_name + "';";
    result R(T.exec(sql));
    return !R.empty();
}

bool StockDatabase::isSellOrderValid(string account_id, string sym_name, double order_num, transaction<isolation> &T){
    string sql = "SELECT ACCOUNT_ID FROM POSITIONS "
                 "WHERE POSITIONS.ACCOUNT_ID = " + account_id
                 + " AND POSITIONS.SYM_NAME = '" + sym_name + "' AND POSITION_AMOUNT >= "
                 + to_string(order_num) + ";";
    result R(T.exec(sql));
    return !R.empty();
}

bool StockDatabase::isBuyOrderValid(double balance, double amount, double limit){
    return balance >= amount*limit;
}

/**********************************************************************************
 *                                Get Functions                                   *
 **********************************************************************************/
result StockDatabase::getTransInfo(string account_id_str, string transaction_id) {
    transaction<isolation> db(*this->C);
    string sql = "SELECT ORDER_AMOUNT, LIMIT_PRICE, EXTRACT(EPOCH FROM TIME), STATUS "
                 "FROM ORDERS "
                 "WHERE TRANS_ID = " + transaction_id + " AND ACCOUNT_ID = " + account_id_str + ";";
    db.exec(sql);
    result R(db.exec(sql));
    db.commit();
    if(R.empty()){
        throw order_exception("Transaction id is invalid");
    }
    return R;
}

double StockDatabase::getAccountBalance(string account_id, transaction<isolation> &T){
    string sql = "SELECT BALANCE FROM ACCOUNTS "
                 "WHERE ACCOUNTS.ACCOUNT_ID = " + account_id + ";";
    result R(T.exec(sql));
    if(!R.empty()){
        return R[0]["BALANCE"].as<double>();
    }
    else{
        throw order_exception("Account Id is invalid");
    }
}

/**********************************************************************************
 *                               Cancel functions                                 *
 **********************************************************************************/

bool StockDatabase::isCancelOperationValid(string account_id_str, string transaction_id, order_info_t &valid_order_info, transaction<isolation> &T){
    /* -> check validation
    *  1. transaction_id exist in the database
    *  2. exist "open" order(only 1) for this transaction id
    */
    string sql = "SELECT * FROM ORDERS "
                 "WHERE TRANS_ID = " + transaction_id + " AND ACCOUNT_ID = " + account_id_str + "ORDER BY STATUS;";
    result R(T.exec(sql));

    // cancel operation can only be performed for existed transaction id
    if(!R.empty()){
        // if exist "open" order, the cancellation is valid
        if(R[0]["STATUS"].as<string>() == "open"){
            valid_order_info.ID = to_string(R[0]["ID"].as<int>());
            valid_order_info.account_id = to_string(R[0]["ACCOUNT_ID"].as<int>());
            valid_order_info.limit_price = R[0]["LIMIT_PRICE"].as<double>();
            valid_order_info.amount = R[0]["ORDER_AMOUNT"].as<double>();
            valid_order_info.type = R[0]["TYPE"].as<string>();
            return true;
        }
        // otherwise, do nothing
        return false;
    }
    else{
        throw order_exception("The cancellation of order is invalid.");
    }
}

void StockDatabase::performCancelOperation(order_info_t cancelled_order_info, transaction<isolation> &T){
    // cancel the order
    updateOrderStatus(cancelled_order_info.ID, "cancelled", cancelled_order_info.amount, T);

    // refund, if the order is buyer order
    if(cancelled_order_info.type == "buy"){
        double refund_money = cancelled_order_info.amount * cancelled_order_info.limit_price;
        updateAccountBalance(cancelled_order_info.account_id, refund_money, "+", T);
    }
}
/**********************************************************************************
 *                            Matching Orders functions                           *
 **********************************************************************************/

void StockDatabase::updateTwoMatchedOrders(execution_info_t execution_info, order_info_t partially_executed_order_info,
                                           order_info_t completely_executed_order_info, bool executed_partially,
                                           transaction<isolation> &T){
    /// update two match orders
    if (executed_partially) {
        // if curr order have left
        // 1. update curr order with (- executed amount)
        // 2. insert executed matched order
        // 3. insert executed curr order

        // insert new "executed" partially order
        insertSplitOrder(partially_executed_order_info.transaction_id, partially_executed_order_info.account_id,
                         execution_info.sym_name, to_string(execution_info.execution_amount),
                         to_string(execution_info.execution_price), partially_executed_order_info.type, T);

        // update the whole "open" complete order into "executed"
        updateOrderStatus(completely_executed_order_info.ID, "executed", execution_info.execution_price, T);

        // update remain "opened" curr order's amount
        updateOrderAmount(partially_executed_order_info.ID, execution_info.execution_amount, "-", T);
    }
    else {
        // if two orders perfectly match
        // update their order status into "executed" with execution price
        updateOrderStatus(partially_executed_order_info.ID, "executed", execution_info.execution_price, T);
        updateOrderStatus(completely_executed_order_info.ID, "executed", execution_info.execution_price, T);

    }
}

void StockDatabase::executeOrder(execution_info_t execution_info, order_info_t partially_executed_order_info,
                                 order_info_t completely_executed_order_info, bool executed_partially,
                                 transaction<isolation> &T){

    // update information of two matched orders
    updateTwoMatchedOrders(execution_info, partially_executed_order_info, completely_executed_order_info, executed_partially, T);

    order_info_t buy_order_info;
    order_info_t sell_order_info;

    if(partially_executed_order_info.type == "buy"){
        buy_order_info = partially_executed_order_info;
        sell_order_info = completely_executed_order_info;
    }
    else{
        buy_order_info = completely_executed_order_info;
        sell_order_info = partially_executed_order_info;
    }

    // add money to seller's account -> + execution_price * share_num
    updateAccountBalance(sell_order_info.account_id, execution_info.execution_amount * execution_info.execution_price, "+", T);

    // create new position in the buyer's account -> + share_num: addShareToBuyerAccount()
    insertIntoPosition(buy_order_info.account_id, execution_info.sym_name, to_string(execution_info.execution_amount), T);

    // refund difference to buyer's account
    if(execution_info.execution_price < buy_order_info.limit_price){
        double price_diff = buy_order_info.limit_price - execution_info.execution_price;
        updateAccountBalance(buy_order_info.account_id, price_diff * execution_info.execution_amount, "+", T);
    }
}

void StockDatabase::matchOrders(string sym_name, order_info_t curr_order_info, transaction<isolation> &T){
    double curr_order_amount = curr_order_info.amount;
    // select sell order with "open" + the same "sym_name" + sell_price <= buy_price(descending)
    while (true){
        transaction<isolation> db(*this->C);
        curr_order_info.amount = curr_order_amount;
        try{
            // select matching orders from database
            // 1. give the best price match
            // 2. break tie by giving priority to older one
            // [caution]: not match self orders
            string sql;
            if(curr_order_info.type == "sell"){
                sql = "SELECT ID, TRANS_ID, ACCOUNT_ID, SYM_NAME, ORDER_AMOUNT, LIMIT_PRICE, TYPE, TIME "
                             "FROM ORDERS WHERE SYM_NAME = '" + sym_name + "' AND STATUS = 'open' AND TYPE = 'buy' " +
                             "AND LIMIT_PRICE >= " + to_string(curr_order_info.limit_price) + " "
                             "AND ACCOUNT_ID <> " + to_string(curr_order_info.account_id) + " "
                             "ORDER BY LIMIT_PRICE DESC, TIME ASC;";
            }
            else{
                sql = "SELECT ID, TRANS_ID, ACCOUNT_ID, SYM_NAME, ORDER_AMOUNT, LIMIT_PRICE, TYPE, TIME "
                             "FROM ORDERS WHERE SYM_NAME = '" + sym_name + "' AND STATUS = 'open' AND TYPE = 'sell' " +
                             "AND LIMIT_PRICE <= " + to_string(curr_order_info.limit_price) + " "
                             " AND ACCOUNT_ID <> " + to_string(curr_order_info.account_id) + " "
                             " ORDER BY LIMIT_PRICE ASC, TIME ASC;";
            }
            result R(db.exec(sql));

            // if exist matching orders
            if(!R.empty()){
                for(auto matched_order : R){
                    execution_info_t execution_info;
                    order_info_t matched_order_info;

                    // insert matched_order info
                    matched_order_info.ID = to_string(matched_order["ID"].as<int>());
                    matched_order_info.transaction_id = to_string(matched_order["TRANS_ID"].as<int>());
                    matched_order_info.account_id = to_string(matched_order["ACCOUNT_ID"].as<int>());
                    matched_order_info.limit_price = matched_order["LIMIT_PRICE"].as<double>();
                    matched_order_info.amount = matched_order["ORDER_AMOUNT"].as<double>();
                    matched_order_info.type = matched_order["TYPE"].as<string>();

                    // insert execution info
                    execution_info.sym_name = sym_name;
                    execution_info.execution_price = matched_order_info.limit_price; // the execution price is the older of those two orders

                    //assert(curr_order_info.amount >= 0);
                    if(matched_order_info.amount > curr_order_info.amount){
                        // split matched order
                        execution_info.execution_amount = curr_order_info.amount;
                        double left_matched_amount = matched_order_info.amount - curr_order_info.amount;
                        matched_order_info.amount = left_matched_amount;
                        // split matched order
                        executeOrder(execution_info, matched_order_info, curr_order_info, true, db);
                        break;
                    }
                    else{
                        // split current order
                        execution_info.execution_amount = matched_order_info.amount;
                        curr_order_info.amount -= matched_order_info.amount;
                        if(curr_order_info.amount == 0){
                            // both order become executed totally
                            executeOrder(execution_info, curr_order_info, matched_order_info, false, db);
                            break;
                        }
                        else{
                            // split current order
                            executeOrder(execution_info, curr_order_info, matched_order_info, true, db);
                        }
                    }
                }
            }
            db.commit();
            break;
        }
        catch (const pqxx::serialization_failure& e){
            db.abort();            // Usually not needed; same happens when T's life ends.
        }
    }
}


/**********************************************************************************
 *                            API for <create> tag                                *
 **********************************************************************************/

void StockDatabase::createAccount(string account_id_str, string balance_str, XMLResponse & response) {
    // format check
    try{
        transaction<isolation> db(*this->C);
        insertIntoAccounts(account_id_str, balance_str, db);
        db.commit();
        response.created_id(account_id_str);
        return;
    }
    catch (const pqxx::unique_violation &e){
        response.error_id(account_id_str, "Account already exist.");
    }
    catch (const pqxx::check_violation &e){
        response.error_id(account_id_str, "Balance is negative.");
    }
    catch (const pqxx::sql_error &e){
        response.error_id(account_id_str, to_string(e.what()));
    }
    catch (const std::exception &e){
        response.error_id(account_id_str, to_string(e.what()));
    }
    catch (...){
        response.error_id(account_id_str, "Unknown error");
    }
}

void StockDatabase::placeSymbol(string sym_name_str, vector<pair<string, string>> sym_info_list, XMLResponse &response) {
    for(auto sym_info_str : sym_info_list){
        string account_id_str = sym_info_str.first;
        string sym_amount_str = sym_info_str.second;

        while(true){
            transaction<isolation> db(*this->C);
            try{
                if(isAccountExist(account_id_str, db)){
                    insertIntoPosition(account_id_str, sym_name_str, sym_amount_str, db);
                    db.commit();
                    response.created_sym(sym_name_str, account_id_str);
                    break;
                }
                else{
                    response.error_sym(sym_name_str, account_id_str, "Account not exist");
                    break;
                }
            }
            catch (const pqxx::serialization_failure& e){
                db.abort();            // Usually not needed; same happens when T's life ends.
            }
            catch (const pqxx::sql_error &e){
                response.error_sym(sym_name_str, account_id_str, e.what());
                break;
            }
            catch (const std::exception &e){
                response.error_sym(sym_name_str, account_id_str, e.what());
                break;
            }
            catch (...){
                response.error_id(account_id_str, "Unknown error");
                break;
            }
        }
        continue;
    }
}


void StockDatabase::processOrder(string account_id_str, string sym_name_str,
                  string order_amount_str, string limit_price_str, XMLResponse &response){
    // Retry transaction if it fails due to serialization failures
    //set limit time for while loop
    while(true){
        transaction<isolation> db(*C);
        try{
            // get balance and check account_id is valid
            double db_balance = getAccountBalance(account_id_str, db);

            order_info_t curr_order_info;
            insertNewOrder(account_id_str, sym_name_str, order_amount_str, limit_price_str, curr_order_info, db);

            if(curr_order_info.type == "buy"){ // buy order
                if(!isBuyOrderValid(db_balance,curr_order_info.amount, curr_order_info.limit_price)){
                    deleteOrder(curr_order_info.ID, db);
                    throw order_exception("The buy order is invalid.");
                }
                // deduct order cost from buyer's account
                updateAccountBalance(curr_order_info.account_id, curr_order_info.amount*curr_order_info.limit_price, "-", db);
            }
            else{ // sell order
                if(!isSellOrderValid(curr_order_info.account_id, curr_order_info.sym_name, curr_order_info.amount, db)){
                    deleteOrder(curr_order_info.ID, db);
                    throw order_exception("The sell order is invalid.");
                }
                // deduct share_amount from seller account
                updatePositionAmount(curr_order_info.account_id, curr_order_info.sym_name, curr_order_info.amount, "-", db);
            }
            db.commit();
            response.opened_order(sym_name_str, order_amount_str, limit_price_str,curr_order_info.transaction_id);

            // match current order with orders in the database
            matchOrders(curr_order_info.sym_name, curr_order_info, db);
            break;
        }
        catch (const pqxx::serialization_failure& e){
            db.abort();
            continue;
        }
        catch (const order_exception &e){
            response.error_order(sym_name_str, order_amount_str, limit_price_str, e.what());
            break;
        }
        catch (const pqxx::sql_error &e){
            cerr << e.what() << std::endl;
            response.error_order(sym_name_str, order_amount_str, limit_price_str, e.what());
            break;
        }
        catch (const std::exception &e){
            response.error_order(sym_name_str, order_amount_str, limit_price_str, e.what());
            break;
        }
        catch (...){
            response.error_id(account_id_str, "Unknown error");
            break;
        }
    }
}


void StockDatabase::processQuery(string account_id_str, string transaction_id_str, XMLResponse &response){
    try{
        result order_info_list = getTransInfo(account_id_str, transaction_id_str);
        //assert(order_info_list.size() > 0);
        for(auto order_info : order_info_list){
            // convert time from double into integer
            string order_status_str = order_info["STATUS"].c_str();
            string order_amount_str = order_info["ORDER_AMOUNT"].c_str();
            string order_limit_str = order_info["LIMIT_PRICE"].c_str();
            double order_time = order_info["date_part"].as<double >();
            string order_time_str = to_string((unsigned long)(order_time*1000000));

            if(order_status_str == "open"){
                response.open_shares(order_amount_str);
            }

            if (order_status_str == "executed") {
                response.exec_shares(order_amount_str, order_limit_str, order_time_str);
            }

            if (order_status_str == "cancelled") {
                response.canceled_shares(order_amount_str, order_time_str);
            }
        }
    }
    catch (const pqxx::sql_error &e){
        response.error_query_or_cancel(transaction_id_str, e.what());
    }
    catch (const std::exception &e){
        response.error_query_or_cancel(transaction_id_str, e.what());
    }
    catch (...){
        response.error_id(account_id_str, "Unknown error");
    }
}


void StockDatabase::processCancel(string account_id_str, string transaction_id_str, XMLResponse &response) {
    /*
     * 1. cancel the open order
     *    -> check validation
     *     1.1. transaction_id exist in the database
     *     1.2. exist "open" order(only 1) for this transaction id
     *    -> update status from "open" to "cancel"
     *    -> return the TYPE of the order
     * 2. if cancel a buy order
     *    -> refund purchase price to the buyer account
     * 3. return the order_info of this transaction id
     */
    try{
        // perform cancellation
        while (true) {
            transaction <isolation> db(*C);
            try {
                // check cancel operation validation
                order_info_t cancelled_order_info;
                if (isCancelOperationValid(account_id_str, transaction_id_str, cancelled_order_info, db)) {
                    performCancelOperation(cancelled_order_info, db);
                }
                // NOTE: do this inside try block
                db.commit();
                break;
            }
            catch (const pqxx::serialization_failure &e) {
                db.abort();
                continue;
            }
        }

        // return new status of ths transaction
        result order_info_list = getTransInfo(account_id_str, transaction_id_str);
        assert(order_info_list.size() > 0);

        for (auto order_info: order_info_list) {
            // convert time from double into integer
            string order_status_str = order_info["STATUS"].c_str();
            string order_amount_str = order_info["ORDER_AMOUNT"].c_str();
            string order_limit_str = order_info["LIMIT_PRICE"].c_str();
            double order_time = order_info["date_part"].as<double >();
            string order_time_str = to_string((unsigned long)(order_time*1000000));

            // no "open" order after perform cancellation successfully
            assert(order_status_str != "open");

            if (order_status_str == "executed") {
                response.exec_shares(order_amount_str, order_limit_str, order_time_str);
            }

            if (order_status_str == "cancelled") {
                response.canceled_shares(order_amount_str, order_time_str);
            }
        }
    }
    catch (const pqxx::sql_error &e){
        response.error_query_or_cancel(transaction_id_str, e.what());
    }
    catch (const std::exception &e){
        response.error_query_or_cancel(transaction_id_str, e.what());
    }
    catch (...){
        response.error_id(account_id_str, "Unknown error");
    }
}

StockDatabase::StockDatabase() {
    setupDatabase();
}

StockDatabase::StockDatabase(bool isInitializeDB) {
    setupDatabase();
    if(isInitializeDB){
        // initialize tables and types in the database
        try{
            initializeDatabase();
        }
        catch (const std::exception &e){
            cerr << e.what() << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

StockDatabase::~StockDatabase() {
    //Close database connection
    this->C->disconnect();
}

