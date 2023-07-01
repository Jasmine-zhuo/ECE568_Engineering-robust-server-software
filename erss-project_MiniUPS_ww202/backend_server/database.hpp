#ifndef _DATABASE_H__
#define _DATABASE_H__

#include "Common.hpp"


class database {
private:
  connection * c;
 const static pqxx::isolation_level isolation = pqxx::isolation_level::serializable;
  
public:
 std::mutex my_mutex_lock;
 int UPS_seqnum;
 unordered_set<long> acked_seqnum_set;
 unordered_set<long> dealt_seqnum_set;


  database(int seqnum);
  ~database();
  result run_query(string &sql);
  result run_command(string &sql);
  void delete_database(); 
  void InitWorld(long world_id,bool is_current_world);
  void InitTruck(int i, int x, int  y);
  int GetTruck();
  bool CheckAccountName(string accountName);
  void DealWithPickTruck(AU_pick_truck cur_pick_truck,socket_out * ClientOfWorld_out);
  void DealWithDeliverPackage(AU_deliver_package cur_deliver_package,socket_out * ClientOfWorld_out);
  void DealWithUfinished(UFinished cur_UFinished,socket_out * ClientOfWorld_out,socket_out * ClientOfAmazon_out);
  void SendAckToWorld(long seqnum,socket_out * ClientOfWorld_out);
  void DealWithDeliverFinished(UFinished myUFinished);
  void DealWithPickupFinished(UFinished myUFinished,socket_out * ClientOfAmazon_out);
  void DealWithUdeliverymade(UDeliveryMade cur_Udelimade,socket_out * ClientOfWorld_out, socket_out * ClientOfAmazon_out);
  void DealWithTruckStatus(UTruck cur_Utruck,socket_out * ClientOfWorld_out, socket_out * ClientOfAmazon);
  void DealWithUerr(UErr cur_UErr, socket_out * ClientOfWorld_out);

  /*
  void add_buy_command(ACommands &WarehouseRequest, int whnum, int id, string &description, int count);
  void add_load_command(ACommands &WarehouseRequest, int whnum, int truckid, int shipid);
  void add_goodReady_command(A2UResponses &UpsRequest, int whid, string username,\
                           int dex, int dey, int ordernum, int id, \
			     string description, int count);
  int get_order(ACommands &WarehouseRequest, A2UResponses &UpsRequest);
  void insert_unfulorder (int ordernum, int id, string &description, int amount, string ups, int whid, int desx, int desy);
  void update_stock(int id, int whid, int amount);

  void deal_truckReady (int goodid, int truckid, int order_num, ACommands & \
			WarehouseRequest);
  void deal_stock_arrived (A2UResponses &UpsRequest, int s_id, int s_whid, int s_amount);
  void deal_pack_ready(int goodid, ACommands & WarehouseRequest);
  void deal_truckArrived(int truckid, int whid, ACommands & WarehouseRequest);
  void deal_loaded(int goodid, A2UResponses &UpsRequest);
  void deal_packageFinished(int goodid);
  */
  
};

#endif
