#include "TGeoManager.h"
#include "TGeoMedium.h"
#include "TGeoMaterial.h"
#include "TGeoVolume.h"
#include <vector>

/*
 * SpiRIT-TPC geometry macro
 *
 * List of volumes
 * - Top
 *   - TPC
 *     - active            : TGeoCompositeShape
 *     - cageFront         : TGeoCompositeShape
 *     - cageSide          : TGeoCompositeShape
 *     - frontWindow       : TGeoVolume
 *     - frontWindowFrame  : TGeoCompositeShape
 *     - frontWindowCradle : TGeoCompositeShape
 *     - bottomPlate       : TGeoCompositeShape
 *     - backWindowFrame   : TGeoCompositeShape
 *     - backWindow        : TGeoVolume
 *     - topFrame          : TGeoCompositeShape
 *     - topPlate          : TGeoVolume
 *     - padPlane          : TGeoVolume
 *     - padArray          : TGeoVolume
*/

/*
 * All dimension units in [mm].
 * Variable name that start with dx(dy,dz,dxy)   mean length in x(y,z,x&y).
 * Variable name that start with offx(offy,offz) mean offset in x(y,z).
 * Variable name that start with dw              mean thickness or width.
 * Variable name that start with r               mean radius.
 * T, B, L, R, S, F, B stands for Top, Bottom, Left, Right, Side, Front, Bottom
*/

/*
 * TODO : check value
 * TODO : overlap between topPlate and (padPlane, padArray)
 * and more TODO inside the code
*/

