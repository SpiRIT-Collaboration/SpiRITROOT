#include <iostream>
#include <fstream>


void test()
{
  ifstream file("edep.out");
  double k, x, y, z, t, e;

	Double_t detZ  = 1446.4;
	Double_t detZ2 = detZ/2;

  TH1D* hist = new TH1D("hist",";Kinetic energy",10,0,400);
  hist -> SetMinimum(0);
  hist -> SetMaximum(0.005);
  TGraph* graph = new TGraph();
  graph -> SetMarkerStyle(20);

  Double_t energyLossMean = 0;
  Double_t energyLossSum  = 0;
  Double_t kineticEnergy  = 0;
  Double_t driftLength    = 0;

  ofstream outFile("geant4Proton.dat");

  Int_t index = 0;
  Int_t dx;
  while(file >> k >> x >> y >> z >> t >> e) {

    if(index==0) kineticEnergy = k;

    if(kineticEnergy==k) {
      energyLossSum += e;
      if(driftLength<z) driftLength = z;
    }

    else {
      //energyLossMean = ( (Double_t)index/(index+1) * energyLossMean ) 
      //               + ( (energyLossSum/driftLength)/(index+1) );
      energyLossMean = energyLossSum/driftLength;

      cout << kineticEnergy << " " << energyLossMean << endl;
      graph -> SetPoint(index, kineticEnergy, energyLossMean);
      outFile << kineticEnergy << " " << energyLossMean << endl;

      kineticEnergy = k;
      energyLossSum = 0;
      driftLength   = 0;

      energyLossSum += e;
      if(driftLength<z) driftLength = z;
    }

    index++;
  }

  TCanvas* cvs = new TCanvas("cvs","cvs",700,700);
  cvs -> SetGrid();

  hist -> Draw("");
  graph -> Draw("same P");
}
