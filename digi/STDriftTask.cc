/**
 * @brief Process drifting of electron from created position to anode wire
 * plane. 
 *
 * @author JungWoo Lee (Korea Univ.)
 *
 * @detail See header file for detail.
 */

// This class & SPiRIT class headers
#include "STDriftTask.hh"
#include "STProcessManager.hh"
#include "STFairMCEventHeader.hh"

// Fair class header
#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

// STL class headers
#include <cmath>
#include <iostream>
#include <iomanip>

// Root class headers
#include "TLorentzVector.h"
#include "TString.h"
#include "TRandom.h"
#include "Math/Interpolator.h"

using namespace std;

STDriftTask::STDriftTask()
:FairTask("STDriftTask"),
 fEventID(0)
{
  fIsPersistence = kFALSE;

  fLogger->Debug(MESSAGE_ORIGIN,"Defaul Constructor of STDriftTask");
}

STDriftTask::~STDriftTask()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Destructor of STDriftTask");
}

void 
STDriftTask::SetParContainers()
{
  fLogger->Debug(MESSAGE_ORIGIN,"SetParContainers of STDriftTask");

  FairRunAna* ana = FairRunAna::Instance();
  FairRuntimeDb* rtdb = ana->GetRuntimeDb();
  fPar = (STDigiPar*) rtdb->getContainer("STDigiPar");
}

InitStatus 
STDriftTask::Init()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Initilization of STDriftTask");
  polya_lowgain->SetNpx(1000); // part of the function is not simulated well due to the narrow nature of the function
  // resolution needs to be manually increase

  FairRootManager* ioman = FairRootManager::Instance();

  fMCPointArray = (TClonesArray*) ioman->GetObject("STMCPoint");
  fMCTrackArray = (TClonesArray*) ioman->GetObject("PrimaryTrack");  
  fFairMCEventHeader = (FairMCEventHeader*) ioman->GetObject("MCEventHeader.");

  fElectronArray = new TClonesArray("STDriftedElectron");
  ioman->Register("STDriftedElectron","ST",fElectronArray,fIsPersistence);

  fYAnodeWirePlane = fPar->GetAnodeWirePlaneY(); // [mm]
  fZWidthPadPlane  = fPar->GetPadPlaneZ(); // [mm]
  fNumWires        = 363;
  fZSpacingWire    = 4;
  fZOffsetWire     = 2;

  /* 
   *           < CONFIGURATION 1 >              < CONFIGURATION 2 >      \n
   *
   *         -----------------------          -----------------------    \n
   *        |                       |        |                       |   \n
   *        |      1/3     2/3      |        |  1/6     1/2     5/6  |   \n
   *   PAD  |                       |        |                       |   \n
   *        |     across the pad    |        |     across the pad    |   \n
   *        |                       |        |                       |   \n
   *         ---------12 mm---------          -----------------------    \n
   *
   *        |       |       |       |            |       |       |       \n
   *        |       |       |       |            |       |       |       \n
   *  WIRE  |       |  4 mm |       |            |       |       |       \n
   *        |       |       |       |            |       |       |       \n
   *        |       |       |       |            |       |       |       \n
   *
   *
   *                             ---> z-axis (layer)
   *
   *
   *  - CONFIGURATION 1 : Use fZOffsetWire = 0
   *  - CONFIGURATION 2 : Use fZOffsetWire = 2
   *
   *  Default is CONFIGURATION 2.
   */

  fZCenterWire = fZWidthPadPlane/2+fZOffsetWire;
  fZFirstWire  = fZCenterWire - fNumWires/2*fZSpacingWire;
  fZLastWire   = fZCenterWire + fNumWires/2*fZSpacingWire;
  fZCritWire   = fZOffsetWire; // = 0*fZSpacingWire + fZOffsetWire
  fICritWire   = (fZCritWire-fZOffsetWire)/fZSpacingWire; // == 0
  fIFirstWire  = (fZFirstWire-fZOffsetWire)/fZSpacingWire;
  fILastWire   = (fZLastWire-fZOffsetWire)/fZSpacingWire;
  
  fEIonize  = fPar->GetEIonize()*1.E6; // [MeV] to [eV]
  fVelDrift = fPar->GetDriftVelocity()/100.; // [cm/us] to [mm/ns]
  fCoefT    = fPar->GetCoefDiffusionTrans()*sqrt(10.); // [cm^(-1/2)] to [mm^(-1/2)]
  fCoefL    = fPar->GetCoefDiffusionLong()*sqrt(10.);  // [cm^(-1/2)] to [mm^(-1/2)]
  fGain     = fPar->GetGain();
  fYDriftOffset = fPar->GetYDriftOffset();

  if(fSpline)
    fInterpolator = BichselCorrection(fSpecies);
  
  return kSUCCESS;
}

