#include "../run_mc_eloss.C"
#include "calculateEnergyLoss.cc"

void energyLossStudy(TString outFileName = "data/energyLoss.dat", 
                     Bool_t  createMC    = kFALSE, 
                     Int_t   nEvents     = 10,
                     Int_t   particle    = 0,
                     Int_t   momentum    = 300)
{
  TString tagHead = "eLossTest";
  TString tag     = tagHead + "_" + TString::Itoa(particle,10) + "_" + TString::Itoa(momentum,10) + "MeV";

  Double_t energy;
  Double_t mass = 938.272; // [MeV]

  Double_t dEdx;
  Double_t travelDistance;

  energy = sqrt(mass + momentum*momentum);
  cout << "[creatMCSamples ] Particle momentum : " << setw(8) << momentum << " MeV/c" << endl;
  cout << "[creatMCSamples ] Particle energy   : " << setw(8) << energy   << " MeV" << endl;

  if(createMC) run_mc_eloss(nEvents, particle, (Double_t)momentum, tag); // MC
  calculateEnergyLoss(tag, dEdx, travelDistance); // Energy loss calculation

  ofstream outFile(outFileName.Data(), ios::out | ios::app );
           outFile << setw(3)  << particle
                   << setw(5)  << momentum
                   << setw(15) << energy
                   << setw(15) << dEdx
                   << setw(15) << travelDistance
                   << endl;
}
