#include "database.hpp"
#include "Common.hpp"

database::database(int seqnum)
  : c(new connection("dbname=postgres user=postgres password=passw0rd" )),UPS_seqnum(seqnum)
{ }

database::~database() {
  delete c;

}
result database::run_query(string &sql) {
  while(true){
    try{
       nontransaction N(*c);
       result R(N.exec(sql));
       //       cout<<"Finished SQL: "<<sql<<endl;
       return R;
       }
    catch(const pqxx::usage_error & e){
      //N.abort();
      // cout<<e.what();
     }
  }
}
result database::run_command(string &sql) {

  while(true){
  //  work W(*c);
  try{
  transaction<isolation> W(*c);
   result R(W.exec(sql));
    W.commit();
    //    cout <<"***COMIITED SQL: "<<sql<<endl<<endl;
    return R;
  }
   catch(const pqxx::usage_error & e){
     // W.abort();
     //     cout<<e.what();
   }
   catch(const pqxx::serialization_failure& e){
     //W.abort();
     //  cout<<e.what();
   }
  }
 
}

void database::delete_database() {
  string sql = " delete from \"myUPS_world\"; delete from \"myUPS_account\";  delete from \"myUPS_truck\"; delete from \"myUPS_package\"; delete from auth_user;";
  
  run_command(sql);
}

void database::InitWorld(long world_id,bool is_current_world){
  
  string cur_world_str = (is_current_world)?("true"):("false");
  string sql = "insert into \"myUPS_world\"(world_id,curr) values(" + to_string(world_id) +", '"+cur_world_str+"');";
  
  run_command(sql);
}

void database::InitTruck(int i, int x, int  y) {
  cout << "*********************In 'InitTruck'***************************"<<endl;
  string sql = "insert into \"myUPS_truck\"(truck_id, cur_status,pos_x,pos_y)" \
    "values (" + to_string(i) + ", 'i', '" + to_string(x) + "', '" + to_string(y) + "');";
  
  run_command(sql);
}

int database::GetTruck(){
 my_mutex_lock.lock();
  cout<<"Searching for a truck that is available!" << endl;
  string sql = "SELECT truck_id from \"myUPS_truck\" WHERE cur_status = 'i' FOR UPDATE;";
  // cout<<sql<<endl;
   result R = run_query(sql);
   //result R = run_command(sql);
  if(R.size()==0){
     my_mutex_lock.unlock();
    return -1;//no truck is available!
  }
  else{
    int truck_id = R[0][0].as<int>();
    string sql = "UPDATE \"myUPS_truck\" SET cur_status = 't' WHERE truck_id = "+to_string(truck_id)+";";
      run_command(sql);
      my_mutex_lock.unlock();
    return truck_id;
  }
    
}

bool database::CheckAccountName(string accountName){

  
  cout << "************************In 'CheckAccountName'***************************" <<endl;
  string sql = "SELECT * FROM auth_user WHERE username = '"+accountName+"' FOR UPDATE;";
  //cout << sql << endl;
   result R = run_query(sql);
  // result R = run_command(sql);
  return ((R.size())!=0);
}