void 
STDriftTask::Exec(Option_t* option)
{
  fLogger->Debug(MESSAGE_ORIGIN,"Exec of STDriftTask");

  if(!fElectronArray) 
    fLogger->Fatal(MESSAGE_ORIGIN,"No DigitizedElectronArray!");
  fElectronArray -> Delete();

  Int_t nMCPoints = fMCPointArray->GetEntries();
  if(nMCPoints<10){
    fLogger->Warning(MESSAGE_ORIGIN, "Not enough hits for digitization! (<10)");
    return;
  }

  /**
   * NOTE! that fMCPoint has unit of [cm] for length scale,
   * [GeV] for energy and [ns] for time.
   */
  
  // associate momentum with track ID
  std::map<int, std::pair<int, double>> trackIDToPDGMom;
  for(int i = 0; i < fMCTrackArray -> GetEntries(); ++i)
  {
    fMCTrack = (STMCTrack*) fMCTrackArray->At(i);
    auto p = fMCTrack -> GetP()*1000;
    trackIDToPDGMom[fMCTrack -> GetTrackId()] = {fMCTrack -> GetPdgCode(),p};
  }

  for(Int_t iPoint=0; iPoint<nMCPoints; iPoint++) {
    fMCPoint = (STMCPoint*) fMCPointArray->At(iPoint);
    Double_t eLoss = 0.;
    if(fSpline)
    {
      TVector3 p(fMCPoint -> GetPx(), fMCPoint -> GetPy(), fMCPoint -> GetPz());
      eLoss = (fMCPoint->GetEnergyLoss())*1.E9*fInterpolator->Eval(p.Mag()); // [GeV] to [eV]        
      //      std::cout<<"[STDriftTask] energy loss GEANT4: " << (fMCPoint->GetEnergyLoss())*1.E9 << " eV" << std::endl;
      std::cout<<"[STDriftTask] energy loss Bichsel (with spline interpolation): " << eLoss << " eV" << std::endl;      
    }
    else
    {
      Double_t corFactor = 1;
      auto it = trackIDToPDGMom.find(fMCPoint -> GetTrackID());
      if(it != trackIDToPDGMom.end()) corFactor = BichselCorrection(it->second.first,it->second.second);
      eLoss = (fMCPoint->GetEnergyLoss())*1.E9*corFactor; // [GeV] to [eV]
    }

    Double_t lDrift = fYAnodeWirePlane-(fMCPoint->GetY())*10; // drift length [mm]
    Double_t tDrift = lDrift/fVelDrift + fYDriftOffset; // drift time [ns]
    Double_t sigmaL = fCoefL*sqrt(lDrift); // sigma in longitudinal direction
    Double_t sigmaT = fCoefT*sqrt(lDrift); // sigma in transversal direction

    Int_t nElectrons = (Int_t)floor(fabs(eLoss/fEIonize));
    for(Int_t iElectron=0; iElectron<nElectrons; iElectron++) {
      Double_t dr    = gRandom->Gaus(0,sigmaT); // displacement in radial direction
      Double_t angle = gRandom->Uniform(2*TMath::Pi()); // random angle

      Double_t dx = dr*TMath::Cos(angle); // displacement in x-direction
      Double_t dz = dr*TMath::Sin(angle); // displacement in y-direction
      Double_t dt = gRandom->Gaus(0,sigmaL)/fVelDrift; // displacement in time
      if(dt + tDrift +fMCPoint->GetTime() < 0) dt = 0;

      Int_t iWire = (Int_t)floor((fMCPoint->GetZ()*10+dz)/fZSpacingWire); //The index of the anode wire is from 0 to 112*3-1
      if(iWire < 0 || iWire > 112*3-1) continue; //the anode wire number wont change for the SpiRIT experiment
      //      if(iWire<fIFirstWire) iWire = fIFirstWire;
      //      if(iWire>fILastWire)  iWire = fILastWire;
      Int_t zWire = iWire*fZSpacingWire+fZOffsetWire;

      Int_t index = fElectronArray->GetEntriesFast();

      Int_t layer = (iWire/3);//We can find layer number with /3 because 3 cases for position of wire 
      Int_t  gain = polya_highgain -> GetRandom();
//      if(fLowAnode)
//	{
//	  if( (layer >=91 && layer <= 98) || layer >=108)
//	    gain = polya_lowgain -> GetRandom(); //low anode sections
//	}
      if(gain<=0) continue;

      STDriftedElectron *electron
        = new ((*fElectronArray)[index])
          STDriftedElectron(fMCPoint->GetX()*10, dx,
                            fMCPoint->GetZ()*10, dz, 
                            fMCPoint->GetY()*10,
                            fMCPoint->GetTime(), tDrift, dt,
                            iWire, zWire,
                            gain);

      electron -> SetIndex(index);
    }
  }

  Int_t nDriftElectrons = fElectronArray->GetEntriesFast();

  fEventID = fFairMCEventHeader -> GetEventID();
  fLogger->Info(MESSAGE_ORIGIN, 
                Form("Event #%d : MC points (%d) found. Drift electrons (%d) created.",
                     fEventID, nMCPoints, nDriftElectrons));


  return;
}

