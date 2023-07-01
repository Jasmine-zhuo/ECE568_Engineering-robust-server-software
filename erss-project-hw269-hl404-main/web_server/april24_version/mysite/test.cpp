#include <iostream>
#include <pqxx/pqxx>

using namespace std;
using namespace pqxx;

int main(int argc, char * argv[]) {
  //Allocate & initialize a Postgres connection object
  connection * C;

  try {
    //Establish a connection to the database
    //Parameters: database name, user name, user password
    C = new connection("dbname=postgres user=postgres password=passw0rd");
    if (C->is_open()) {
      //cout << "Opened database successfully: " << C->dbname() << endl;
    }
    else {
      cout << "Can't open database" << endl;
      return 1;
    }
  }
  catch (const std::exception & e) {
    cerr << e.what() << std::endl;
    return 1;
  }

  //TODO: create PLAYER, TEAM, STATE, and COLOR tables in the ACC_BBALL database
  //      load each table with rows from the provided source txt files

  stringstream sql;
  sql << "delete from \"myUPS_package\"; delete from \"myUPS_truck\"; delete from "
         "\"myUPS_account\"; delete from \"myUPS_world\";";

  //sql << "delete from \"myUPS_world\";";
  work W(*C);
  W.exec(sql.str());
  W.commit();

  //Close database connection
  C->disconnect();

  return 0;
}