void database::DealWithPickTruck(AU_pick_truck cur_pick_truck,socket_out * ClientOfWorld_out){
cout << "************************In 'DealWithPickTruck'***************************" <<endl;

  long shipid = cur_pick_truck.shipid();
    int whid = cur_pick_truck.whid();
    long trackingNumber = cur_pick_truck.trackingnumber();
    string accountName = "Nobody"; //This means no user is related to the package
    long user_id = -1;

    bool has_valid_accountname = false;
    if(cur_pick_truck.has_accountname()){
      accountName = cur_pick_truck.accountname(); 
      if(this->CheckAccountName(accountName)){
	string sql = "SELECT id FROM auth_user WHERE username = '"+accountName+"' FOR UPDATE;";
	//        cout << sql << endl;
	    	result R = run_query(sql);
	    //	result R = run_command(sql);
       user_id = R[0][0].as<int>();
       has_valid_accountname = true;
      }
      else{
	cout << "Received an invalid accountname from Amazon!"<<endl;
      }
    }
    
    
    cout << "Got a [AU_pick_truck] message from Amazon!" << endl;
    cout << "shipid: " <<shipid<<endl;
    cout << "whid: " <<whid<<endl;
    cout << "trackingnumber: " << trackingNumber << endl;
    cout << "userName: " << accountName << endl;
    cout << "userID: " << user_id << endl;
    //assign a truck that is available to this package
    int truck_id; 
    do{
      truck_id = this->GetTruck();
    }while(truck_id < 0);
    //Modify the database
    stringstream ss;
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    ss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S") << std::endl;


    if(has_valid_accountname){
      string sql = "INSERT INTO \"myUPS_package\"(package_id,user_id,wh_id,cur_status,truck_id,ready_for_pickup_time,dest_x,dest_y) VALUES ('"+to_string(shipid)+"', "+to_string(user_id)+", '"+to_string(whid)+"', 'p',"+to_string(truck_id)+",'"+ss.str()+"','0','0');";
     
      run_command(sql);
    }    
    else{
      string sql = "INSERT INTO \"myUPS_package\"(package_id,wh_id,cur_status,truck_id,ready_for_pickup_time,dest_x,dest_y) VALUES('"+to_string(shipid)+"', '"+to_string(whid)+"','p',"+to_string(truck_id)+",'"+ss.str()+"','0','0');";
     
      run_command(sql);
    }

    //string sql = "UPDATE \"myUPS_truck\" SET cur_status = 't' WHERE truck_id = "+to_string(truck_id)+";";
     
    //run_command(sql);
 
    //get the current seqnum(in UPS space)
    int seqnum;
    my_mutex_lock.lock();
      seqnum = UPS_seqnum;
      UPS_seqnum++;
    my_mutex_lock.unlock();
    //build a UGopickUp command
    UCommands myUcommands;
    UGoPickup * my_UGopickup = myUcommands.add_pickups();
    my_UGopickup->set_truckid(truck_id);
    my_UGopickup->set_whid(whid);
    my_UGopickup->set_seqnum(seqnum);

    //change simspeed!!!!
    //  myUcommands.set_simspeed(1200);
    //keep sending things to WORLD until acked!(check if global acked set contains seqnum)
    do{
      if (!sendMesgTo<UCommands> (myUcommands, ClientOfWorld_out)){
         cerr << "failed to send a 'UCommand' message  to world\n";

      }else{
       
	cout<<"[SUCCESSFUL] Sent a UGoPickup to world! seqnum is :"<<seqnum<<", package id is "<<shipid<<"truck id is "<<truck_id<<endl;
	
      }
      	 std::this_thread::sleep_for(chrono::milliseconds(5000));     
      
    }while(acked_seqnum_set.find(seqnum)==acked_seqnum_set.end());

    cout<<"[SUCCESSFUL] Seqnum "<<seqnum<<" was acked by world!\n ";
    
}

void database::DealWithDeliverPackage(AU_deliver_package cur_deliver_package,socket_out * ClientOfWorld_out){
cout << "************************In 'DealWithDeliverPackage'***************************" <<endl;
  UCommands myUCommands;
  UGoDeliver * myUgoDeliver = myUCommands.add_deliveries();

      auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    stringstream ss;
    ss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S") << std::endl;
        
  for(int j = 0; j < cur_deliver_package.packages_size(); j++){
    UDeliveryLocation cur = cur_deliver_package.packages(j);
    UDeliveryLocation * curUdeliLoc = myUgoDeliver->add_packages();
    int cur_pkgid = cur.packageid();
    curUdeliLoc->set_packageid(cur_pkgid);
    int dest_x = cur.x();
    int dest_y = cur.y();
    string sql;
    sql = "SELECT dest_x,dest_y FROM \"myUPS_package\" WHERE package_id = '"+to_string(cur_pkgid)+"';";
    result R = run_command(sql);
    if(R.size()!=0){

      string dest_x_str = R[0][0].as<string>();
      string dest_y_str = R[0][1].as<string>();
      int already_stored_x = stol(dest_x_str);
      int already_stored_y = stol(dest_y_str);
      if(already_stored_x !=0 || already_stored_y!=0){
	dest_x = already_stored_x;
	dest_y = already_stored_y;
      }
    }
    
    sql = "UPDATE \"myUPS_package\" SET cur_status = 'o',load_time = '"+ss.str()+"',dest_x ='"+to_string(dest_x)+"', dest_y = '"+to_string(dest_y)+"'  WHERE package_id = '"+to_string(cur_pkgid)+"';";   
    run_command(sql);
    curUdeliLoc->set_x(dest_x);
    curUdeliLoc->set_y(dest_y);
    cout<<"Ready to send 'UGoDeliver' to world! packageid = "<<cur.packageid()<<", x = "<< cur.x()<<", y = "<<cur.y()<<endl;
  }
  int truckid = cur_deliver_package.truckid();
  myUgoDeliver->set_truckid(truckid);
  
  //NOTE: An AU_deliver_package can have multiple UdeliveryLocations!
  //assign a seqnum
  int seqnum;
  my_mutex_lock.lock();
  seqnum = UPS_seqnum;
  UPS_seqnum++;
  my_mutex_lock.unlock(); 

  myUgoDeliver->set_seqnum(seqnum);
 
    //undate database： truck status
    string sql;
    sql = "UPDATE \"myUPS_truck\" SET cur_status = 'd' WHERE truck_id = "+to_string(truckid)+";";
   
    run_command(sql);
    //keep sending things to WORLD until acked!(check if global acked set contains seqnum)
    do{
      if (!sendMesgTo<UCommands> (myUCommands, ClientOfWorld_out)){
         cerr << "failed to send a 'UCommand' message  to world\n";
      }
      else{
	cout<<"Successfully sent a 'UGoDeliver' message to World! Seqnum is : "<<seqnum<<endl;
      }
       std::this_thread::sleep_for(chrono::milliseconds(5000)); 
    }while(acked_seqnum_set.find(seqnum)==acked_seqnum_set.end());
    
}

