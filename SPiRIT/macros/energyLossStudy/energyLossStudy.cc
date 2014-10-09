#include "../run_mc_eloss.C"
#include "calculateEnergyLoss.cc"

void energyLossStudy(Bool_t createMC = kFALSE)
{
  TString tagHead = "eLossTest";
  TString tag;

  Int_t nEvents     = 10;
  Int_t particle    = 0;
  Int_t maxMomentum = 400;
  Int_t dMomentum   = 10;

  Int_t    momentum;
  Double_t energy;

  Double_t mass = 938.272; // [MeV]

  const Int_t nEnergy = maxMomentum/dMomentum;
  Double_t dEdx[nEnergy];
  Double_t travelDistance[nEnergy];

  Double_t dEdxVal;
  Double_t travelDistanceVal;

  cout << "[creatMCSamples ] Number of Energy  : " << setw(8) << nEnergy << endl;

  for(Int_t iEnergy=0; iEnergy<nEnergy; iEnergy++)
  {
    momentum = (iEnergy+1)*dMomentum;
    energy = sqrt(mass + momentum*momentum);
    cout << "[creatMCSamples ] Particle momentum : " << setw(8) << momentum << endl;
    cout << "[creatMCSamples ] Particle energy   : " << setw(8) << energy   << endl;
    tag = tagHead + "_" + TString::Itoa(particle,10) + "_" + TString::Itoa(momentum,10) + "MeV";




    if(createMC) run_mc_eloss(nEvents, particle, (Double_t)momentum, tag); // MC
    calculateEnergyLoss(tag, dEdxVal, travelDistanceVal); // Energy loss calculation




    dEdx[iEnergy]           = dEdxVal;
    travelDistance[iEnergy] = travelDistanceVal;
  }

  cout << endl;
  cout << setw(20) << "Energy [MeV]" 
       << setw(20) << "dE/dx [MeV/mm]" 
       << setw(25) << "Travel distance [mm]" << endl;
  cout << "--------------------|" 
       << "-------------------|"
       << "------------------------" << endl;
  for(Int_t iEnergy=0; iEnergy<nEnergy; iEnergy++)
  {
    momentum = (iEnergy+1)*dMomentum;
    energy = sqrt(mass + momentum*momentum);
    cout << setw(20) << energy
         << setw(20) << dEdx[iEnergy]
         << setw(25) << travelDistance[iEnergy] << endl;
  }

}
