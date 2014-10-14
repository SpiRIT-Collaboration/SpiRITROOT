{
  gStyle -> SetPadRightMargin(0.15);
  gStyle -> SetOptStat(11);
  gStyle -> SetStatX(0.3);             
  gStyle -> SetStatY(0.9);             

  TFile* file = new TFile("data/spirit.digi.root");
  TDirectory* dir = (TDirectory*) file -> GetDirectory("PadPlane");

  Double_t x, z, zWire;

  /*
  FairRunAna* run = new FairRunAna();
              run -> SetInputFile("data/spirit.digi.root");
              run -> Init();

  FairRootManager* ioman = FairRootManager::Instance();
  TClonesArray* elArray = (TClonesArray *) ioman -> GetObject("STDigitizedElectron"); 
  */

  TTree* tree = (TTree*) file -> Get("cbmsim"); 
  /*
         tree -> SetBranchAddress("STDigitizedElectron.fX",     &x);
         tree -> SetBranchAddress("STDigitizedElectron.fZ",     &z);
         tree -> SetBranchAddress("STDigitizedElectron.fZWire", &zWire);
  */

  //TH2D* histXZr = new TH2D("histXZr",";z (cm);x (cm)", 500,-96.61/2,96.61/2,500,0,144.64);
  //TH2D* histXZw = new TH2D("histXZw",";z (cm);x (cm)", 500,-96.61/2,96.61/2,500,0,144.64);

  //cbmsim -> Draw("STDigitizedElectron.fX:STDigitizedElectron.fZ >> histXZr");
  //cbmsim -> Draw("cbmout.ST.STDigitizedElectron.fX:cbmout.ST.STDigitizedElectron.fZ>>histXZr");
  //cbmsim -> Draw("cbmout.ST.STDigitizedElectron.fX:cbmout.ST.STDigitizedElectron.fZ");

  /*
  Int_t entries = tree -> GetEntries();
  for(Int_t i=0; i<entries; i++)
  {
    histXZr -> Fill(z,x);
    histXZw -> Fill(zWire,x);
  }
  */

  //histXZr -> Draw("colz");
  //histXZw -> Draw("colz");

  TCanvas* cvs1 = new TCanvas("cvs1","",800,600);
  TH2D* hist1 = dir -> FindObjectAny("ElDistXZAval");
        hist1 -> SetNameTitle("pad plane",";z (cm);x (cm)");
        hist1 -> Draw("colz");

        cvs1 -> SaveAs("padplane.pdf");
}
