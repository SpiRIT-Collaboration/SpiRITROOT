
/** Configuration macro for setting common cuts and processes for G3, G4 and Fluka (M. Al-Turany 27.03.2008)
    specific cuts and processes to g3 or g4 should be set in the g3Config.C, g4Config.C or flConfig.C

*/

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


  gMC->SetProcess("PAIR",0); /** pair production **/
  gMC->SetProcess("COMP",0); /** Compton scattering **/
  gMC->SetProcess("PHOT",0); /** photo electric effect **/
  gMC->SetProcess("PFIS",0); /** photofission **/
  gMC->SetProcess("DRAY",0); /** delta-ray **/
  gMC->SetProcess("ANNI",0); /** annihilation **/
  gMC->SetProcess("BREM",0); /** bremsstrahlung **/
  gMC->SetProcess("HADR",1); /** hadronic process **/
  gMC->SetProcess("MUNU",0); /** muon nuclear interaction **/
  gMC->SetProcess("DCAY",0); /** decay **/
  gMC->SetProcess("LOSS",1); /** energy loss **/
  gMC->SetProcess("MULS",0); /** multiple scattering **/

  Double_t cut0 = 1.0E-6;         // [GeV]
  Double_t cut1 = 1.0E-7;         // [GeV]
  Double_t cutp = 2.0E-3;         // [GeV]
  Double_t tofmax = 1.E10;        // seconds
  cout << "SetCuts Macro: Setting cuts.." <<endl;
  
  gMC->SetCut("CUTGAM",cut0);   /** gammas (GeV)*/
  gMC->SetCut("CUTELE",cut1);   /** electrons (GeV)*/
  gMC->SetCut("CUTNEU",cut0);   /** neutral hadrons (GeV)*/
  gMC->SetCut("CUTHAD",cut1);   /** charged hadrons (GeV)*/
  gMC->SetCut("CUTMUO",cut0);   /** muons (GeV)*/
  gMC->SetCut("BCUTE",cut0);    /** electron bremsstrahlung (GeV)*/
  gMC->SetCut("BCUTM",cut0);    /** muon and hadron bremsstrahlung(GeV)*/ 
  gMC->SetCut("DCUTE",cut0);    /** delta-rays by electrons (GeV)*/
  gMC->SetCut("DCUTM",cut0);    /** delta-rays by muons (GeV)*/
  gMC->SetCut("PPCUTM",cutp);   /** direct pair production by muons (GeV)*/
  gMC->SetCut("TOFMAX",tofmax); /**time of flight cut in seconds*/
}
