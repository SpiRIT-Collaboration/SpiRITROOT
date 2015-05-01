#include "Riostream.h"
void Bfield_Map() {
//  Read data from an ascii file without any header and space as delimeter and create a root file with an histograms and an ntuple.
//Author: Spirit
      

// Bfield_Map.dat this file has 6 columns of float data

   TString dir = gSystem->UnixPathName(gInterpreter->GetCurrentMacroName());
   dir.ReplaceAll("Bfield_Map.C","");
   dir.ReplaceAll(",","/");
   ifstream field;
   field.open(Form("%sBfield_map5T.dat",dir.Data()));
//    in.ignore(8,'\n');   

   Double_t x,y,z,Bx,By,Bz;
   Int_t npoints = 0;
   TFile *file = new TFile("Bfield_map_5T.root","RECREATE");
   
  TH1D *x1 = new TH1D("x1","x distribution",1000,-50,50);
  TH1D *y1 = new TH1D("y1","y distribution",1000,-53,1);
  TH1D *z1 = new TH1D("z1","z distribution",1000,0,146);
  TH1D *b1 = new TH1D("b1","Bx distribution",10000,-600,600);
  TH1D *b2 = new TH1D("b2","By distribution",10000,-2600,2600);
  TH1D *b3 = new TH1D("b3","Bz distribution",10000,-700,700);

TNtuple *ntuple = new TNtuple("ntuple","data from ascii file","x:y:z:Bx:By:Bz");

   while (1) {
      field >> x >> y >> z >> Bx >> By >> Bz;
      if (!field.good()) break;
      if (npoints < 4) printf("x=%8f, y=%8f, z=%8f ,Bx=%8f, By=%8f, Bz=%8f\n",x,y,z,Bx,By,Bz);
      
     ntuple->Fill(x,y,z,Bx,By,Bz);
     x1->Fill(x);
     y1->Fill(y);
     z1->Fill(z);
     b1->Fill(Bx);
     b2->Fill(By);
     b3->Fill(Bz);
      npoints++;
   }
   printf(" found %d points\n",npoints);
//ntuple->Draw("y:x:By");

   field.close();

   file->Write();
}
