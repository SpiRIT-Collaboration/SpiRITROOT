#include "TGeoManager.h" 
#include "TGeoMedium.h"
#include "TGeoMaterial.h"
#include "TGeoVolume.h"
#include <vector>

void geomSpiRITSimple()
{
  TString name = "geomSpiRIT";
  TString dir = gSystem -> Getenv("VMCWORKDIR");
  TString dirGeom = dir + "/geometry/";
  TString geoFileName = dirGeom + name + ".root";
  TString geoManFileName = dirGeom + name + ".man.root";

  // ----------------------------------------------------
  //  Dimensions (cm)
  // ----------------------------------------------------
  TVector3 sTOP(300., 300., 300.); ///< size of top volume
  TVector3 sTPC(300., 300., 300.); ///< size of tpc volume
  /* 
   * "Active"
   * Size of gas volume inside the field cage. 
   * Thickness of cage side(tCageFrame) is not included.
   */ 
  Double_t xCage = 96.61;  ///< x-length of cage volume
  Double_t yCage = 51.01;  ///< y-length of cage volume, from cathode top to pad-plane bottom 
  Double_t zCage = 144.64; ///< z-length of cage volume

  Double_t rActiveCorner = 2.8495; ///< radius of active corner
  Double_t tCageFrame = 0.1575;    ///< Thickness of cage frame
  Double_t rCageCorner = rActiveCorner + tCageFrame; ///< radius of field cage corner

  Double_t xActiveC = xCage - 2*rCageCorner; ///< x-length of active volume (=90.96 in Mike's macro)
  Double_t yActiveC = 49.6;                  ///< y-length of acitve volume
  Double_t zActiveC = zCage - 2*rCageCorner; ///< z-length of active volume (=138.96 in Mike's macro)

  Double_t xPadPlane = 86.4;  ///< x-length of pad plane
  Double_t yPadPlane = 0.1;   ///< height of pad plane, TODO : check the value 
  Double_t zPadPlane = 134.4; ///< z-length of pad plane

  // ----------------------------------------------------
  //  Mediums
  // ----------------------------------------------------
  FairGeoLoader *geoLoader = new FairGeoLoader("TGeo", "FairGeoLoader");
  FairGeoInterface *geoIF = geoLoader -> getGeoInterface();
  geoIF -> setMediaFile(dirGeom + "media.geo");
  geoIF -> readMedia();

  FairGeoMedia *media = geoIF -> getMedia();
  FairGeoBuilder *geoBuilder = geoLoader -> getGeoBuilder();
  
  FairGeoMedium *FGMvacuum = media -> getMedium("vacuum");
  FairGeoMedium *FGMp10 = media -> getMedium("p10");
  FairGeoMedium *FGMkapton = media -> getMedium("kapton");
  FairGeoMedium *FGMpcb = media -> getMedium("pcb");
  FairGeoMedium *FGMaluminium = media -> getMedium("aluminium");
  FairGeoMedium *FGMcopper = media -> getMedium("copper");

  geoBuilder -> createMedium(FGMvacuum);
  geoBuilder -> createMedium(FGMp10);
  geoBuilder -> createMedium(FGMkapton);
  geoBuilder -> createMedium(FGMpcb);
  geoBuilder -> createMedium(FGMaluminium);
  geoBuilder -> createMedium(FGMcopper);
  
  gGeoManager = (TGeoManager *) gROOT -> FindObject("FAIRGeom");
  gGeoManager -> SetName("SpiRIT");

  TGeoMedium *vacuum = gGeoManager -> GetMedium("vacuum");
  TGeoMedium *p10 = gGeoManager -> GetMedium("p10");
  TGeoMedium *kapton = gGeoManager -> GetMedium("kapton");
  TGeoMedium *pcb = gGeoManager -> GetMedium("pcb");
  TGeoMedium *aluminium = gGeoManager -> GetMedium("aluminium");
  TGeoMedium *copper = gGeoManager -> GetMedium("copper");
  

  // ----------------------------------------------------
  //  TOP
  // ----------------------------------------------------
  TGeoVolume* top = gGeoManager -> MakeBox("top", vacuum, sTOP.X(), sTOP.Y(), sTOP.Z());
  gGeoManager -> SetTopVolume(top);

  // ----------------------------------------------------
  //  TPC
  // ----------------------------------------------------
  TGeoVolume* tpc = gGeoManager -> MakeBox("tpc", vacuum, sTPC.X(), sTPC.Y(), sTPC.Z());
  TGeoRotation*   rotatTPC = new TGeoRotation("rotatTPC", 180.0, 180.0, 0.0);
  TGeoCombiTrans* combiTPC  = new TGeoCombiTrans("combiTPC", 0.0, -27.5305, 5.08548 + 134.4/2., rotatTPC);

  // ----------------------------------------------------
  //  Active Volume (Union)
  // ----------------------------------------------------
  TGeoVolume* activeCenter  = gGeoManager -> MakeBox("activeCenter", p10, xActiveC/2.,      yActiveC/2., zActiveC/2.);
  TGeoVolume* activeTop     = gGeoManager -> MakeBox("activeTop",    p10, xActiveC/2.,      yActiveC/2., rActiveCorner/2.);
  TGeoVolume* activeBottom  = gGeoManager -> MakeBox("activeBottom", p10, xActiveC/2.,      yActiveC/2., rActiveCorner/2.);
  TGeoVolume* activeRight   = gGeoManager -> MakeBox("activeRight",  p10, rActiveCorner/2., yActiveC/2., zActiveC/2.);
  TGeoVolume* activeLeft    = gGeoManager -> MakeBox("activeLeft",   p10, rActiveCorner/2., yActiveC/2., zActiveC/2.);

  TGeoVolume* activeCorner1 = gGeoManager -> MakeTubs("activeCorner1", p10, 0., rActiveCorner, yActiveC/2., 0,    90. );
  TGeoVolume* activeCorner2 = gGeoManager -> MakeTubs("activeCorner2", p10, 0., rActiveCorner, yActiveC/2., 90.,  180.);
  TGeoVolume* activeCorner3 = gGeoManager -> MakeTubs("activeCorner3", p10, 0., rActiveCorner, yActiveC/2., 180., 270.);
  TGeoVolume* activeCorner4 = gGeoManager -> MakeTubs("activeCorner4", p10, 0., rActiveCorner, yActiveC/2., 270., 360.);

  Double_t offXActive = xActiveC/2. + rActiveCorner/2.;
  Double_t offZActive = zActiveC/2. + rActiveCorner/2.;

  TGeoTranslation* transActiveTop     = new TGeoTranslation("transActiveTop",    0, 0,  offZActive);
  TGeoTranslation* transActiveBottom  = new TGeoTranslation("transActiveBottom", 0, 0, -offZActive);
  TGeoTranslation* transActiveRight   = new TGeoTranslation("transActiveRight",  offXActive, 0, 0);
  TGeoTranslation* transActiveLeft    = new TGeoTranslation("transActiveLeft",  -offXActive, 0, 0);
  TGeoRotation*    rotatCorner        = new TGeoRotation("rotatCorner", 0, 90, 0);
  TGeoCombiTrans*  combiActiveCorner1 = new TGeoCombiTrans("combiActiveCorner1",  xActiveC/2., 0,  zActiveC/2., rotatCorner);
  TGeoCombiTrans*  combiActiveCorner2 = new TGeoCombiTrans("combiActiveCorner2", -xActiveC/2., 0,  zActiveC/2., rotatCorner);
  TGeoCombiTrans*  combiActiveCorner3 = new TGeoCombiTrans("combiActiveCorner3", -xActiveC/2., 0, -zActiveC/2., rotatCorner);
  TGeoCombiTrans*  combiActiveCorner4 = new TGeoCombiTrans("combiActiveCorner4",  xActiveC/2., 0, -zActiveC/2., rotatCorner);

  transActiveTop     -> RegisterYourself();
  transActiveBottom  -> RegisterYourself();
  transActiveRight   -> RegisterYourself();
  transActiveLeft    -> RegisterYourself();
  combiActiveCorner1 -> RegisterYourself();
  combiActiveCorner2 -> RegisterYourself();
  combiActiveCorner3 -> RegisterYourself();
  combiActiveCorner4 -> RegisterYourself();

  TGeoCompositeShape* activeComposite
    = new TGeoCompositeShape("activeComposite", "activeCenter+activeTop:transActiveTop+activeBottom:transActiveBottom+activeRight:transActiveRight+activeLeft:transActiveLeft+activeCorner1:combiActiveCorner1+activeCorner2:combiActiveCorner2+activeCorner3:combiActiveCorner3+activeCorner4:combiActiveCorner4");
  TGeoVolume *active = new TGeoVolume("field_cage_in", activeComposite);
  active -> SetMedium(p10);

  // ----------------------------------------------------
  //  Field Cage Corner Frame
  // ----------------------------------------------------
  TGeoVolume *cageCorner1 = gGeoManager->MakeTubs("cageCorner1", aluminium, rActiveCorner, rCageCorner, yActiveC/2., 0  , 90 );
  TGeoVolume *cageCorner2 = gGeoManager->MakeTubs("cageCorner2", aluminium, rActiveCorner, rCageCorner, yActiveC/2., 90 , 180);
  TGeoVolume *cageCorner3 = gGeoManager->MakeTubs("cageCorner3", aluminium, rActiveCorner, rCageCorner, yActiveC/2., 180, 270);
  TGeoVolume *cageCorner4 = gGeoManager->MakeTubs("cageCorner4", aluminium, rActiveCorner, rCageCorner, yActiveC/2., 270, 360);

  TGeoCompositeShape* cageCornerComposite = new TGeoCompositeShape("cageCornerComposite", "cageCorner1:combiActiveCorner1+cageCorner2:combiActiveCorner2+cageCorner3:combiActiveCorner3+cageCorner4:combiActiveCorner4");
  TGeoVolume *cageCorner = new TGeoVolume("cageCorner", cageCornerComposite);
  cageCorner -> SetMedium(aluminium);

  // ----------------------------------------------------
  //  Field Cage Frame
  // ----------------------------------------------------
  Double_t xCageFrontRL = 40.48;
  Double_t xCageFrontTB = xActiveC - 2*xCageFrontRL;
  Double_t yCageFrontT  = 13.0;
  Double_t yCageFrontB  = 19.6;

  TGeoVolume* cageRight  = gGeoManager -> MakeBox("cageRight" , pcb, tCageFrame/2.,   yActiveC/2.,    zActiveC/2.);
  TGeoVolume* cageLeft   = gGeoManager -> MakeBox("cageLeft"  , pcb, tCageFrame/2.,   yActiveC/2.,    zActiveC/2.);
  TGeoVolume* cageFrontR = gGeoManager -> MakeBox("cageFrontR", pcb, xCageFrontRL/2., yActiveC/2.,    tCageFrame/2.);
  TGeoVolume* cageFrontL = gGeoManager -> MakeBox("cageFrontL", pcb, xCageFrontRL/2., yActiveC/2.,    tCageFrame/2.);
  TGeoVolume* cageFrontT = gGeoManager -> MakeBox("cageFrontT", pcb, xCageFrontTB/2., yCageFrontT/2., tCageFrame/2.);
  TGeoVolume* cageFrontB = gGeoManager -> MakeBox("cageFrontB", pcb, xCageFrontTB/2., yCageFrontB/2., tCageFrame/2.);

  Double_t offXCageSide   =  tCageFrame/2. + xActiveC/2. + rActiveCorner;
  Double_t offZCageFront  =  zActiveC/2. + rActiveCorner + tCageFrame/2.;
  Double_t offXCageFrontR =  xActiveC/2. - xCageFrontRL/2.;
  Double_t offXCageFrontL = -xActiveC/2. + xCageFrontRL/2.;
  Double_t offYCageFrontT =  yActiveC/2. - yCageFrontT/2.;
  Double_t offYCageFrontB = -yActiveC/2. + yCageFrontB/2.;

  TGeoTranslation* transCageRight  = new TGeoTranslation("transCageRight",  offXCageSide,   0, 0);
  TGeoTranslation* transCageLeft   = new TGeoTranslation("transCageLeft",  -offXCageSide,   0, 0);
  TGeoTranslation* transCageFrontR = new TGeoTranslation("transCageFrontR", offXCageFrontR, 0, offZCageFront);
  TGeoTranslation* transCageFrontL = new TGeoTranslation("transCageFrontL", offXCageFrontL, 0, offZCageFront);
  TGeoTranslation* transCageFrontT = new TGeoTranslation("transCageFrontT", 0, offYCageFrontT, offZCageFront);
  TGeoTranslation* transCageFrontB = new TGeoTranslation("transCageFrontB", 0, offYCageFrontB, offZCageFront);

  transCageRight  -> RegisterYourself();
  transCageLeft   -> RegisterYourself();
  transCageFrontR -> RegisterYourself();
  transCageFrontL -> RegisterYourself();
  transCageFrontT -> RegisterYourself();
  transCageFrontB -> RegisterYourself();

  TGeoCompositeShape* cageSideComposite = new TGeoCompositeShape("cageSideComposite", "cageRight:transCageRight+cageLeft:transCageLeft");
  TGeoVolume *cageSide = new TGeoVolume("cageSide", cageSideComposite);
  cageSide -> SetMedium(pcb);

  TGeoCompositeShape* cageFrontComposite = new TGeoCompositeShape("cageFrontComposite", "cageFrontR:transCageFrontR+cageFrontL:transCageFrontL+cageFrontT:transCageFrontT+cageFrontB:transCageFrontB");
  TGeoVolume *cageFront = new TGeoVolume("cageFront", cageFrontComposite);
  cageFront -> SetMedium(pcb);

  // ----------------------------------------------------
  //  Back Window (from Mike's macro)
  // ----------------------------------------------------
  Double_t xBackWindow     = 89.76;
  //Double_t yBackWindow     = 48.2475;
  Double_t yBackWindow     = yActiveC;
  Double_t zBackWindow     = 0.0125;
  //Double_t zBackWindowTop  = 1.27;
  //Double_t zBackWindowTop2 = 0.635;
  Double_t zBackWindowTop  = 0;
  Double_t zBackWindowTop2 = 0;

  TGeoVolume *backWindow = gGeoManager -> MakeBox("backWindow", kapton, xBackWindow/2, yBackWindow/2, zBackWindow/2);
  backWindow -> SetMedium(pcb);

  Double_t transZBackWindow = -zActiveC/2. - rActiveCorner - zBackWindowTop - zBackWindowTop2 - zBackWindow/2.0;
  TGeoTranslation *trbackwindow = new TGeoTranslation(0.0, 0.0, transZBackWindow);
  
  // ----------------------------------------------------
  //  AddNode to TOP
  // ----------------------------------------------------
  top -> AddNode(tpc, 1, combiTPC);
 
  // ----------------------------------------------------
  //  AddNode to TPC
  // ----------------------------------------------------
  tpc -> AddNode(active, 1);
  tpc -> AddNode(cageFront, 1);
  tpc -> AddNode(cageSide, 1);
  tpc -> AddNode(cageCorner, 1);
  tpc -> AddNode(backWindow, 1, trbackwindow);

  // ----------------------------------------------------
  //  Visual Attributes 
  // ----------------------------------------------------
  Int_t transparency = 85;

  active     -> SetVisibility(kFALSE);
  active     -> SetTransparency(transparency);
  cageFront  -> SetTransparency(transparency);
  cageSide   -> SetTransparency(transparency);
  cageCorner -> SetTransparency(transparency);
  backWindow -> SetTransparency(transparency);

  Color_t frontColor = kBlue - 4;
  Color_t cageColor = 18;

  active     -> SetLineColor(kBlue - 4);
  cageFront  -> SetLineColor(frontColor);
  cageSide   -> SetLineColor(cageColor);
  cageCorner -> SetLineColor(cageColor);
  backWindow -> SetLineColor(cageColor);

  // ----------------------------------------------------
  //  End of Building Geometry
  // ----------------------------------------------------
  gGeoManager -> CloseGeometry();
  top->Draw("ogl");

  TFile *geoFile = new TFile(geoFileName, "recreate"); 
  top -> Write(); 
  geoFile -> Close(); 

  TFile *geoManFile = new TFile(geoManFileName, "recreate"); 
  gGeoManager -> Write(); 
  geoManFile -> Close(); 

  cout << geoFileName << " created." << endl;
  cout << geoManFileName << " created." << endl;
}
