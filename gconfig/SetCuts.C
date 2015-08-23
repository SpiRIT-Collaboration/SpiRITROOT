/// Configuration macro for setting common cuts and processes for G3, G4 and Fluka (M. Al-Turany 27.03.2008)
/// specific cuts and processes to g3 or g4 should be set in the g3Config.C, g4Config.C or flConfig.C

void SetCuts()
{
  cout << "SetCuts Macro: Setting Processes.." <<endl;
   
  // ------>>>> IMPORTANT!!!!
  // For a correct comparison between GEANE and MC (pull distributions) 
  // or for a simulation without the generation of secondary particles:
  // 1. set LOSS = 2, DRAY = 0, BREM = 1
  // 2. set the following cut values: CUTGAM, CUTELE, CUTNEU, CUTHAD, CUTMUO = 1 MeV or less
  //                                  BCUTE, BCUTM, DCUTE, DCUTM, PPCUTM = 10 TeV
  // (For an explanation of the chosen values, please refer to the GEANT User's Guide
  // or to message #5362 in the PandaRoot Forum >> Monte Carlo Engines >> g3Config.C thread)
  // 
  // The default settings refer to a complete simulation which generates and follows also the secondary particles.


/*
  gMC->SetProcess("PAIR",1); /// pair production 
  gMC->SetProcess("COMP",1); /// Compton scattering 
  gMC->SetProcess("PHOT",1); /// photo electric effect 
  gMC->SetProcess("PFIS",1); /// photofission 
  gMC->SetProcess("DRAY",1); /// delta-ray 
  gMC->SetProcess("ANNI",1); /// annihilation 
  gMC->SetProcess("BREM",1); /// bremsstrahlung 
  gMC->SetProcess("HADR",1); /// hadronic process 
  gMC->SetProcess("MUNU",1); /// muon nuclear interaction 
  gMC->SetProcess("DCAY",1); /// decay 
  gMC->SetProcess("LOSS",1); /// energy loss 
  gMC->SetProcess("MULS",1); /// multiple scattering 

  Double_t cutC = 1.0E-5;
  //Double_t cutB = 1.E4;
  Double_t cutB = -1;
  Double_t cut1 = 1.0E-8;   // [GeV]
  Double_t cutp = 0.01;     // [GeV]
  Double_t tofmax = 1.E10;  // seconds
  cout << "SetCuts Macro: Setting cuts.." <<endl;
  
  gMC->SetCut("CUTGAM",cutC);   /// gammas (GeV)
  gMC->SetCut("CUTELE",cut1);   /// electrons (GeV)
  gMC->SetCut("CUTNEU",cutC);   /// neutral hadrons (GeV)
  gMC->SetCut("CUTHAD",cutC);   /// charged hadrons (GeV)
  gMC->SetCut("CUTMUO",cutC);   /// muons (GeV)
  gMC->SetCut("BCUTE",cutB);    /// electron bremsstrahlung (GeV)
  gMC->SetCut("BCUTM",cutB);    /// muon and hadron bremsstrahlung(GeV) 
  gMC->SetCut("DCUTE",cutB);    /// delta-rays by electrons (GeV)
  gMC->SetCut("DCUTM",cutB);    /// delta-rays by muons (GeV)
  gMC->SetCut("PPCUTM",cutp);   /// direct pair production by muons (GeV)
  gMC->SetCut("TOFMAX",tofmax); ///time of flight cut in seconds
  */
}
