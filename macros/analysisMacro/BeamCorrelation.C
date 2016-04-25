#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "TCutG.h"

const char *cutfname =NULL;
bool filecalled = false;

void BeamCorrelation(Int_t runNo = 1930) {

  auto beam = new TChain("beam");
  auto cbmsim = new TChain("cbmsim");

  for (Int_t run : {1930} )
  {

    TString dataDir = "./data/";
    TString tag  = Form("run%d", run);

    TString ridfName = dataDir + tag + ".ridf.root";
    cout << ridfName << endl;
    beam -> AddFile(ridfName);

    TString recoName = dataDir + tag + ".reco.root";
    cout << recoName << endl;
    cbmsim -> AddFile(recoName);
  }
  cbmsim -> AddFriend(beam);

  TClonesArray *eventArray = nullptr;
  cbmsim -> SetBranchAddress("STEvent", &eventArray);

  auto *cvs1 = new TCanvas("cvs", "", 800, 800);
  cvs1 -> Divide(2,2);
  cvs1 -> cd(1); cbmsim -> Draw("fBeamVx:tx >> histbxvx(200, 0, 30, 200, -15, 15)");
  cvs1 -> cd(2); cbmsim -> Draw("fBeamVy:ty >> histbyvy(200, -15, 15, 200, -275, -250)");
  cvs1 -> cd(3); cbmsim -> Draw("fBeamVx-tx >> histx(200, -35, 0)");
  cvs1 -> cd(4); cbmsim -> Draw("fBeamVy-ty >> histy(200, -280, -250)");

  auto *cvs2 = new TCanvas("cvs2", "", 700, 500);
  auto *histBeamPID = new TH2D("histBeamPID", "", 100, 2.74, 2.87, 100, 56.5, 59.5);
  cbmsim -> Draw("z:aoq >> histBeamPID", "", "colz");

}
  
