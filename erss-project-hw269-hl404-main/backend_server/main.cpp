#include "Common.hpp"
#include "client.hpp"
#include "database.hpp"
#include "server.hpp"


void amazon_task(AU_commands myAUcommands, database * UPSdatabase, socket_out * ClientOfWorld_out,  socket_out * ClientOfAmazon_out, int simspeed) { 
   bool has_sth = false;
  for (int j = 0; j < myAUcommands.pick_size(); j++) {
    has_sth = true;
    AU_pick_truck cur_pick_truck = myAUcommands.pick(j);
    UPSdatabase->DealWithPickTruck(cur_pick_truck,ClientOfWorld_out);
  }
  
  for (int j = 0; j < myAUcommands.deliver_size(); j++) {
    has_sth = true;
    AU_deliver_package cur_deliver_package = myAUcommands.deliver(j);
    UPSdatabase->DealWithDeliverPackage(cur_deliver_package,ClientOfWorld_out);
  }
  
  for (int j = 0; j < myAUcommands.errors_size(); j++) {
    has_sth = true;
    UA_err cur_error = myAUcommands.errors(j);
    cout << "Received an error message from Amazon!\n";
    cout << "It is: " << cur_error.err() << endl;
  }
  if(!has_sth){
    cout<<"Received something but readed nothing useful out of it. Was it a valid 'AU_commands' message?!"<<endl;
  }
}



  
void world_task(UResponses myUResponses, database * UPSdatabase, socket_out * ClientOfWorld_out,  socket_out * ClientOfAmazon_out, int simspeed) {

  for (int j = 0; j < myUResponses.completions_size(); j++) {
    UFinished cur_ufinished = myUResponses.completions(j);
    UPSdatabase->DealWithUfinished(cur_ufinished,ClientOfWorld_out,ClientOfAmazon_out);
  }
  
  for (int j = 0; j < myUResponses.delivered_size(); ++j) {
    UDeliveryMade cur_Udelimade = myUResponses.delivered(j);
    UPSdatabase->DealWithUdeliverymade(cur_Udelimade,ClientOfWorld_out,ClientOfAmazon_out);
  }

  if(myUResponses.has_finished()){
    //this is World telling us that it got our 'Disconnect' request
    
  }
  
  for (int j = 0; j < myUResponses.acks_size(); ++j) {
    long cur_ack = myUResponses.acks(j);
    UPSdatabase->my_mutex_lock.lock();
    UPSdatabase->acked_seqnum_set.insert(cur_ack);
    UPSdatabase->my_mutex_lock.unlock();
  }
  
  for(int j=0; j < myUResponses.truckstatus_size(); ++j){
    UTruck cur_Utruck = myUResponses.truckstatus(j);
    UPSdatabase->DealWithTruckStatus(cur_Utruck,ClientOfWorld_out,ClientOfAmazon_out);
  }

  for(int j=0;j < myUResponses.error_size();++j){
    UErr cur_Uerr = myUResponses.error(j);
    UPSdatabase->DealWithUerr(cur_Uerr,ClientOfWorld_out);
    //this is the world telling us that some message we sent went wrong!
    //We definitedly need also to ack to this one because it has a new seqnum!
  }
  
}



