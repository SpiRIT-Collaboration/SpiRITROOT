// All sizes are given in cm

#include "TSystem.h"
#include "TGeoManager.h"
#include "TGeoVolume.h"
#include "TGeoMaterial.h"
#include "TGeoMedium.h"
#include "TGeoPgon.h"
#include "TGeoMatrix.h"
#include "TGeoCompositeShape.h"
#include "TFile.h"
#include "TString.h"
#include "TList.h"
#include "TROOT.h"

#include <iostream>

// Spirit geometry version and output file
const TString geoVersion = "spirit_v03.1";
const TString FileName = geoVersion + ".root";
const TString FileName1 = geoVersion + "_geomanager.root";

// Names of the different used materials and the materials are defined in the global media.geo file 
const TString TrueVolumeMedium     = "p10";
const TString CageVolumeMedium         = "copper";

// some global variables
TGeoManager* gGeoMan = new TGeoManager("SPiRIT","SPiRIT");  // Pointer to TGeoManager instance
TGeoVolume* gModules; // Global storage for module types

// Forward declarations
void create_materials_from_media_file();
TGeoVolume* create_detector();

void Spirit_Geom_v2_1() {
  gROOT->LoadMacro("$VMCWORKDIR/gconfig/basiclibs.C");
  basiclibs();
  gSystem->Load("libGeoBase");
  gSystem->Load("libParBase");
  gSystem->Load("libBase");

  // Load needed material definition from media.geo file
  create_materials_from_media_file();

  // Get the GeoManager for later usage
  gGeoMan = (TGeoManager*) gROOT->FindObject("FAIRGeom");
  gGeoMan->SetVisLevel(7);  

  // Create the top volume 

  TGeoVolume* top = new TGeoVolumeAssembly("top");
  gGeoMan->SetTopVolume(top);
 
  TGeoMedium* gas   = gGeoMan->GetMedium(TrueVolumeMedium);
  TGeoVolume* tpc = new TGeoVolumeAssembly(geoVersion);
  tpc -> SetMedium(gas);
  top->AddNode(tpc, 1);
 
  gModules = create_detector();

  cout<<"Voxelizing."<<endl;
  top->Voxelize("");
  gGeoMan->CloseGeometry();

  gGeoMan->CheckOverlaps(0.001);
  gGeoMan->PrintOverlaps();
  gGeoMan->Test();

  TFile* outfile = new TFile(FileName,"RECREATE");
  top->Write();
  outfile->Close();

  TFile* outfile = new TFile(FileName1,"RECREATE");
  gGeoMan->Write();
  outfile->Close();

    top->Draw("ogl");
  //top->Raytrace();

}

void create_materials_from_media_file()
{
  // Use the FairRoot geometry interface to load the media which are already defined
  FairGeoLoader* geoLoad = new FairGeoLoader("TGeo", "FairGeoLoader");
  FairGeoInterface* geoFace = geoLoad->getGeoInterface();
  TString geoFile = "media.geo";
  geoFace->setMediaFile(geoFile);
  geoFace->readMedia();

  // Read the required media and create them in the GeoManager
  FairGeoMedia* geoMedia = geoFace->getMedia();
  FairGeoBuilder* geoBuild = geoLoad->getGeoBuilder();

  FairGeoMedium* air      = geoMedia->getMedium("air");
  FairGeoMedium* vacuum   = geoMedia->getMedium("vacuum");
  FairGeoMedium* p10      = geoMedia->getMedium("p10");
  FairGeoMedium* copper   = geoMedia->getMedium("copper");
  FairGeoMedium* aluminium = geoMedia->getMedium("aluminium");

  // include check if all media are found

  geoBuild->createMedium(air);
  geoBuild->createMedium(vacuum);
  geoBuild->createMedium(p10);
  geoBuild->createMedium(copper);
  geoBuild->createMedium(aluminium);
//  geoBuild->createMedium(zinc);
}


TGeoVolume* create_detector()
{
  // Obtain needed materials
  TGeoMedium* CageVolMed_in   = gGeoMan->GetMedium(CageVolumeMedium);
  TGeoMedium* gas   = gGeoMan->GetMedium(TrueVolumeMedium);

  Double_t pad_x=96.61;
   Double_t drift_y=51.01;
   Double_t beam_z=144.64;


//   TGeoVolume *field_cage_out = gGeoManager->MakeBox("field_cage_out", CageVolMed_in, (pad_x+10.30)/2, (drift_y+0.18)/2, (beam_z+5)/2);
// gGeoMan->GetVolume(geoVersion)->AddNode(field_cage_out,1);


   TGeoVolume *field_cage_in = gGeoManager->MakeBox("field_cage_in", gas,  pad_x/2, drift_y/2, beam_z/2);

 // gGeoMan->GetVolume(geoVersion)->AddNode(field_cage_in,1);
   //field_cage_in2->SetLineColor(kRed);


   TGeoVolume *pad = gGeoManager->MakeBox("pad", CageVolMed_in, pad_x/2, 0.18/2, beam_z/2);
  gGeoMan->GetVolume(geoVersion)->AddNode(pad,1, new TGeoTranslation(0,0.22,(beam_z)/2));
  pad->SetLineColor(kBlue);


   TGeoVolume *anode = gGeoManager->MakeBox("anode", CageVolMed_in, pad_x/2, 0.1/2, beam_z/2);
     gGeoMan->GetVolume(geoVersion)->AddNode(anode,1, new TGeoTranslation(0,-0.42,(beam_z)/2));
       anode->SetLineColor(kRed);

   TGeoVolume *ground = gGeoManager->MakeBox("ground", CageVolMed_in, pad_x/2, 0.1/2, beam_z/2);
     gGeoMan->GetVolume(geoVersion)->AddNode(ground,1, new TGeoTranslation(0,-0.82,(beam_z)/2));
       ground->SetLineColor(kMagenta);

//    TGeoCompositeShape *cs1 = new TGeoCompositeShape("cs1","(field_cage_out)-(field_cage_in1)"); 
//      TGeoVolume *comp1 = new TGeoVolume("field_cage_in",cs1,CageVolMed_in);
//            comp1->SetLineColor(kBlue);
  gGeoMan->GetVolume(geoVersion)->AddNode(field_cage_in,1, new TGeoTranslation(0,-drift_y/2,(beam_z)/2));
 
  return field_cage_in;
}
