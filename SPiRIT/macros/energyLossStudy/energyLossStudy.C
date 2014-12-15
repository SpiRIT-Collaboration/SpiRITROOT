#include "../run_mc_eloss.C"
#include "calculateEnergyLoss.C"

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
  Double_t travelDistance;

  energy = sqrt(mass*mass + momentum*momentum) - mass;
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