void STDriftTask::SetPersistence(Bool_t value) { fIsPersistence = value; }
void STDriftTask::SetVerbose(Bool_t value) { fVerbose = value; }
void STDriftTask::SetParticleForCorrection(TString value) { fSpecies = value; }
void STDriftTask::SetSplineInterpolation(Bool_t value) { fSpline = value; }
Double_t STDriftTask::BichselCorrection(TString species, Double_t value)
{
  double value2 = value*value;
  double value3 = value2*value;
  double value4 = value3*value;

  if (species == "pi")                                                                                   
    return 1;                                                                                            
  else if(species == "p")                                                                                
  { return 0.907872 + 0.000539408*value - 5.45602e-7*value2 + 2.32446e-10*value3 - 1.50572e-14*value4; }
  else if(species == "d")                                                                                
  { 
    if(value > 336) return (0.264173 - 60.1493*exp(-0.0260451*value))*(3.80426 + 0.000273719*value + 3.22777e-8*value2);
    else return 0.993113;//4.289173 - 0.015664*value + 1.74247e-5*value2; 
  }
  else if(species == "t")                                                                                
  { 
    double factor = (660.941 < value && value < 885.039)? -1.2083 + 0.00583629*value - 3.77514e-6*value2: 1;
    return factor*(0.695761 + 0.000670917*value - 3.48589e-7*value2 + 6.06975e-11*value3 + 7.63133e-16*value4); 
  }
  else if(species == "he3")                                                                              
  { 
    double factor = (516.329 < value/2 && value/2 < 820.706)? (0.21386 + 0.00248044*value/2. - 1.85518e-6*value2/4)*(0.886278 + 0.000136209*value/2) : 1;
    return factor*(1.8109 - 0.0018949*value + 1.5737e-6*value2 - 5.38819e-10*value3 + 6.67678e-14*value4); 
  }
  else if(species == "he4")                                                                              
  {
    if(value > 3600) return 1.20753*0.976371;
    else
    {
      double factor = (value/2 < 1317.8)? 0.703 + 0.0007266*value/2 - 3.80084e-7*value2/4. : 1;
      return factor*(6.34187 - 0.0110978*value + 8.84578e-6*value2 - 3.42063e-9*value3 + 6.5104e-13*value4 - 4.90277e-17*value4*value)*0.976371;
    }
  }
  else{
    std::cout << "It needs implementation, sorry!" << std::endl;
    exit(0);
  }
}