int main(int argc, char *argv[]) {


    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::cout << std::put_time(&tm, "%d-%m-%Y %H-%M-%S") << std::endl;



  
  //set up parameters
  int TruckAmount, SimSpeed; 
  TruckAmount = 30;
  SimSpeed = 300;//this is the same with the default value
  srand(time(NULL));

  /*******************************************
   * STEP1: Open and initialize the database *
   *******************************************/
  
  
  unique_ptr<database> UPSdatabase (new database(101));//seqnum starts from 101
  UPSdatabase->delete_database();
  cout << "clear the database\n";

 /*************************************************************
   * STEP2: Connect to Amazon and then send the worldid to it  *
   *************************************************************/
    
  //   Client ClientOfAmazon((char*)"vcm-25368.vm.duke.edu",(char*)"2024");
        	      Client ClientOfAmazon((char*)"vcm-25052.vm.duke.edu",(char*)"34567"); 
  int result = ClientOfAmazon.to_connect();
  if (result == -1) {
    perror("connect to Amazon server failed\n");
  }
  
  int amazon_sockfd = ClientOfAmazon.get_sockfd();
  UA_commands myUAcommands;
  AU_commands myAUcommands;
  unique_ptr<socket_out> ClientOfAmazon_out(new socket_out(amazon_sockfd));
  
  unique_ptr<socket_in> ClientOfAmazon_in(new socket_in(amazon_sockfd));
  
  /***********************************************************************
   * STEP3: Connect to WorldSim server, intialize a world and get the id *  
   ***********************************************************************/
  //Client ClientOfWorld((char*)"vcm-25368.vm.duke.edu",(char*)"12345");
     Client ClientOfWorld((char*)"vcm-25052.vm.duke.edu",(char*)"12345"); 
  result = ClientOfWorld.to_connect();
  if (result == -1) {
    perror("connect to WorldSim server failed\n");
  }
  
  int world_sockfd = ClientOfWorld.get_sockfd();
  UConnect myUconnect;
  UConnected myUconnected;
  unique_ptr<socket_out> ClientOfWorld_out(new socket_out(world_sockfd));
  unique_ptr<socket_in> ClientOfWorld_in(new socket_in(world_sockfd));

  myUconnect.set_isamazon(false);
  cout<<"I'm here!\n";
  
 // init truck
  for (int i = 1;i < TruckAmount+1; i++) {
    UInitTruck * truck = myUconnect.add_trucks();
    int x = rand() % 2000 - 1000;
    int y = rand() % 2000 - 1000;
    truck->set_id(i);
    truck->set_x(x);
    truck->set_y(y);
    UPSdatabase->InitTruck(i, x, y);
  }
  //set simulation speed
  //  myUconnect.add_simspeed(SimSpeed);
  
  if (!sendMesgTo<UConnect> (myUconnect, ClientOfWorld_out.get()) ) {
    cerr << "failed to connect to world\n";
  }
  if (!recvMesgFrom<UConnected> (myUconnected, ClientOfWorld_in.get()) ) {
    cout << "failed to connect to world\n";
  }
  cout <<"The worldid from the Uconnected message is: " <<myUconnected.worldid()<<endl;
  cout<<"The result string from the Uconnected message is: "<<myUconnected.result()<<endl;

  /*
  UCommands test_commands;
  UGoPickup * test_ugpickup = test_commands.add_pickups();
  test_ugpickup->set_truckid(2);
  test_ugpickup->set_x(3);
  test_ugpickup->set_y(3);
  test_ugpickup->set_seqnum(101);

  if (!sendMesgTo<UCommands> (test_commands, ClientOfWorld_out.get()) ) {
    cerr << "failed to send Ugopickup to world\n";
  }
  
  */
  

  /*******************************************************************
   * STEP4: Save the World information into databse                  *
   *******************************************************************/
  long long world_id = myUconnected.worldid();
   UPSdatabase->InitWorld(world_id,true); //true means this is the current world we are using
  
  /*************************************************************
   * STEP5: Send the worldid to Amazon                         *
   *************************************************************/
  
    
   //  ssize_t send(int sockfd, const void *buf, size_t len, int flags);
  send(amazon_sockfd,&world_id,sizeof(long long),0);//send world id to Amazon
  
  /********************************************************************************
   * STEP6: Use socket select to interact with WorldSim server and Amazon Server  *
   ********************************************************************************/
  
  // epoll or select two fd
  
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(world_sockfd, &readfds);
  FD_SET(amazon_sockfd, &readfds);
  int numfds = max(world_sockfd,amazon_sockfd)+1;
  
  while(true) {
    fd_set tmp = readfds;
    if (select(numfds, &tmp, NULL, NULL, NULL) == -1) {
      perror("select error!");
      exit(4);
    }
    if (FD_ISSET(world_sockfd, &tmp)) {
      UResponses myUResponses;
      recvMesgFrom<UResponses> (myUResponses, ClientOfWorld_in.get());  
      std::thread world_handler(world_task, myUResponses, UPSdatabase.get(), ClientOfWorld_out.get(), ClientOfAmazon_out.get(), SimSpeed);
      if(world_handler.joinable()) {
	 cout << "Detaching world_handler thread" << endl;
	 world_handler.detach();
      } 
    }    
    if (FD_ISSET(amazon_sockfd, &tmp)){
      AU_commands myAUcommands;
      recvMesgFrom<AU_commands> (myAUcommands, ClientOfAmazon_in.get());
      std::thread amazon_handler(amazon_task, myAUcommands, UPSdatabase.get(),ClientOfWorld_out.get(), ClientOfAmazon_out.get(), SimSpeed);
      if(amazon_handler.joinable()) {
      cout << "Detaching amazon_handler thread" << endl;
      amazon_handler.detach();
      }
    }
  }
   
  cout<<"initial thread entered infinite loop!\n"; 
  while(1){
  }
  return EXIT_SUCCESS;
  
}
