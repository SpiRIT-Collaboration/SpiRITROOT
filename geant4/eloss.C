#include <iostream>
#include <fstream>

TString eLossProtonFile = "eLossProton.dat";

void eloss()
{
  gStyle -> SetOptStat(0);

  makeTree();
  draw();
}


void makeTree()
{
  ifstream inFile("edep.out");
  ofstream outFile(eLossProtonFile);

  int eventID, nHits, parentID;
  double eKin, x, y, z, eLoss;

  double eLossSum = 0;
  double rMax = 0;
  double dEdxMean = 0;
  double eKinPre = 0;

  int line = 0;
  int index = 0;

  while(inFile >> eKin >> eventID >> nHits)
  {
    if(line==0) eKinPre=eKin;
    if(eKinPre!=eKin) {
      outFile << eKinPre << " " << dEdxMean << endl;
      eKinPre=eKin;
      dEdxMean = 0;
      index++;
    }

    rMax = 0;
    eLossSum = 0;
    for(int iHit=0; iHit<nHits; iHit++) {
      inFile >> parentID >> x >> y >> z >> eLoss;
      double r = sqrt(x*x + y*y + z*z);
      if(parentID==0 && rMax<r) rMax = r;
      eLossSum += eLoss;
    }
    double dEdx = eLossSum/r;
    dEdxMean = (double)eventID/(eventID+1)*dEdxMean + dEdx/(eventID+1);
    line++;
  }
  outFile << eKinPre << " " << dEdxMean << endl;
}




void draw()
{
  TH1D* hist = new TH1D("hist",";Kinetic energy",10,0,400);
  hist -> SetMinimum(0);
  hist -> SetMaximum(0.005);
  TGraph* graph = new TGraph();
  graph -> SetMarkerStyle(20);

  Int_t i=0;
  Double_t k, e;
  ifstream inFile(eLossProtonFile);
  while(inFile >> k >> e)
    graph -> SetPoint(i++, k, e);

  TCanvas* cvs = new TCanvas("cvs","cvs",700,700);
  cvs -> SetGrid();
  hist -> Draw("");
  graph -> Draw("same P");
}