Double_t STDriftTask::BichselCorrection(Int_t pdg, Double_t value)
{
  TString pname;
  if(pdg == 2212) pname = "p";
  else if(fabs(pdg) == 211) pname = "pi";
  else if(pdg == 1000010020) pname = "d";
  else if(pdg == 1000010030) pname = "t";
  else if(pdg == 1000020030) pname = "he3";
  else if(pdg == 1000020040) pname = "he4";
  else return 1.; // return 1. for unknown substance (e.g. e-)

  return this->BichselCorrection(pname, value);
}

ROOT::Math::Interpolator* STDriftTask::BichselCorrection(TString species)
{
  const volatile Int_t n = 75;
  int &nn = const_cast <int &> (n);
  if (species == "pi") nn = 69; 
  
  Double_t betagamma, eloss, pmin, pmax;
  Double_t p, avgdEdx, avgdEdx2, dEdx, dEdx2;
  Double_t pB[n], dEdxB[n], pG[n], dEdxG[n], ratio[n];

  ifstream file;
  if (species == "pi") file.open("../input/bgamma_pid_pions.dat");
  else file.open("../input/bgamma_pid.dat");
    
  ifstream file2; 
  
  // Bichsel file  
  for(Int_t i=0; i<n; i++){
    file>>betagamma>>eloss;
    if (species == "p")
      pB[i]=betagamma*938.27;
    else if(species == "pi")
      pB[i]=betagamma*139.57;
    else if(species == "d")
      pB[i]=betagamma*(938.27+939.57);
    else if(species == "t")
      pB[i]=betagamma*(938.27+2*939.57);
    else{
      std::cout << "It needs implementation, sorry!" << std::endl;
      exit(0);}
    
    dEdxB[i]=eloss/10000; // keV/cm to MeV/mm    

  }
  
  fPmin = pB[0];
  fPmax = pB[n-1];

  // support variables for pion case
  Int_t j=0;
  Double_t pG2[63], ratio2[63];      
  
  // load geant4 file for calculating the ration
  if (species == "p")
    file2.open("../input/geant4protons_75_events.txt");
  else if(species == "pi")
    file2.open("../input/geant4piminus_75_events.txt");  
  else if(species == "d")
    file2.open("../input/geant4deuterons_75_events.txt");      
  else if(species == "t")
    file2.open("../input/geant4tritons_75_events.txt");
  else{
    std::cout << "It needs implementation, sorry!" << std::endl;
    exit(0);}

  for(Int_t i=0; i<n; i++){
    file2>>p>>avgdEdx>>avgdEdx2>>dEdx>>dEdx2;
    pG[i]=p;
    dEdxG[i]=dEdx;
    ratio[i]=dEdxB[i]/dEdxG[i];
    if (species == "pi"){
      if(ratio[i]<1.5){
	pG2[j]=pG[i];
	ratio2[j]=ratio[i];
	j++;                    
      }
    }
  }

  // interpolation
  ROOT::Math::Interpolator* myinter = new ROOT::Math::Interpolator(n, ROOT::Math::Interpolation::kCSPLINE );
  if (species == "pi") myinter->SetData(63, pG2, ratio2);
  else myinter->SetData(n, pG, ratio);

  return myinter;

} 

ClassImp(STDriftTask);
