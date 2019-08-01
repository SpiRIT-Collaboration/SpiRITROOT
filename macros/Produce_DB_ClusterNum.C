/*
  This script is uset to illustrate the method that we produce the DB for the Cluster number, dE/dX, track length.
*/

void Produce_DB_ClusterNum()
{
  ST_ProduceDB_ClusterNum* db_producer  = new ST_ProduceDB_ClusterNum();
  db_producer->Initial("Momentum.config");
  db_producer->BuildDB("f1_DB_ClusterNum.root");
}
