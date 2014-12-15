#include <iostream>
#include <fstream>


void test()
{
  ifstream file("edep.out");
  double k, x, y, z, t, e;

  TH1D* hist = new TH1D("histZ",";z",100,-800,800);
  while(file >> k >> x >> y >> z >> t >> e) {
    histZ -> Fill(z,e);
  }
  histZ -> Draw();
}