void database::DealWithUfinished(UFinished cur_UFinished,socket_out * ClientOfWorld_out,socket_out * ClientOfAmazon_out){
cout << "************************In 'DealWithUfinished'***************************" <<endl;

  if(dealt_seqnum_set.find(cur_UFinished.seqnum())!=dealt_seqnum_set.end()){
    cout << "Already dealt with this UFinished response!!!!!!!!!!";
    return;
  }
  
  long seqnum = cur_UFinished.seqnum();
  SendAckToWorld(seqnum,ClientOfWorld_out);
  if(cur_UFinished.status() == "IDLE"){//this means this truck just finished all delivery mission
    cout<<"Dealing with a Ufinished for delivery finish!\n";
    DealWithDeliverFinished(cur_UFinished);
  }
  else{
    DealWithPickupFinished(cur_UFinished,ClientOfAmazon_out);
    cout<<"Dealing with a Ufinished for pickup finished!\n";
  }

}


void database::SendAckToWorld(long seqnum,socket_out * ClientOfWorld_out){
cout << "************************In 'SendAckToWorld'***************************" <<endl;
  my_mutex_lock.lock();
  dealt_seqnum_set.insert(seqnum);
  my_mutex_lock.unlock();
  
  UCommands myUCommands;
  myUCommands.add_acks((int)seqnum); 
  if(!sendMesgTo<UCommands> (myUCommands, ClientOfWorld_out)){
    cerr << "failed to send ACK for seqnum "+ to_string(seqnum)+" to world!\n";
    }
  else{
    cout << "Sent ACK for seqnum "+ to_string(seqnum) + " to world!\n";
  }
}

    
 
void database::DealWithDeliverFinished(UFinished myUFinished){
  
  cout << "************************In 'DealWithDeliverFinished'***************************" <<endl;
   int truckid = myUFinished.truckid();
   int x = myUFinished.x();
   int y = myUFinished.y();
   cout<<"From 'UFinished' information from world:A truck has finished all its mission!\n";
   cout<<"truckid = "<<truckid<<", the final position of this truck is "<<"("<<x<<", "<<y<<")\n";
   string sql = "UPDATE \"myUPS_truck\" SET cur_status = 'i', pos_x =  '"+to_string(x)+"', pos_y = '"+to_string(y)+"' WHERE truck_id = "+to_string(truckid)+";";
  
   run_command(sql);   
  }
  
