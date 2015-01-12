#include "../run_mc_eloss.C"

//#include "STMCPoint.hh"

#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"

#include <iostream>
#include <fstream>

using namespace std;





void energyLossStudy(TString outFileName = "data/energyLoss.dat", 
                     Bool_t  createMC    = kFALSE,   // do create MC or not
                     Int_t   nEvents     = 10,       // number of events to run for each momentum
                     Int_t   particle    = 0,        // particle number written in run_mc_eloss.C
                     Int_t   momentum    = 300)      // [MeV/c2]
{
  TString tag = "eLossTest_" + TString::Itoa(particle,10) + "_" + TString::Itoa(momentum,10) + "MeV";

  Double_t uToMeV = 931.494061;   // [Atomic weight] to [MeV/c2]
  Double_t energy;
  Double_t mass;                  // [MeV]

  if (particle==0) mass =  938.272;     // Proton
  if (particle==4) mass = 3727.379;     // Helion

  Double_t dEdx;
  Double_t dEdx2; // for secondaries
  Double_t travelDistance;

  energy = sqrt(mass*mass + momentum*momentum) - mass;
  cout << "[creatMCSamples ] Particle momentum : " << setw(8) << momentum << " MeV/c" << endl;
  cout << "[creatMCSamples ] Particle energy   : " << setw(8) << energy   << " MeV" << endl;

  if(createMC) run_mc_eloss(nEvents, particle, (Double_t)momentum, tag); // MC
  CalculateEnergyLoss(tag, dEdx, dEdx2, travelDistance); // Energy loss calculation

  ofstream outFile(outFileName.Data(), ios::out | ios::app );
           outFile << setw(3)  << particle
                   << setw(5)  << momentum
                   << setw(15) << energy
                   << setw(15) << dEdx
                   << setw(15) << dEdx2
                   << setw(15) << travelDistance
                   << endl;
}





void CalculateEnergyLoss(TString   mcFileNameTag = "test",
                         Double_t  &dEdxVal,
                         Double_t  &dEdxVal2,
                         Double_t  &travelDistanceVal)
{
  TString mcFileNameHead = "data/spirit_";
  TString mcFileNameTail = ".mc.root";
  TString mcFileName     = mcFileNameHead + mcFileNameTag + mcFileNameTail;

  cout << "[CalculateEnergyLoss ] Calculating " << mcFileName << endl;

  Double_t zLength = 0;    // traveled length [cm]
  Double_t zLengthMean;    // traveled length [cm] - mean value

  Double_t energyLoss;     // energy loss
  Double_t energyLossSum;  // energy loss sum
  Double_t energyLossMean; // energy loss - mean value

  Double_t energyLoss2;     // energy loss secondaries
  Double_t energyLossSum2;  // energy loss sum secondaries
  Double_t energyLossMean2; // energy loss secondaries - mean value

  STMCPoint* point; 
  TClonesArray *pointArray;

  TFile* file = new TFile(mcFileName.Data(),"READ");
  TTree* tree = (TTree*) file -> Get("cbmsim");
         tree -> SetBranchAddress("STMCPoint", &pointArray);

  cout << " - " << "i "<< "zLength[mm] " << "energyLossSum[MeV]" << endl;
  Int_t nEvents = tree -> GetEntriesFast();
  for(Int_t iEvent=0; iEvent<nEvents; iEvent++) 
  {
    tree -> GetEntry(iEvent);

    energyLossSum  = 0;
    energyLossSum2 = 0;
    Int_t n = pointArray -> GetEntries();
    for(Int_t i=0; i<n; i++) {
      point = (STMCPoint*) pointArray -> At(i);
      if((point -> GetTrackID())!= 0) //continue;
      {
        energyLoss2 = ( point -> GetEnergyLoss() )*1000; // [GeV] to [MeV]
        energyLossSum2 += energyLoss2;
      }
      else
      {
        energyLoss = ( point -> GetEnergyLoss() )*1000; // [GeV] to [MeV]
        energyLossSum += energyLoss; 
        if(zLength<(point->GetLength()*10)) zLength = point -> GetLength()*10; // [cm] to [mm]
      }
    }

    if(zLength==0) continue;
    energyLossMean  = ((Double_t)iEvent/(iEvent+1) *  energyLossMean) + (( energyLossSum/zLength)/(iEvent+1));
    energyLossMean2 = ((Double_t)iEvent/(iEvent+1) * energyLossMean2) + ((energyLossSum2/zLength)/(iEvent+1));
    zLengthMean     = ((Double_t)iEvent/(iEvent+1) *     zLengthMean) + (                zLength /(iEvent+1));
  }

  dEdxVal           = energyLossMean;
  dEdxVal2          = energyLossMean2;
  travelDistanceVal = zLengthMean;
}
