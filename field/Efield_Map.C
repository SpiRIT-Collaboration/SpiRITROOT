#include "Riostream.h"
void Efield_Map() {
//  Read data from an ascii file without any header and space as delimeter and create a root file with an histograms and an ntuple.
//Author: Spirit
      

// Efield_Map.dat this file has 6 columns of float data

   TString dir = gSystem->UnixPathName(gInterpreter->GetCurrentMacroName());
   dir.ReplaceAll("Efield_Map.C","");
   dir.ReplaceAll(",","/");
   ifstream field;
   field.open(Form("%selec_new.dat",dir.Data()));
//    in.ignore(8,'\n');   

   Float_t x,y,z,Ex,Ey,Ez;
   Int_t npoints = 0;
   TFile *file = new TFile("elec_shift.root","RECREATE");
   
  TH1F *x1 = new TH1F("x1","x distribution",1000,-50,50);
  TH1F *y1 = new TH1F("y1","y distribution",1000,-53,1);
  TH1F *z1 = new TH1F("z1","z distribution",1000,0,146);
  TH1F *e1 = new TH1F("e1","Ex distribution",10000,-600,600);
  TH1F *e2 = new TH1F("e2","Ey distribution",10000,-2600,2600);
  TH1F *e3 = new TH1F("e3","Ez distribution",10000,-700,700);

TNtuple *ntuple = new TNtuple("ntuple","data from ascii file","x:y:z:Ex:Ey:Ez");

   while (1) {
      field >> x >> y >> z >> Ex >> Ey >> Ez;
      if (!field.good()) break;
 //     if (npoints < 4) printf("x=%8f, y=%8f, z=%8f ,Ex=%8f, Ey=%8f, Ez=%8f\n",x,y,z,Ex,Ey,Ez);
      
     ntuple->Fill(x,-25.6-y,z+72,Ex,Ey,Ez);
     x1->Fill(x);
     y1->Fill(-y-26.6);
     z1->Fill(z+72);
     e1->Fill(Ex);
     e2->Fill(Ey);
     e3->Fill(Ez);
      npoints++;
   }
   printf(" found %d points\n",npoints);
//ntuple->Draw("y:x:Ey");
   field.close();

   file->Write();
}