void database::DealWithPickupFinished(UFinished myUFinished,socket_out * ClientOfAmazon_out){
cout << "************************In 'DealWithPickupFinished'***************************" <<endl;
   int truckid = myUFinished.truckid();
   int x = myUFinished.x();
   int y = myUFinished.y();
   cout<<"[SUCCESSFUL] Received UFinished (pick up finished) for truck id"<< truckid<<endl;
   
   cout<<"The information contained in this 'UFinished' message is:\n";
   cout<<"truckid = "<<truckid<<", x ="<<x<<", y = "<<y<<endl<<endl;
   string sql = "UPDATE \"myUPS_truck\" SET cur_status = 'l', pos_x =  "+to_string(x)+", pos_y = "+to_string(y)+" WHERE truck_id = "+to_string(truckid)+";";
   run_command(sql);
   sql = "UPDATE \"myUPS_package\" SET wh_addr_x = '"+to_string(x)+"', wh_addr_y = '"+to_string(y)+"' WHERE truck_id = "+to_string(truckid)+";";
   run_command(sql);
    
   
   //Build A UA_truck_picked message and send it to Amazon
   //get the package id associated with this truck
   sql = "SELECT package_id FROM \"myUPS_package\" WHERE truck_id = "+to_string(truckid)+" AND cur_status = 'p' FOR UPDATE;";
   // cout << sql << endl;
     result R = run_query(sql);
    //result R = run_command(sql);
   string shipid_str;
   int shipid;
   if(R.size()==0){
     cout<<"!!!!!!!!!!!!!!!!!!!!!didn't find such packaged!!!!!\n";
     shipid = -1;
   }
   else{
    
    shipid_str = R[0][0].as<string>();
    shipid = stol(shipid_str);
   }
    UA_commands myUAcommands;
   UA_truck_picked * myUAtruckpicked = myUAcommands.add_pick();
   myUAtruckpicked->set_shipid(shipid);
   myUAtruckpicked->set_truckid(truckid);  
   if (!sendMesgTo<UA_commands> (myUAcommands, ClientOfAmazon_out) ) {
    cerr << "failed to send a UA_truck_picked message to Amazon!\n";
   }
   else{
     cout << "Successfully sent a UA_truck_picked message to Amazon!\n";
   }
 }

void database::DealWithUdeliverymade(UDeliveryMade cur_Udelimade,socket_out * ClientOfWorld_out, socket_out * ClientOfAmazon_out){
cout << "************************In 'DealWithUdeliverymade'***************************" <<endl;
std::this_thread::sleep_for(chrono::milliseconds(10000)); 


 int truckid = cur_Udelimade.truckid();
  long packageid = cur_Udelimade.packageid();
  long seqnum = cur_Udelimade.seqnum();
  //send ack to world
  SendAckToWorld(seqnum,ClientOfWorld_out);
  //modify database: package status
  stringstream ss;
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    ss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S") << std::endl;



  

  string sql;
  sql = "UPDATE \"myUPS_package\" SET cur_status = 'd',delivered_time = '"+ss.str()+"'  WHERE package_id = '"+to_string(packageid)+"';";
  
  run_command(sql);

  //send U_A_package_delivered to Amazon
  UA_commands myUAcommands;
  UA_package_delivered * cur_pkg_dived = myUAcommands.add_deliver();
  cur_pkg_dived->set_shipid(packageid);
  if (!sendMesgTo<UA_commands> (myUAcommands,ClientOfAmazon_out)){
    cerr<<"failed to send a UA_package_delivered message to Amazon!\n";
      }
  else{
    cout << "successfully sent a UA_package_delivered message to Amazon!\n";
  }
}

void database::DealWithTruckStatus(UTruck cur_Utruck,socket_out * ClientOfWorld_out, socket_out * ClientOfAmazon){
cout << "************************In 'DealWithTruckStatus'***************************" <<endl;
  int truckid = cur_Utruck.truckid();
  string truck_status = cur_Utruck.status();
  int x = cur_Utruck.x();
  int y = cur_Utruck.y();
  long seqnum = cur_Utruck.seqnum();

  //send ack to world
  SendAckToWorld(seqnum,ClientOfWorld_out);

  //haven't decide what to do, perhaps send this messgae to Amazon...

  cout<<"Got truck status informtion from World. truckid : "<<to_string(truckid)<<", status: "<<truck_status <<endl;
  
}

 
void database::DealWithUerr(UErr cur_UErr, socket_out * ClientOfWorld_out){
cout << "************************In 'DealWithUerr'***************************" <<endl;
  string err_msg = cur_UErr.err();
  long origi_seqnum = cur_UErr.originseqnum();
  long seqnum = cur_UErr.seqnum();
  //send ack to world
  SendAckToWorld(seqnum,ClientOfWorld_out);

  cout<<"Received an UErr message from World! message: "<<err_msg<<endl;

  
}
