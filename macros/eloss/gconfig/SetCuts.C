void SetCuts()
{
  TString configDir(gSystem->Getenv("VMCWORKDIR"));
  ifstream cut(configDir+"/macros/eloss/gconfig/SetCuts.dat");

  Int_t pPAIR; cut>>pPAIR;
  Int_t pCOMP; cut>>pCOMP;
  Int_t pPHOT; cut>>pPHOT;
  Int_t pPFIS; cut>>pPFIS;
  Int_t pDRAY; cut>>pDRAY;
  Int_t pANNI; cut>>pANNI;
  Int_t pBREM; cut>>pBREM;
  Int_t pHADR; cut>>pHADR;
  Int_t pMUNU; cut>>pMUNU;
  Int_t pDCAY; cut>>pDCAY;
  Int_t pLOSS; cut>>pLOSS;
  Int_t pMULS; cut>>pMULS;

  Double_t CUT_C; cut>>CUT_C;
  Double_t CUT_B; cut>>CUT_B;
  Double_t CUT_E; cut>>CUT_E;
  Double_t CUT_P; cut>>CUT_P;
  Double_t TOFMAX; cut>>TOFMAX;

  cut.close();

  gMC->SetProcess("PAIR",1); /** pair production **/
  gMC->SetProcess("COMP",1); /** Compton scattering **/
  gMC->SetProcess("PHOT",1); /** photo electric effect **/
  gMC->SetProcess("PFIS",1); /** photofission **/
  gMC->SetProcess("DRAY",0); /** delta-ray **/
  gMC->SetProcess("ANNI",1); /** annihilation **/
  gMC->SetProcess("BREM",1); /** bremsstrahlung **/
  gMC->SetProcess("HADR",1); /** hadronic process **/
  gMC->SetProcess("MUNU",1); /** muon nuclear interaction **/
  gMC->SetProcess("DCAY",1); /** decay **/
  gMC->SetProcess("LOSS",4); /** energy loss **/
  gMC->SetProcess("MULS",1); /** multiple scattering **/
  
  gMC->SetCut("CUTGAM",CUT_C);   /** gammas (GeV)*/
  gMC->SetCut("CUTELE",CUT_E);   /** electrons (GeV)*/
  gMC->SetCut("CUTNEU",CUT_C);   /** neutral hadrons (GeV)*/
  gMC->SetCut("CUTHAD",CUT_C);   /** charged hadrons (GeV)*/
  gMC->SetCut("CUTMUO",CUT_C);   /** muons (GeV)*/
  gMC->SetCut("BCUTE",CUT_B);    /** electron bremsstrahlung (GeV)*/
  gMC->SetCut("BCUTM",CUT_B);    /** muon and hadron bremsstrahlung(GeV)*/ 
  gMC->SetCut("DCUTE",CUT_B);    /** delta-rays by electrons (GeV)*/
  gMC->SetCut("DCUTM",CUT_B);    /** delta-rays by muons (GeV)*/
  gMC->SetCut("PPCUTM",CUT_P);   /** direct pair production by muons (GeV)*/
  gMC->SetCut("TOFMAX",TOFMAX); /**time of flight cut in seconds*/
}