void geomSpiRIT()
{
  TString dir = gSystem -> Getenv("VMCWORKDIR");
  TString dirGeom = dir + "/geometry/";

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
  FairGeoMedium *FGMpolycarb = media -> getMedium("polycarb");

  geoBuilder -> createMedium(FGMvacuum);
  geoBuilder -> createMedium(FGMp10);
  geoBuilder -> createMedium(FGMkapton);
  geoBuilder -> createMedium(FGMpcb);
  geoBuilder -> createMedium(FGMaluminium);
  geoBuilder -> createMedium(FGMcopper);
  geoBuilder -> createMedium(FGMpolycarb);
  
  gGeoManager = (TGeoManager *) gROOT -> FindObject("FAIRGeom");
  gGeoManager -> SetName("SpiRIT");

  TGeoMedium *vacuum = gGeoManager -> GetMedium("vacuum");
  TGeoMedium *p10 = gGeoManager -> GetMedium("p10");
  TGeoMedium *kapton = gGeoManager -> GetMedium("kapton");
  TGeoMedium *pcb = gGeoManager -> GetMedium("pcb");
  TGeoMedium *aluminium = gGeoManager -> GetMedium("aluminium");
  TGeoMedium *copper = gGeoManager -> GetMedium("copper");
  TGeoMedium *polycarb = gGeoManager -> GetMedium("polycarb");

  // ----------------------------------------------------
  //  Dimensions (cm)
  // ----------------------------------------------------

  // Active : Gas volume inside the field cage
  Double_t dxActive = 96.61;  // x-length of active volume
  Double_t dyActive = 49.6;   // y-length of active volume, from cathode top to pad-plane bottom 
  Double_t dzActive = 144.64; // z-length of active volume

  // Pad Plane dimension
  Double_t dxPadPlane = 86.4;
  Double_t dzPadPlane = 134.4;

  // Top Plate dimension including top-frame, pad-plane and some part of active area
  Double_t dxTopPlate = 152.4;
  Double_t dzTopPlate = 206.06;
  Double_t dyTopPlate = 10.9042;

  Double_t dyBottomPlate = 1.0541; // thickness of the bottom plate

  // For additional active volume just below pad plane. Middle is below Top.
  Double_t dyActiveMiddle = 0.8763;  // TODO check this value
  Double_t dyActiveTop = 1.8;        // TODO original value in macro was 1.905
  Double_t dyActiveTopMiddle = dyActiveMiddle + dyActiveTop;

  Double_t dzActiveMiddle = 143.899; // TODO Should be smaller than dzAtive. original value in macro was 143.899
  Double_t dzActiveTop = 155.7;

  // Tpc (World) dimension
  Double_t dxTpc = 200.;
  Double_t dyTpc = dyActive + dyTopPlate + dyBottomPlate + dyActiveTopMiddle; // = 64.2346
  Double_t dzTpc = dzTopPlate; // = 206.06

  Double_t dxTop = dxTpc + 10;
  Double_t dyTop = dyTpc + 10;
  Double_t dzTop = dzTpc + 10;

  cout << "Tpc volume: " << dxTpc << ", " << dyTpc << ", " << dzTpc << endl;
  cout << "Top volume: " << dxTop << ", " << dyTop << ", " << dzTop << endl;

  Double_t dxTopFrameFloorSide = 17.5326;
  Double_t dxTopFrameFloorFB = 95.8523;
  Double_t dzTopFrameFloorFB = 8.0594;
  Double_t dzTopFrameFloorSide = dzActiveMiddle + 2 * dzTopFrameFloorFB; // was Double_t dzTopFrameFloorSide = 160.0175;

  Double_t dxTopFrameLexanFB = 126.6;
  Double_t dxTopFrameLexanSide = 2.;
  Double_t dzTopFrameLexanFB = 2.;
  Double_t dzTopFrameLexanSide = dzActiveTop + 2 * dzTopFrameLexanFB; // was Double_t dzTopFrameLexanSide = 159.7;

  // y-offset of origin from center of active volume. TODO check value
  Double_t offyActiveToOrigin = dyActive/2 + dyActiveMiddle + dyActiveTop; // was Double_t offyActiveToOrigin = 27.5305; 

  Double_t dwCage = 0.1575; // thickness of field cage frame
  Double_t rActiveCorner = 2.8495; // radius of active corner
  Double_t rCageCorner = rActiveCorner + dwCage; // radius of outer field cage corner

  Double_t dxActiveIn = dxActive - 2 * rCageCorner; // x-length of active volume
  Double_t dzActiveIn = dzActive - 2 * rCageCorner; // z-length of active volume

  // origin is at the front(z) and xy-center of the pad plane.
  Double_t offzPadPlane = 5.08548; // distance between active volume center and pad plane center. TODO check value 
  Double_t offzTpc = offzPadPlane + dzPadPlane/2; // z-distance between active volume and origin
  Double_t offyTpc = (dyTopPlate +  dyActiveTopMiddle - dyBottomPlate)/2;
  Double_t offyTop = offyTpc - offyActiveToOrigin;
  Double_t offsetTop[] = {0,offyTop,offzTpc};
  Double_t offsetTpc[] = {0,offyTpc,0};

  // ----------------------------------------------------
  //  Top (World)
  // ----------------------------------------------------

  auto boxTop = new TGeoBBox(dxTop/2,dyTop/2,dzTop/2,offsetTop);
  TGeoVolume *top;
  if (boxTop -> IsRunTimeShape()) {
    top = gGeoManager -> MakeVolumeMulti("top", vacuum);
    top -> SetShape(boxTop);
  } else {
    top = new TGeoVolume("top", boxTop, vacuum);
  }
  gGeoManager -> SetTopVolume(top);
  gGeoManager -> SetTopVolume(top);

  // ----------------------------------------------------
  //  TPC
  // ----------------------------------------------------

  auto boxTpc = new TGeoBBox(dxTpc/2,dyTpc/2,dzTpc/2,offsetTpc);
  TGeoVolume *tpc;
  if (boxTpc -> IsRunTimeShape()) {
    tpc = gGeoManager -> MakeVolumeMulti("tpc", vacuum);
    tpc -> SetShape(boxTpc);
  } else {
    tpc = new TGeoVolume("tpc", boxTpc, vacuum);
  }

  TGeoRotation*   rotatTpc = new TGeoRotation("rotatTpc",180,180,0);
  TGeoCombiTrans* combiTpc = new TGeoCombiTrans("combiTpc",0,-offyActiveToOrigin,offzTpc,rotatTpc);

  // ----------------------------------------------------
  //  Active Volume (Union)
  // ----------------------------------------------------

  TGeoVolume* activeCenter  = gGeoManager -> MakeBox("activeCenter",p10,dxActiveIn/2,dyActive/2,dzActiveIn/2);
  TGeoVolume* activeFront   = gGeoManager -> MakeBox("activeFront", p10,dxActiveIn/2,dyActive/2,rActiveCorner/2);
  TGeoVolume* activeBack    = gGeoManager -> MakeBox("activeBack",  p10,dxActiveIn/2,dyActive/2,rActiveCorner/2);
  TGeoVolume* activeRight   = gGeoManager -> MakeBox("activeRight", p10,rActiveCorner/2,dyActive/2,dzActiveIn/2);
  TGeoVolume* activeLeft    = gGeoManager -> MakeBox("activeLeft",  p10,rActiveCorner/2,dyActive/2,dzActiveIn/2);
  TGeoVolume* activeCorner1 = gGeoManager -> MakeTubs("activeCorner1",p10,0.,rActiveCorner,dyActive/2,  0., 90.);
  TGeoVolume* activeCorner2 = gGeoManager -> MakeTubs("activeCorner2",p10,0.,rActiveCorner,dyActive/2, 90.,180.);
  TGeoVolume* activeCorner3 = gGeoManager -> MakeTubs("activeCorner3",p10,0.,rActiveCorner,dyActive/2,180.,270.);
  TGeoVolume* activeCorner4 = gGeoManager -> MakeTubs("activeCorner4",p10,0.,rActiveCorner,dyActive/2,270.,360.);

  Double_t offxActive = dxActiveIn/2 + rActiveCorner/2;
  Double_t offzActive = dzActiveIn/2 + rActiveCorner/2;

  TGeoTranslation* transActiveFront   = new TGeoTranslation("transActiveFront",0,0, offzActive);
  TGeoTranslation* transActiveBack    = new TGeoTranslation("transActiveBack", 0,0,-offzActive);
  TGeoTranslation* transActiveRight   = new TGeoTranslation("transActiveRight", offxActive,0,0);
  TGeoTranslation* transActiveLeft    = new TGeoTranslation("transActiveLeft", -offxActive,0,0);
  TGeoRotation*    rotatCorner        = new TGeoRotation("rotatCorner",0,90,0);
  TGeoCombiTrans*  combiActiveCorner1 = new TGeoCombiTrans("combiActiveCorner1", dxActiveIn/2,0, dzActiveIn/2,rotatCorner);
  TGeoCombiTrans*  combiActiveCorner2 = new TGeoCombiTrans("combiActiveCorner2",-dxActiveIn/2,0, dzActiveIn/2,rotatCorner);
  TGeoCombiTrans*  combiActiveCorner3 = new TGeoCombiTrans("combiActiveCorner3",-dxActiveIn/2,0,-dzActiveIn/2,rotatCorner);
  TGeoCombiTrans*  combiActiveCorner4 = new TGeoCombiTrans("combiActiveCorner4", dxActiveIn/2,0,-dzActiveIn/2,rotatCorner);

  transActiveFront   -> RegisterYourself();
  transActiveBack    -> RegisterYourself();
  transActiveRight   -> RegisterYourself();
  transActiveLeft    -> RegisterYourself();
  combiActiveCorner1 -> RegisterYourself();
  combiActiveCorner2 -> RegisterYourself();
  combiActiveCorner3 -> RegisterYourself();
  combiActiveCorner4 -> RegisterYourself();

  // Addition to Active Volume Below pad plane
  // Middle is below Top. Middle_y = TopFrameFloor_y, Top_y = TopFrameLexan_y,

  Double_t offyActiveMiddle = (dyActive + dyActiveMiddle)/2;
  Double_t offyActiveTop = offyActiveMiddle + dyActiveTopMiddle/2;
  Double_t offzActiveMiddle = -(dzActive - dzActiveMiddle)/2;

  TGeoVolume *activeMiddle = gGeoManager -> MakeBox("activeMiddle",p10,dxTopFrameFloorFB/2,dyActiveMiddle/2,dzActiveMiddle/2);
  TGeoVolume *activeTop = gGeoManager -> MakeBox("activeTop",p10,dxTopFrameLexanFB/2,dyActiveTop/2,dzActiveTop/2);

  TGeoTranslation *transActiveMiddle = new TGeoTranslation("transActiveMiddle",0,offyActiveMiddle,offzActiveMiddle);
  TGeoTranslation *transActiveTop = new TGeoTranslation("transActiveTop",0,offyActiveTop,0);

  transActiveMiddle -> RegisterYourself();
  transActiveTop -> RegisterYourself();

  TGeoCompositeShape* activeComposite
    = new TGeoCompositeShape("activeComposite","activeCenter+activeFront:transActiveFront+activeBack:transActiveBack+activeRight:transActiveRight+activeLeft:transActiveLeft+activeCorner1:combiActiveCorner1+activeCorner2:combiActiveCorner2+activeCorner3:combiActiveCorner3+activeCorner4:combiActiveCorner4+activeMiddle:transActiveMiddle+activeTop:transActiveTop");
  TGeoVolume *active = new TGeoVolume("field_cage_in",activeComposite);


  // ----------------------------------------------------
  //  Field Cage Corner Frame
  // ----------------------------------------------------

  TGeoVolume *cageCorner1 = gGeoManager -> MakeTubs("cageCorner1",pcb,rActiveCorner,rCageCorner,dyActive/2,0  ,90 );
  TGeoVolume *cageCorner2 = gGeoManager -> MakeTubs("cageCorner2",pcb,rActiveCorner,rCageCorner,dyActive/2,90 ,180);
  TGeoVolume *cageCorner3 = gGeoManager -> MakeTubs("cageCorner3",pcb,rActiveCorner,rCageCorner,dyActive/2,180,270);
  TGeoVolume *cageCorner4 = gGeoManager -> MakeTubs("cageCorner4",pcb,rActiveCorner,rCageCorner,dyActive/2,270,360);

  // ----------------------------------------------------
  //  Field Cage Frame
  // ----------------------------------------------------

  Double_t dxCageFrontRL = 40.48;
  Double_t dxCageFrontTB = dxActiveIn - 2*dxCageFrontRL;
  Double_t dyCageFrontT  = 13.0;
  Double_t dyCageFrontHole = 17;
  Double_t dyCageFrontB  = dyActive - dyCageFrontT - dyCageFrontHole;

  TGeoVolume* cageRight  = gGeoManager -> MakeBox("cageRight" ,pcb,dwCage/2,dyActive/2,dzActiveIn/2);
  TGeoVolume* cageLeft   = gGeoManager -> MakeBox("cageLeft"  ,pcb,dwCage/2,dyActive/2,dzActiveIn/2);
  TGeoVolume* cageFrontR = gGeoManager -> MakeBox("cageFrontR",pcb,dxCageFrontRL/2,dyActive/2,dwCage/2);
  TGeoVolume* cageFrontL = gGeoManager -> MakeBox("cageFrontL",pcb,dxCageFrontRL/2,dyActive/2,dwCage/2);
  TGeoVolume* cageFrontT = gGeoManager -> MakeBox("cageFrontT",pcb,dxCageFrontTB/2,dyCageFrontT/2,dwCage/2);
  TGeoVolume* cageFrontB = gGeoManager -> MakeBox("cageFrontB",pcb,dxCageFrontTB/2,dyCageFrontB/2,dwCage/2);

  Double_t offxCageSide   =  dwCage/2 + dxActiveIn/2 + rActiveCorner;
  Double_t offzCageFront  =  dzActiveIn/2 + rActiveCorner+dwCage/2;
  Double_t offxCageFrontR =  dxActiveIn/2 - dxCageFrontRL/2;
  Double_t offxCageFrontL = -dxActiveIn/2 + dxCageFrontRL/2;
  Double_t offyCageFrontT =  dyActive/2 - dyCageFrontT/2;
  Double_t offyCageFrontB = -dyActive/2 + dyCageFrontB/2;

  TGeoTranslation* transCageRight  = new TGeoTranslation("transCageRight", offxCageSide,0,0);
  TGeoTranslation* transCageLeft   = new TGeoTranslation("transCageLeft", -offxCageSide,0,0);
  TGeoTranslation* transCageFrontR = new TGeoTranslation("transCageFrontR",offxCageFrontR,0,offzCageFront);
  TGeoTranslation* transCageFrontL = new TGeoTranslation("transCageFrontL",offxCageFrontL,0,offzCageFront);
  TGeoTranslation* transCageFrontT = new TGeoTranslation("transCageFrontT",0,offyCageFrontT,offzCageFront);
  TGeoTranslation* transCageFrontB = new TGeoTranslation("transCageFrontB",0,offyCageFrontB,offzCageFront);

  transCageRight  -> RegisterYourself();
  transCageLeft   -> RegisterYourself();
  transCageFrontR -> RegisterYourself();
  transCageFrontL -> RegisterYourself();
  transCageFrontT -> RegisterYourself();
  transCageFrontB -> RegisterYourself();

  TGeoCompositeShape* cageSideComposite = new TGeoCompositeShape("cageSideComposite","cageRight:transCageRight+cageLeft:transCageLeft+cageCorner1:combiActiveCorner1+cageCorner2:combiActiveCorner2+cageCorner3:combiActiveCorner3+cageCorner4:combiActiveCorner4");
  TGeoVolume *cageSide = new TGeoVolume("cageSide",cageSideComposite);

  TGeoCompositeShape* cageFrontComposite = new TGeoCompositeShape("cageFrontComposite","cageFrontT:transCageFrontT+cageFrontB:transCageFrontB+cageFrontR:transCageFrontR+cageFrontL:transCageFrontL");
  TGeoVolume *cageFront = new TGeoVolume("cageFront",cageFrontComposite);

  // ----------------------------------------------------
  //  Front Widow & Front Window Frame
  // ----------------------------------------------------

  Double_t yGapFrontWindowToCage = 0.05;

  Double_t dwFrontWindowFrame = 1.45;
  Double_t dyFrontWindowFrame = dyActive - dyCageFrontT - dyCageFrontB - 2*yGapFrontWindowToCage;
  Double_t dxFrontWindowFrame = dyFrontWindowFrame/2;// - dwFrontWindowFrame;

  Double_t dxyFrontWindow = dyFrontWindowFrame/2 - dwFrontWindowFrame;
  Double_t dxFrontWindowIn = 5.73;
  Double_t dzFrontWindow = 0.005;

  Double_t offyFrontWindow = offyCageFrontT - dyCageFrontT/2 - yGapFrontWindowToCage - dwFrontWindowFrame - dxyFrontWindow/2;
  Double_t offzFrontWindow = dzActiveIn/2 + rActiveCorner + dzFrontWindow/2;

  Double_t offxFrontWindowFrameS = (dwFrontWindowFrame + dxyFrontWindow)/2;
  Double_t offyFrontWindowFrame  = offyCageFrontT - dyCageFrontT/2 - yGapFrontWindowToCage - dyFrontWindowFrame/2;
  Double_t offyFrontWindowFrameT = offyCageFrontT - dyCageFrontT/2 - yGapFrontWindowToCage - dwFrontWindowFrame/2;
  Double_t offyFrontWindowFrameB = offyCageFrontT - dyCageFrontT/2 - yGapFrontWindowToCage - dwFrontWindowFrame - dxyFrontWindow - dxFrontWindowFrame/2;

  Double_t dxFrontWindowInnerFrame = (dxyFrontWindow - dxFrontWindowIn)/2;
  Double_t thetaFrontWindowFrame2 = TMath::RadToDeg()*TMath::ATan(dxFrontWindowInnerFrame/2/dzFrontWindow);
  Double_t offxFrontWindowFrame2 = 0.16 - 3.50125; // TODO ???

  TGeoVolume *FWFR = gGeoManager -> MakeBox("FWFR",polycarb,dwFrontWindowFrame/2,dyFrontWindowFrame/2,dzFrontWindow/2);
  TGeoVolume *FWFL = gGeoManager -> MakeBox("FWFL",polycarb,dwFrontWindowFrame/2,dyFrontWindowFrame/2,dzFrontWindow/2);
  TGeoVolume *FWFT = gGeoManager -> MakeBox("FWFT",polycarb,dxyFrontWindow/2,dwFrontWindowFrame/2,dzFrontWindow/2);
  TGeoVolume *FWFB = gGeoManager -> MakeBox("FWFB",polycarb,dxyFrontWindow/2,dxFrontWindowFrame/2,dzFrontWindow/2);
  TGeoVolume *FWF2R = gGeoManager -> MakeTrap("FWF2R",polycarb,dzFrontWindow/2,thetaFrontWindowFrame2,0,dxyFrontWindow/2,
                                                                                          dxFrontWindowInnerFrame/2,dxFrontWindowInnerFrame/2,0,
                                                                                          dxyFrontWindow/2,0,0,0);
  TGeoVolume *FWF2L = gGeoManager -> MakeTrap("FWF2L",polycarb,dzFrontWindow/2,-thetaFrontWindowFrame2,0,dxyFrontWindow/2,
                                                                                          dxFrontWindowInnerFrame/2,dxFrontWindowInnerFrame/2,0,
                                                                                          dxyFrontWindow/2,0,0,0);

  TGeoTranslation *transFWFL = new TGeoTranslation("transFWFL", offxFrontWindowFrameS,offyFrontWindowFrame,offzFrontWindow);
  TGeoTranslation *transFWFR = new TGeoTranslation("transFWFR",-offxFrontWindowFrameS,offyFrontWindowFrame,offzFrontWindow);
  TGeoTranslation *transFWFT = new TGeoTranslation("transFWFT",0,offyFrontWindowFrameT,offzFrontWindow);
  TGeoTranslation *transFWFB = new TGeoTranslation("transFWFB",0,offyFrontWindowFrameB,offzFrontWindow);
  TGeoTranslation *transFWTL = new TGeoTranslation("transFWF2L", offxFrontWindowFrame2,offyFrontWindow,offzFrontWindow);
  TGeoTranslation *transFWTR = new TGeoTranslation("transFWF2R",-offxFrontWindowFrame2,offyFrontWindow,offzFrontWindow);

  transFWFL -> RegisterYourself();
  transFWFR -> RegisterYourself();
  transFWFT -> RegisterYourself();
  transFWFB -> RegisterYourself();
  transFWTL -> RegisterYourself();
  transFWTR -> RegisterYourself();

  TGeoVolume *frontWindow = gGeoManager -> MakeTrd1("frontWindow",kapton,dxFrontWindowIn/2,dxyFrontWindow/2,dxyFrontWindow/2,dzFrontWindow/2);
  TGeoTranslation *transFrontWindow = new TGeoTranslation("transFrontWindow",0,offyFrontWindow,offzFrontWindow);

  TGeoCompositeShape* frontWindowFrameComposite = new TGeoCompositeShape("FWFComposite","FWFR:transFWFR+FWFL:transFWFL+FWFT:transFWFT+FWFB:transFWFB+FWF2R:transFWF2R+FWF2L:transFWF2L");
  TGeoVolume *frontWindowFrame = new TGeoVolume("frontWindowFrame",frontWindowFrameComposite);

  // ----------------------------------------------------
  //  Front Window Cradle
  // ----------------------------------------------------

  Double_t dwFrontWindowCradle = 5.31;
  Double_t dzFrontWindowCradle = 1.27;
  Double_t dxFrontWindowCradle = 17.62;
  Double_t dyFrontWindowCradle = 14.0;

  Double_t dxFrontWindowCradleTB = 6.35;
  Double_t dyFrontWindowCradleTop = 9.19;
  Double_t dyFrontWindowCradleBottom = dyActive - dyFrontWindowCradle - 2*dwFrontWindowCradle - dyFrontWindowCradleTop;

  Double_t offyFrontWindowCradleTop = dyActive/2. - dyFrontWindowCradleTop/2.;
  Double_t offzFrontWindowCradle = dzActiveIn/2. + rActiveCorner + dwCage + dzFrontWindowCradle/2.;
  Double_t offyFrontWindowCradleBottom = -dyActive/2. + dyFrontWindowCradleBottom/2.;

  // middle bottom, middle top, middle side
  Double_t offyFrontWindowCradleMB = offyFrontWindowCradleBottom + dyFrontWindowCradleBottom/2. + dwFrontWindowCradle/2.;
  Double_t offyFrontWindowCradleMT = offyFrontWindowCradleTop - dyFrontWindowCradleTop/2. - dwFrontWindowCradle/2.;
  Double_t offxFrontWindowCradleMS = dxFrontWindowCradle/2. - dwFrontWindowCradle/2.;
  Double_t offyFrontWindowCradleMS = dyActive/2. - dyFrontWindowCradleTop - dwFrontWindowCradle - dyFrontWindowCradle/2.;

  TGeoVolume *FWCT  = gGeoManager -> MakeBox("FWCT",polycarb,dxFrontWindowCradleTB/2.,dyFrontWindowCradleTop/2.,dzFrontWindowCradle/2.);
  TGeoVolume *FWCB  = gGeoManager -> MakeBox("FWCB",polycarb,dxFrontWindowCradleTB/2.,dyFrontWindowCradleBottom/2.,dzFrontWindowCradle/2.);
  TGeoVolume *FWCMB = gGeoManager -> MakeBox("FWCMB",polycarb,dxFrontWindowCradle/2.,dwFrontWindowCradle/2.,dzFrontWindowCradle/2.);
  TGeoVolume *FWCMT = gGeoManager -> MakeBox("FWCMT",polycarb,dxFrontWindowCradle/2.,dwFrontWindowCradle/2.,dzFrontWindowCradle/2.);
  TGeoVolume *FWCML = gGeoManager -> MakeBox("FWCML",polycarb,dwFrontWindowCradle/2.,dyFrontWindowCradle/2.,dzFrontWindowCradle/2.);
  TGeoVolume *FWCMR = gGeoManager -> MakeBox("FWCMR",polycarb,dwFrontWindowCradle/2.,dyFrontWindowCradle/2.,dzFrontWindowCradle/2.);

  TGeoTranslation *transFWCT  = new TGeoTranslation("transFWCT",0,offyFrontWindowCradleTop,offzFrontWindowCradle);
  TGeoTranslation *transFWCB  = new TGeoTranslation("transFWCB",0,offyFrontWindowCradleBottom,offzFrontWindowCradle);
  TGeoTranslation *transFWCMB = new TGeoTranslation("transFWCMB",0,offyFrontWindowCradleMB,offzFrontWindowCradle);
  TGeoTranslation *transFWCMT = new TGeoTranslation("transFWCMT",0,offyFrontWindowCradleMT,offzFrontWindowCradle);
  TGeoTranslation *transFWCML = new TGeoTranslation("transFWCML",offxFrontWindowCradleMS,offyFrontWindowCradleMS,offzFrontWindowCradle);
  TGeoTranslation *transFWCMR = new TGeoTranslation("transFWCMR",-offxFrontWindowCradleMS,offyFrontWindowCradleMS,offzFrontWindowCradle);

  transFWCT  -> RegisterYourself();
  transFWCB  -> RegisterYourself();
  transFWCMB -> RegisterYourself();
  transFWCMT -> RegisterYourself();
  transFWCML -> RegisterYourself();
  transFWCMR -> RegisterYourself();

  TGeoCompositeShape* frontWindowCradleComposite = new TGeoCompositeShape("frontWindowCradleComposite","FWCT:transFWCT+FWCB:transFWCB+FWCMT:transFWCMT+FWCMB:transFWCMB+FWCML:transFWCML+FWCMR:transFWCMR");
  TGeoVolume *frontWindowCradle = new TGeoVolume("frontWindowCradle",frontWindowCradleComposite);

  // ----------------------------------------------------
  //  Bottom Plate
  // ----------------------------------------------------

  Double_t rBottomPlateCorner = 3.5;
  Double_t dzBottomPlate = dzActive + 2*rBottomPlateCorner;
  Double_t offyBottomPlate = -(dyActive + dyBottomPlate)/2.;
  Double_t offyBottomPlateRL = (dxActive + rBottomPlateCorner)/2.;

  TGeoVolume *bottomPlateCenter  = gGeoManager -> MakeBox("bottomPlateCenter",aluminium,dxActive/2.,dyBottomPlate/2.,dzBottomPlate/2.);
  TGeoVolume *bottomPlateLeft    = gGeoManager -> MakeBox("bottomPlateLeft",  aluminium,rBottomPlateCorner/2.,dyBottomPlate/2.,dzActive/2.);
  TGeoVolume *bottomPlateRight   = gGeoManager -> MakeBox("bottomPlateRight", aluminium,rBottomPlateCorner/2.,dyBottomPlate/2.,dzActive/2.);
  TGeoVolume *bottomPlateCorner1 = gGeoManager -> MakeTubs("bottomPlateCorner1",aluminium,0,rBottomPlateCorner,dyBottomPlate/2,0,90);
  TGeoVolume *bottomPlateCorner2 = gGeoManager -> MakeTubs("bottomPlateCorner2",aluminium,0,rBottomPlateCorner,dyBottomPlate/2,90,180);
  TGeoVolume *bottomPlateCorner3 = gGeoManager -> MakeTubs("bottomPlateCorner3",aluminium,0,rBottomPlateCorner,dyBottomPlate/2,180,270);
  TGeoVolume *bottomPlateCorner4 = gGeoManager -> MakeTubs("bottomPlateCorner4",aluminium,0,rBottomPlateCorner,dyBottomPlate/2,270,360);

  TGeoTranslation *transBottomPlateCenter = new TGeoTranslation("transBottomPlateCenter",0,offyBottomPlate,0);
  TGeoTranslation *transBottomPlateLeft   = new TGeoTranslation("transBottomPlateLeft",  offyBottomPlateRL,offyBottomPlate,0);
  TGeoTranslation *transBottomPlateRight  = new TGeoTranslation("transBottomPlateRight",-offyBottomPlateRL,offyBottomPlate,0);
  TGeoCombiTrans *combiBottomPlateCorner1 = new TGeoCombiTrans("combiBottomPlateCorner1", dxActive/2,offyBottomPlate, dzActive/2,rotatCorner);
  TGeoCombiTrans *combiBottomPlateCorner2 = new TGeoCombiTrans("combiBottomPlateCorner2",-dxActive/2,offyBottomPlate, dzActive/2,rotatCorner);
  TGeoCombiTrans *combiBottomPlateCorner3 = new TGeoCombiTrans("combiBottomPlateCorner3",-dxActive/2,offyBottomPlate,-dzActive/2,rotatCorner);
  TGeoCombiTrans *combiBottomPlateCorner4 = new TGeoCombiTrans("combiBottomPlateCorner4", dxActive/2,offyBottomPlate,-dzActive/2,rotatCorner);

  transBottomPlateCenter  -> RegisterYourself();
  transBottomPlateLeft    -> RegisterYourself();
  transBottomPlateRight   -> RegisterYourself();
  combiBottomPlateCorner1 -> RegisterYourself();
  combiBottomPlateCorner2 -> RegisterYourself();
  combiBottomPlateCorner3 -> RegisterYourself();
  combiBottomPlateCorner4 -> RegisterYourself();

  TGeoCompositeShape* bottomPlateComposite = new TGeoCompositeShape("bottomPlateComposite","bottomPlateCenter:transBottomPlateCenter+bottomPlateRight:transBottomPlateRight+bottomPlateLeft:transBottomPlateLeft+bottomPlateCorner1:combiBottomPlateCorner1+bottomPlateCorner2:combiBottomPlateCorner2+bottomPlateCorner3:combiBottomPlateCorner3+bottomPlateCorner4:combiBottomPlateCorner4");
  TGeoVolume *bottomPlate = new TGeoVolume("bottomPlate",bottomPlateComposite);

  Int_t colorBottomPlate = 18;

  // ----------------------------------------------------
  //  Back Window Frame & Back Window
  // ----------------------------------------------------

  Double_t dxBackWindow = 89.76;
  Double_t dyBackWindow = dyActive; // TODO was 48.2475;
  Double_t dzBackWindow = 0.0125;

  Double_t dxBackWindowFrame = 89.6584;
  Double_t dyBackWindowFrameT = 3.15; // top frame
  Double_t dyBackWindowFrameB = 3.55; // bottom frame
  Double_t dzBackWindowFrame = 1.27;
  Double_t dxBackWindowFrameSide = 2.4492;
  Double_t dyBackWindowFrameSide = dyActive - dyBackWindowFrameT - dyBackWindowFrameB;
  cout << dyBackWindowFrameSide << endl;

  Double_t dwBackWindowFrame2 = 4.5;
  Double_t dzBackWindowFrame2 = 0.635;
  Double_t dxBackWindowFrame2TB = dxBackWindow;
  Double_t dyBackWindowFrame2B = 4.8475;
  Double_t dyBackWindowFrame2S = dyActive - dwBackWindowFrame2 - dyBackWindowFrame2B;

  Double_t offzBackWindow = -dzActiveIn/2 - rActiveCorner - dzBackWindowFrame - dzBackWindowFrame2 - dzBackWindow/2;

  Double_t offyBackWindowFrameT = dyActive/2 - dyBackWindowFrameT/2;
  Double_t offyBackWindowFrameB = -dyActive/2 + dyBackWindowFrameB/2;
  Double_t offxBackWindowFrameS = dxBackWindowFrame/2 - dxBackWindowFrameSide/2;
  Double_t offyBackWindowFrameS = (dyBackWindowFrameB - dyBackWindowFrameT)/2;
  Double_t offzBackWindowFrame = -(dzActiveIn/2 + rActiveCorner + dzBackWindowFrame/2);

  Double_t offyBackWindowFrame2T = dyBackWindowFrame2B/2 + dyBackWindowFrame2S/2;
  Double_t offyBackWindowFrame2B = -dyBackWindowFrame2S/2 - dwBackWindowFrame2/2;
  Double_t offzBackWindowFrame2TB = -dzBackWindowFrame2/2 - dzBackWindowFrame/2 + offzBackWindowFrame;
  Double_t offxBackWindowFrame2S = dxBackWindowFrame2TB/2 - dwBackWindowFrame2/2;
  Double_t offzBackWindowFrame2S = offzBackWindowFrame2TB;
  Double_t offyBackWindowFrame2S = (dyBackWindowFrame2B - dwBackWindowFrame2)/2;

  TGeoVolume *backWindow = gGeoManager -> MakeBox("backWindow",kapton,dxBackWindow/2,dyBackWindow/2,dzBackWindow/2);
  TGeoTranslation *trbackwindow = new TGeoTranslation(0,0,offzBackWindow);

  TGeoVolume *BWFT  = gGeoManager -> MakeBox("BWFT", aluminium,dxBackWindowFrame/2,dyBackWindowFrameT/2,dzBackWindowFrame/2);
  TGeoVolume *BWFB  = gGeoManager -> MakeBox("BWFB", aluminium,dxBackWindowFrame/2,dyBackWindowFrameB/2,dzBackWindowFrame/2);
  TGeoVolume *BWFL  = gGeoManager -> MakeBox("BWFL", aluminium,dxBackWindowFrameSide/2,dyBackWindowFrameSide/2,dzBackWindowFrame/2);
  TGeoVolume *BWFR  = gGeoManager -> MakeBox("BWFR", aluminium,dxBackWindowFrameSide/2,dyBackWindowFrameSide/2,dzBackWindowFrame/2);
  TGeoVolume *BWF2T = gGeoManager -> MakeBox("BWF2T",aluminium,dxBackWindowFrame2TB/2,dwBackWindowFrame2/2,dzBackWindowFrame2/2);
  TGeoVolume *BWF2B = gGeoManager -> MakeBox("BWF2B",aluminium,dxBackWindowFrame2TB/2,dyBackWindowFrame2B/2,dzBackWindowFrame2/2);
  TGeoVolume *BWF2L = gGeoManager -> MakeBox("BWF2L",aluminium,dwBackWindowFrame2/2,dyBackWindowFrame2S/2,dzBackWindowFrame2/2);
  TGeoVolume *BWF2R = gGeoManager -> MakeBox("BWF2R",aluminium,dwBackWindowFrame2/2,dyBackWindowFrame2S/2,dzBackWindowFrame2/2);

  TGeoTranslation *transBWFT  = new TGeoTranslation("transBWFT",0,offyBackWindowFrameT,offzBackWindowFrame);
  TGeoTranslation *transBWFB  = new TGeoTranslation("transBWFB",0,offyBackWindowFrameB,offzBackWindowFrame);
  TGeoTranslation *transBWFL  = new TGeoTranslation("transBWFL", offxBackWindowFrameS,offyBackWindowFrameS,offzBackWindowFrame);
  TGeoTranslation *transBWFR  = new TGeoTranslation("transBWFR",-offxBackWindowFrameS,offyBackWindowFrameS,offzBackWindowFrame);
  TGeoTranslation *transBWF2T = new TGeoTranslation("transBWF2T",0,offyBackWindowFrame2T,offzBackWindowFrame2TB);
  TGeoTranslation *transBWF2B = new TGeoTranslation("transBWF2B",0,offyBackWindowFrame2B,offzBackWindowFrame2TB);
  TGeoTranslation *transBWF2L = new TGeoTranslation("transBWF2L", offxBackWindowFrame2S,offyBackWindowFrame2S,offzBackWindowFrame2S);
  TGeoTranslation *transBWF2R = new TGeoTranslation("transBWF2R",-offxBackWindowFrame2S,offyBackWindowFrame2S,offzBackWindowFrame2S);

  transBWFT  -> RegisterYourself();
  transBWFB  -> RegisterYourself();
  transBWFL  -> RegisterYourself();
  transBWFR  -> RegisterYourself();
  transBWF2T -> RegisterYourself();
  transBWF2B -> RegisterYourself();
  transBWF2L -> RegisterYourself();
  transBWF2R -> RegisterYourself();

  TGeoCompositeShape* backWindowFrameComposite = new TGeoCompositeShape("backWindowFrameComposite","BWFT:transBWFT+BWFB:transBWFB+BWFL:transBWFL+BWFR:transBWFR+BWF2T:transBWF2T+BWF2B:transBWF2B+BWF2L:transBWF2L+BWF2R:transBWF2R");
  TGeoVolume *backWindowFrame = new TGeoVolume("backWindowFrame",backWindowFrameComposite);
  
  // ----------------------------------------------------
  //  Top Frame
  // ----------------------------------------------------

  Double_t offxTopFrameSide = dxTopFrameFloorFB/2 + dxTopFrameFloorSide/2;
  Double_t offyTopFrameSide = dyActiveMiddle/2 + dyActive/2;
  Double_t offzTopFrameFB = dzTopFrameFloorSide/2 - dzTopFrameFloorFB/2;

  TGeoVolume *topFrameF = gGeoManager -> MakeBox("topFrameF",aluminium,dxTopFrameFloorFB/2,dyActiveMiddle/2,dzTopFrameFloorFB/2);
  TGeoVolume *topFrameB = gGeoManager -> MakeBox("topFrameB",aluminium,dxTopFrameFloorFB/2,dyActiveMiddle/2,dzTopFrameFloorFB/2);
  TGeoVolume *topFrameL = gGeoManager -> MakeBox("topFrameL",aluminium,dxTopFrameFloorSide/2,dyActiveMiddle/2,dzTopFrameFloorSide/2);
  TGeoVolume *topFrameR = gGeoManager -> MakeBox("topFrameR",aluminium,dxTopFrameFloorSide/2,dyActiveMiddle/2,dzTopFrameFloorSide/2);

  TGeoTranslation *transTopFrameF = new TGeoTranslation("transTopFrameF",0,offyTopFrameSide, offzTopFrameFB);
  TGeoTranslation *transTopFrameB = new TGeoTranslation("transTopFrameB",0,offyTopFrameSide,-offzTopFrameFB);
  TGeoTranslation *transTopFrameL = new TGeoTranslation("transTopFrameL", offxTopFrameSide,offyTopFrameSide,0);
  TGeoTranslation *transTopFrameR = new TGeoTranslation("transTopFrameR",-offxTopFrameSide,offyTopFrameSide,0);

  transTopFrameF -> RegisterYourself();
  transTopFrameB -> RegisterYourself();
  transTopFrameL -> RegisterYourself();
  transTopFrameR -> RegisterYourself();

  //Making Lexan part (currently called Aluminum)
  Double_t offxTopFrameLexanSide = dxTopFrameLexanFB/2 + dxTopFrameLexanSide/2;
  Double_t offyTopFrameLexanSide = dyActiveTop/2 + dyActiveMiddle + dyActive/2;
  Double_t offzTopFrameLexanFB   = dzTopFrameLexanSide/2 - dzTopFrameLexanFB/2;

  TGeoVolume *topFrameLexanF = gGeoManager -> MakeBox("topFrameLexanF",aluminium,dxTopFrameLexanFB/2,  dyActiveTop/2,dzTopFrameLexanFB/2);
  TGeoVolume *topFrameLexanB = gGeoManager -> MakeBox("topFrameLexanB",aluminium,dxTopFrameLexanFB/2,  dyActiveTop/2,dzTopFrameLexanFB/2);
  TGeoVolume *topFrameLexanL = gGeoManager -> MakeBox("topFrameLexanL",aluminium,dxTopFrameLexanSide/2,dyActiveTop/2,dzTopFrameLexanSide/2);
  TGeoVolume *topFrameLexanR = gGeoManager -> MakeBox("topFrameLexanR",aluminium,dxTopFrameLexanSide/2,dyActiveTop/2,dzTopFrameLexanSide/2);

  TGeoTranslation *transTopFrameLexanF = new TGeoTranslation("transTopFrameLexanF",0,offyTopFrameLexanSide,offzTopFrameLexanFB);
  TGeoTranslation *transTopFrameLexanB = new TGeoTranslation("transTopFrameLexanB",0,offyTopFrameLexanSide,-offzTopFrameLexanFB);
  TGeoTranslation *transTopFrameLexanL = new TGeoTranslation("transTopFrameLexanL",offxTopFrameLexanSide,offyTopFrameLexanSide,0);
  TGeoTranslation *transTopFrameLexanR = new TGeoTranslation("transTopFrameLexanR",-offxTopFrameLexanSide,offyTopFrameLexanSide,0);

  transTopFrameLexanF -> RegisterYourself();
  transTopFrameLexanB -> RegisterYourself();
  transTopFrameLexanL -> RegisterYourself();
  transTopFrameLexanR -> RegisterYourself();

  TGeoCompositeShape* topFrameComposite = new TGeoCompositeShape("topFrameComposite","topFrameF:transTopFrameF+topFrameB:transTopFrameB+topFrameL:transTopFrameL+topFrameR:transTopFrameR+topFrameLexanF:transTopFrameLexanF+topFrameLexanB:transTopFrameLexanB+topFrameLexanL:transTopFrameLexanL+topFrameLexanR:transTopFrameLexanR");
  TGeoVolume *topFrame = new TGeoVolume("topFrame",topFrameComposite);

  // ----------------------------------------------------
  //  Top Plate
  // ----------------------------------------------------

  TGeoVolume *topPlate = gGeoManager -> MakeBox("topPlate",aluminium,dxTopPlate/2,dyTopPlate/2,dzTopPlate/2);
  TGeoTranslation *transTopPlateVolume = new TGeoTranslation(0,dyActive/2+dyTopPlate/2+dyActiveTopMiddle,0);

  // ----------------------------------------------------
  //  Pad Plane
  //  Make Pad Plane (Setting to be .062 inches (.15748 cm) thick TODO ???
  // ----------------------------------------------------

  Double_t dyPadPlanePcb = 0.254;
  Double_t dyPad = 0.01;
  Double_t offyPadPlane = offyActiveToOrigin - dyPadPlanePcb/2;
  Double_t offyPadArray = offyActiveToOrigin + dyPad/2;

  TGeoVolume *padPlane = gGeoManager -> MakeBox("padPlane",pcb,dxPadPlane/2,dyPadPlanePcb/2,dzPadPlane/2);
  TGeoVolume *padArray = gGeoManager -> MakeBox("padArray",copper,dxPadPlane/2,dyPad/2,dzPadPlane/2);

  TGeoTranslation *transPadPlane = new TGeoTranslation(0,offyPadPlane,offzPadPlane); //need to verify z shift
  TGeoTranslation *transPadArray = new TGeoTranslation(0,offyPadArray,offzPadPlane);

  // ----------------------------------------------------
  //  Set Medium
  // ----------------------------------------------------

  active            -> SetMedium(p10);
  cageSide          -> SetMedium(aluminium);
  cageFront         -> SetMedium(pcb);
  frontWindow       -> SetMedium(kapton);
  frontWindowFrame  -> SetMedium(polycarb);
  frontWindowCradle -> SetMedium(polycarb);
  backWindow        -> SetMedium(kapton);
  backWindowFrame   -> SetMedium(vacuum);
  topFrame          -> SetMedium(vacuum);
  topPlate          -> SetMedium(aluminium);
  bottomPlate       -> SetMedium(vacuum);
  padPlane          -> SetMedium(pcb);
  padArray          -> SetMedium(copper);

  // ----------------------------------------------------
  //  AddNode to TOP
  // ----------------------------------------------------

  top -> AddNode(tpc,1,combiTpc);
 
  // ----------------------------------------------------
  //  AddNode to TPC
  //  *** Note when adding node, that center of TPC is center of active volume.
  // ----------------------------------------------------

  tpc -> AddNode(active,1);
  tpc -> AddNode(cageFront,1);
  tpc -> AddNode(cageSide,1);
  tpc -> AddNode(frontWindow,1,transFrontWindow);
  tpc -> AddNode(frontWindowFrame,1);
  tpc -> AddNode(frontWindowCradle,1);
  tpc -> AddNode(bottomPlate,1);
  tpc -> AddNode(backWindowFrame,1);
  tpc -> AddNode(backWindow,1,trbackwindow);
  tpc -> AddNode(topFrame,1);
  tpc -> AddNode(topPlate,1,transTopPlateVolume);
  /*
  */
  //tpc -> AddNode(padPlane,1,transPadPlane); // TODO decide whether to add pad-plane to node or not, overlap with topPlate
  //tpc -> AddNode(padArray,1,transPadArray); // TODO decide whether to add pad-plane to node or not, overlap with topPlate

  // ----------------------------------------------------
  //  Visual Attributes 
  // ----------------------------------------------------

  Int_t transparency = 70;
  Int_t transparencyActive = 70 + transparency; if (transparencyActive > 100) transparencyActive = 100;
  Int_t transparencyWindow = 50 + transparency; if (transparencyWindow > 100) transparencyWindow = 100;
  Int_t transparencyWire   = 50 + transparency; if (transparencyWire   > 100) transparencyWire   = 100;

  //active            -> SetVisibility(false);
  active            -> SetTransparency(transparency);
  cageFront         -> SetTransparency(transparency);
  cageSide          -> SetTransparency(transparency);
  frontWindow       -> SetTransparency(transparencyWindow);
  frontWindowFrame  -> SetTransparency(transparency);
  frontWindowCradle -> SetTransparency(transparency);
  bottomPlate       -> SetTransparency(transparency);
  backWindowFrame   -> SetTransparency(transparency);
  backWindow        -> SetTransparency(transparencyWindow);
  topFrame          -> SetTransparency(transparency);
  topPlate          -> SetTransparency(transparency);
  padPlane          -> SetTransparency(transparency);
  padArray          -> SetTransparency(transparency);

  active            -> SetLineColor(kBlue-10);
  cageFront         -> SetLineColor(kBlue-4);
  cageSide          -> SetLineColor(18);
  frontWindow       -> SetLineColor(18);
  frontWindowFrame  -> SetLineColor(0);
  frontWindowCradle -> SetLineColor(15);
  bottomPlate       -> SetLineColor(18);
  backWindowFrame   -> SetLineColor(13);
  backWindow        -> SetLineColor(12);
  topFrame          -> SetLineColor(12);
  topPlate          -> SetLineColor(18);
  padPlane          -> SetLineColor(kRed+4);
  padArray          -> SetLineColor(kRed);

  // ----------------------------------------------------
  //  End of Building Geometry
  // ----------------------------------------------------

  gGeoManager -> CloseGeometry();
  gGeoManager -> CheckOverlaps();

  //To check top volume visually, uncomment two lines below
  //top -> SetLineColor(kYellow);
  //top -> SetTransparency(95);
  //gGeoManager -> SetTopVisible(true);

  top -> Draw("ogl");

  TString geoFileName    = dirGeom + "geomSpiRIT.root";
  TString geoManFileName = dirGeom + "geomSpiRIT.man.root";

  TFile *geoFile = new TFile(geoFileName,"recreate"); 
  top -> Write(); 
  geoFile -> Close(); 
  cout << geoFileName << " created." << endl;

  TFile *geoManFile = new TFile(geoManFileName,"recreate"); 
  gGeoManager -> Write();
  geoManFile -> Close(); 
  cout << geoManFileName << " created." << endl;
}
