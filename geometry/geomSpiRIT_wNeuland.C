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

void geomSpiRIT_wNeuland()
{
  TString dir = gSystem -> Getenv("VMCWORKDIR");
  TString dirGeom = dir + "/geometry/";

  // ----------------------------------------------------
  //  Mediums
  // ----------------------------------------------------

  FairGeoLoader *geoLoader = new FairGeoLoader("TGeo", "FairGeoLoader");
  FairGeoInterface *geoIF = geoLoader -> getGeoInterface();
 // geoIF -> setMediaFile(dirGeom + "media.geo");
  geoIF -> setMediaFile(dirGeom + "media.geo");
  geoIF -> readMedia();

  FairGeoMedia *media = geoIF -> getMedia();
  FairGeoBuilder *geoBuilder = geoLoader -> getGeoBuilder();
  
  FairGeoMedium *FGMvacuum = media -> getMedium("vacuum");
  FairGeoMedium *FGMp10    = media -> getMedium("p10");
  FairGeoMedium *FGMkapton = media -> getMedium("kapton");
  FairGeoMedium *FGMpcb    = media -> getMedium("pcb");
  FairGeoMedium *FGMaluminium = media -> getMedium("aluminium");
  FairGeoMedium *FGMcopper = media -> getMedium("copper");
  FairGeoMedium *FGMiron   = media -> getMedium("iron");
  FairGeoMedium *FGMair    = media -> getMedium("air");

  FairGeoMedium *FGMpolystyrene = media -> getMedium("polystyrene");
  FairGeoMedium *FGMppo         = media -> getMedium("PPO");
  FairGeoMedium *FGMpopop       = media -> getMedium("POPOP");
  FairGeoMedium *FGMbc404       = media -> getMedium("bc404");
  FairGeoMedium *FGMpolyvinyltoluene = media -> getMedium("polyvinyltoluene");

  geoBuilder -> createMedium(FGMvacuum);
  geoBuilder -> createMedium(FGMp10);
  geoBuilder -> createMedium(FGMkapton);
  geoBuilder -> createMedium(FGMpcb);
  geoBuilder -> createMedium(FGMaluminium);
  geoBuilder -> createMedium(FGMcopper);
  geoBuilder -> createMedium(FGMiron);
  geoBuilder -> createMedium(FGMair);
  geoBuilder -> createMedium(FGMpolystyrene);
  geoBuilder -> createMedium(FGMppo);
  geoBuilder -> createMedium(FGMpopop);
  geoBuilder -> createMedium(FGMbc404);
  geoBuilder -> createMedium(FGMpolyvinyltoluene);
  
  gGeoManager = (TGeoManager *) gROOT -> FindObject("FAIRGeom");
  gGeoManager -> SetName("SpiRIT");

  TGeoMedium *vacuum = gGeoManager -> GetMedium("vacuum");
  TGeoMedium *p10    = gGeoManager -> GetMedium("p10");
  TGeoMedium *kapton = gGeoManager -> GetMedium("kapton");
  TGeoMedium *pcb    = gGeoManager -> GetMedium("pcb");
  TGeoMedium *aluminium = gGeoManager -> GetMedium("aluminium");
  TGeoMedium *copper = gGeoManager -> GetMedium("copper");
  TGeoMedium *iron   = gGeoManager -> GetMedium("iron");
  TGeoMedium *air    = gGeoManager -> GetMedium("air");
  TGeoMedium *polystyrene = gGeoManager -> GetMedium("polystyrene");
  TGeoMedium *ppo         = gGeoManager -> GetMedium("ppo");
  TGeoMedium *popop       = gGeoManager -> GetMedium("popop");
  TGeoMedium *bc404       = gGeoManager -> GetMedium("bc404");
  TGeoMedium *polyvinyltoluene = gGeoManager -> GetMedium("polyvinyltoluene");

  

  // ----------------------------------------------------
  //  Dimensions (cm)
  // ----------------------------------------------------

  // Active : Gas volume inside the field cage
  Double_t dxActive = 96.61;  // x-length of active volume
  Double_t dyActive = 49.6;   // y-length of active volume, from cathode top to pad-plane bottom 
  Double_t dzActive = 144.64; // z-length of active volume

  // Pad Plane dimension
  Double_t dxPadPlane = 86.4; // 8 mm * 108
  Double_t dzPadPlane = 134.4; // 12 mm * 112

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
  //Double_t dyTpc = dyActive + dyTopPlate + dyBottomPlate + dyActiveTopMiddle; // = 64.2346
  Double_t dyTpc = dyActive + dyTopPlate + dyBottomPlate + dyActiveTopMiddle + 5.; // = 64.2346
  //Double_t dzTpc = dzTopPlate; // = 206.06
  Double_t dzTpc = 240.;

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

  //auto boxTop = new TGeoBBox(dxTop/2,dyTop/2,dzTop/2,offsetTop);
  auto boxTop = new TGeoBBox(1000/2,300/2,2000/2,offsetTop);
  TGeoVolume *top;
  if (boxTop -> IsRunTimeShape()) {
    top = gGeoManager -> MakeVolumeMulti("top", vacuum);
    top -> SetShape(boxTop);
  } else {
    top = new TGeoVolume("top", boxTop, vacuum);
  }
  gGeoManager -> SetTopVolume(top);
  //gGeoManager -> SetTopVolume(top);

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

  TGeoRotation*   rotatTpc = new TGeoRotation("rotatTpc",180,180,0); // set Z axis as a beam axis, X axis as a laboratory horizon
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

  TGeoVolume *cageCorner1 = gGeoManager -> MakeTubs("cageCorner1",aluminium,rActiveCorner,rCageCorner,dyActive/2,0  ,90 );
  TGeoVolume *cageCorner2 = gGeoManager -> MakeTubs("cageCorner2",aluminium,rActiveCorner,rCageCorner,dyActive/2,90 ,180);
  TGeoVolume *cageCorner3 = gGeoManager -> MakeTubs("cageCorner3",aluminium,rActiveCorner,rCageCorner,dyActive/2,180,270);
  TGeoVolume *cageCorner4 = gGeoManager -> MakeTubs("cageCorner4",aluminium,rActiveCorner,rCageCorner,dyActive/2,270,360);

  // ----------------------------------------------------
  //  Field Cage Frame
  // ----------------------------------------------------

  Double_t dxCageFrontRL = 40.48;
  Double_t dxCageFrontTB = dxActiveIn - 2*dxCageFrontRL;
  Double_t dyCageFrontT  = 13.0;
  Double_t dyCageFrontHole = 17;
  Double_t dyCageFrontB  = dyActive - dyCageFrontT - dyCageFrontHole;

  TGeoVolume* cageRight  = gGeoManager -> MakeBox("cageRight" ,aluminium,dwCage/2,dyActive/2,dzActiveIn/2);
  TGeoVolume* cageLeft   = gGeoManager -> MakeBox("cageLeft"  ,aluminium,dwCage/2,dyActive/2,dzActiveIn/2);
  TGeoVolume* cageFrontR = gGeoManager -> MakeBox("cageFrontR",aluminium,dxCageFrontRL/2,dyActive/2,dwCage/2);
  TGeoVolume* cageFrontL = gGeoManager -> MakeBox("cageFrontL",aluminium,dxCageFrontRL/2,dyActive/2,dwCage/2);
  TGeoVolume* cageFrontT = gGeoManager -> MakeBox("cageFrontT",aluminium,dxCageFrontTB/2,dyCageFrontT/2,dwCage/2);
  TGeoVolume* cageFrontB = gGeoManager -> MakeBox("cageFrontB",aluminium,dxCageFrontTB/2,dyCageFrontB/2,dwCage/2);

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

  TGeoVolume *FWFR = gGeoManager -> MakeBox("FWFR",aluminium,dwFrontWindowFrame/2,dyFrontWindowFrame/2,dzFrontWindow/2);
  TGeoVolume *FWFL = gGeoManager -> MakeBox("FWFL",aluminium,dwFrontWindowFrame/2,dyFrontWindowFrame/2,dzFrontWindow/2);
  TGeoVolume *FWFT = gGeoManager -> MakeBox("FWFT",aluminium,dxyFrontWindow/2,dwFrontWindowFrame/2,dzFrontWindow/2);
  TGeoVolume *FWFB = gGeoManager -> MakeBox("FWFB",aluminium,dxyFrontWindow/2,dxFrontWindowFrame/2,dzFrontWindow/2);
  TGeoVolume *FWF2R = gGeoManager -> MakeTrap("FWF2R",aluminium,dzFrontWindow/2,thetaFrontWindowFrame2,0,dxyFrontWindow/2,
                                                                                          dxFrontWindowInnerFrame/2,dxFrontWindowInnerFrame/2,0,
                                                                                          dxyFrontWindow/2,0,0,0);
  TGeoVolume *FWF2L = gGeoManager -> MakeTrap("FWF2L",aluminium,dzFrontWindow/2,-thetaFrontWindowFrame2,0,dxyFrontWindow/2,
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
  transFrontWindow -> RegisterYourself();

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

  TGeoVolume *FWCT  = gGeoManager -> MakeBox("FWCT",aluminium,dxFrontWindowCradleTB/2.,dyFrontWindowCradleTop/2.,dzFrontWindowCradle/2.);
  TGeoVolume *FWCB  = gGeoManager -> MakeBox("FWCB",aluminium,dxFrontWindowCradleTB/2.,dyFrontWindowCradleBottom/2.,dzFrontWindowCradle/2.);
  TGeoVolume *FWCMB = gGeoManager -> MakeBox("FWCMB",aluminium,dxFrontWindowCradle/2.,dwFrontWindowCradle/2.,dzFrontWindowCradle/2.);
  TGeoVolume *FWCMT = gGeoManager -> MakeBox("FWCMT",aluminium,dxFrontWindowCradle/2.,dwFrontWindowCradle/2.,dzFrontWindowCradle/2.);
  TGeoVolume *FWCML = gGeoManager -> MakeBox("FWCML",aluminium,dwFrontWindowCradle/2.,dyFrontWindowCradle/2.,dzFrontWindowCradle/2.);
  TGeoVolume *FWCMR = gGeoManager -> MakeBox("FWCMR",aluminium,dwFrontWindowCradle/2.,dyFrontWindowCradle/2.,dzFrontWindowCradle/2.);

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
  TGeoTranslation *transBackWindow = new TGeoTranslation(0,0,offzBackWindow);
  transBackWindow -> RegisterYourself();

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
  
 
  
  // ##############################################################
  // M. Kaneko added below geometry part with inventer information.
  // ##############################################################
  // VOLUMENAME_in means the volume that will be subtracted from "VOLUMENAME".
  //
  //
  // ----------------------------------------------------
  //  Enclosure iron frame
  // ----------------------------------------------------

  // total size
  Double_t dxEnclosureFrame = 147.32;
  Double_t dyEnclosureFrame = 53.962;
  Double_t dzEnclosureFrame = 200.98;
  Double_t dwEnclosureFrame = 1.27; // frame thickness half inch
  
  // frame width
  Double_t dwzEncTopFrame = 7.62; // 3 inches
  Double_t dwxEncTopFrame = 7.62; // 3 inches
  Double_t dwyEncSideFrame = 7.62; // 3 inches
  Double_t dwzEncSideFrame = 10.16; // 4 inches
  Double_t dwxEncFrontFrame = 10.16;
  Double_t dwyEncFrontFrame = 7.62;
  
  // top and bottom part
  TGeoVolume *encFrameTB = gGeoManager -> MakeBox("encFrameTB",iron,dxEnclosureFrame/2,dwEnclosureFrame/2,dzEnclosureFrame/2);
  Double_t dxEncTopFrame_in = dxEnclosureFrame - 2.*dwxEncTopFrame;
  Double_t dyEncTopFrame_in = dwEnclosureFrame + 0.1; // not to overlap the plane
  Double_t dzEncTopFrame_in = dzEnclosureFrame - 2.*dwzEncTopFrame;
  TGeoVolume *encFrameTB_in = gGeoManager -> MakeBox("encFrameTB_in",iron,dxEncTopFrame_in/2,dyEncTopFrame_in,dzEncTopFrame_in/2);
  TGeoCompositeShape* encFrameTBComposite = new TGeoCompositeShape("encFrameTBComposite","encFrameTB-encFrameTB_in"); // subtract box.
  
  Double_t offyEncFrameTB = dyEnclosureFrame/2. - dwEnclosureFrame/2.;
  TGeoTranslation* transEncFrameT = new TGeoTranslation("transEncFrameT",0,offyEncFrameTB,0);
  TGeoTranslation* transEncFrameB = new TGeoTranslation("transEncFrameB",0,-offyEncFrameTB,0);
  
  // side part (has a column in-between)
  Double_t dyEncSideFrame = dyEnclosureFrame - 2.*dwEnclosureFrame;
  TGeoVolume * encFrameSide = gGeoManager -> MakeBox("encFrameSide",iron,dwEnclosureFrame/2,dyEncSideFrame/2,dzEnclosureFrame/2);
  Double_t dxEncSideFrame_in = dwEnclosureFrame + 0.1;
  Double_t dyEncSideFrame_in = dyEnclosureFrame - 2.*dwyEncSideFrame;
  Double_t dzEncSideFrame_inF = 24.661;
  Double_t dzEncSideFrame_inB = dzEnclosureFrame - 3.*dwzEncSideFrame - dzEncSideFrame_inF;
  TGeoVolume * encFrameSide_inF = gGeoManager -> MakeBox("encFrameSide_inF",iron,dxEncSideFrame_in/2,dyEncSideFrame_in/2,dzEncSideFrame_inF/2);
  TGeoVolume * encFrameSide_inB = gGeoManager -> MakeBox("encFrameSide_inB",iron,dxEncSideFrame_in/2,dyEncSideFrame_in/2,dzEncSideFrame_inB/2);
  Double_t offzEncFrameSide_inF = dzEnclosureFrame/2-dwzEncSideFrame-dzEncSideFrame_inF/2;
  Double_t offzEncFrameSide_inB = dzEnclosureFrame/2-dwzEncSideFrame-dzEncSideFrame_inF-dwzEncSideFrame-dzEncSideFrame_inB/2;
  TGeoTranslation * transEncFrameSide_inF = new TGeoTranslation("transEncFrameSide_inF",0,0,offzEncFrameSide_inF);
  TGeoTranslation * transEncFrameSide_inB = new TGeoTranslation("transEncFrameSide_inB",0,0,offzEncFrameSide_inB);
  transEncFrameSide_inF -> RegisterYourself();
  transEncFrameSide_inB -> RegisterYourself();
  TGeoCompositeShape* encFrameSideComposite = new TGeoCompositeShape("encFrameSideComposite","encFrameSide-encFrameSide_inF:transEncFrameSide_inF-encFrameSide_inB:transEncFrameSide_inB");
  Double_t offxEncFrameSide = dxEnclosureFrame/2. - dwEnclosureFrame/2.;
  TGeoTranslation* transEncFrameL = new TGeoTranslation("transEncFrameL",-offxEncFrameSide,0,0);
  TGeoTranslation* transEncFrameR = new TGeoTranslation("transEncFrameR",offxEncFrameSide,0,0);
  
  // front and back part
  Double_t dyEncFBFrame = dyEnclosureFrame - 2.*dwEnclosureFrame; // = dyEncSideFrame
  Double_t dxEncFBFrame = dxEnclosureFrame - 2.*dwEnclosureFrame;
  TGeoVolume* encFrameFB  = gGeoManager -> MakeBox("encFrameFB",iron,dxEncFBFrame/2,dyEncFBFrame/2,dwEnclosureFrame/2);
  Double_t dxEncFBFrame_in = dxEnclosureFrame - 2.*dwxEncFrontFrame;
  Double_t dyEncFBFrame_in = dyEnclosureFrame - 2.*dwyEncFrontFrame;
  Double_t dzEncFBFrame_in = dwEnclosureFrame + 0.1;
  TGeoVolume* encFrameFB_in = gGeoManager -> MakeBox("encFrameFB_in",iron,dxEncFBFrame_in/2,dyEncFBFrame_in/2,dzEncFBFrame_in/2);
  TGeoCompositeShape* encFrameFBComposite = new TGeoCompositeShape("encFrameFBComposite","encFrameFB-encFrameFB_in");
  Double_t offzEncFrameFB = dzEnclosureFrame/2. - dwEnclosureFrame/2.;
  TGeoTranslation* transEncFrameFront = new TGeoTranslation("transEncFrameFront",0,0,offzEncFrameFB);
  TGeoTranslation* transEncFrameBack = new TGeoTranslation("transEncFrameBack",0,0,-offzEncFrameFB);
  
  // make composite for the enclosure frame. 
  transEncFrameT -> RegisterYourself();
  transEncFrameB -> RegisterYourself();
  transEncFrameL -> RegisterYourself();
  transEncFrameR -> RegisterYourself();
  transEncFrameFront -> RegisterYourself();
  transEncFrameBack  -> RegisterYourself();

  TGeoCompositeShape* encFrameComposite = new TGeoCompositeShape("encFrameComposite","encFrameTBComposite:transEncFrameT+encFrameTBComposite:transEncFrameB+encFrameSideComposite:transEncFrameL+encFrameSideComposite:transEncFrameR+encFrameFBComposite:transEncFrameFront+encFrameFBComposite:transEncFrameBack");
 
  TGeoVolume* enclosureFrame = new TGeoVolume("enclosureFrame",encFrameComposite);
  
  // ----------------------------------------------------
  // thin windows for external trigger scitillator (both-side and back.)
  // ----------------------------------------------------
  
  Double_t dwThinWindow = 0.051; // 0.02 inch
  Double_t dyThinWindow = 43.421;
  Double_t dzThinWindowSide = 148.458;
  TGeoVolume * thinWindowLR = gGeoManager -> MakeBox("thinWindowLR",aluminium,dwThinWindow/2,dyThinWindow/2,dzThinWindowSide/2);
  Double_t dxThinWindowD = 129.629;
  TGeoVolume * thinWindowD  = gGeoManager -> MakeBox("thinWindowD", aluminium,dxThinWindowD/2,dyThinWindow/2,dwThinWindow/2);
   
  // ----------------------------------------------------
  // Side plates with space for thin window
  // ----------------------------------------------------

  Double_t dyPlateSide = 53.797;
  Double_t dzPlateSide = 154.17;
  Double_t dwPlate = 1.27;
  Double_t offwPlateOutToThinWindow = 1.016; // from outer plate surface to thin window surface.
  Double_t dwPlateSideOut = offwPlateOutToThinWindow;
  Double_t dwPlateSideIn  = dwPlate - dwPlateSideOut;
  TGeoVolume * plateSideOut = gGeoManager -> MakeBox("plateSideOut",aluminium,dwPlateSideOut/2,dyPlateSide/2,dzPlateSide/2);
  Double_t dyPlateSideOut_in = dyPlateSide - 2.*6.35;
  Double_t dzPlateSideOut_in = dzPlateSide - 2.*4.018;
  TGeoVolume * plateSideOut_in = gGeoManager -> MakeBox("plateSideOut_in",aluminium,dwPlateSideOut/2+0.1,dyPlateSideOut_in/2,dzPlateSideOut_in/2);
  TGeoCompositeShape * plateSideOutComposite = new TGeoCompositeShape("plateSideOutComposite","plateSideOut-plateSideOut_in");
  
  TGeoVolume * plateSideIn = gGeoManager -> MakeBox("plateSideIn",aluminium,dwPlateSideIn/2,dyPlateSide/2,dzPlateSide/2);
  TGeoVolume * plateSideIn_in = gGeoManager -> MakeBox("plateSideIn_in",aluminium,dwPlateSideIn/2+0.1,dyThinWindow/2,dzThinWindowSide/2);
  TGeoCompositeShape * plateSideInComposite = new TGeoCompositeShape("plateSideInComposite","plateSideIn-plateSideIn_in");
  
  Double_t offxPlateOutToPlateIn = dwPlate/2.; //( = dwPlateSideIn/2 + dwPlateSideOut/2)
  TGeoTranslation * transPlateSideOutL = new TGeoTranslation("transPlateSideOutL",-offxPlateOutToPlateIn,0,0);
  TGeoTranslation * transPlateSideOutR = new TGeoTranslation("transPlateSideOutR",offxPlateOutToPlateIn,0,0);
  transPlateSideOutL -> RegisterYourself();
  transPlateSideOutR -> RegisterYourself();
  // attach "Outer plate" to "Inner plate" -> coordinate origin of this geometry is Inner plate
  TGeoCompositeShape * plateSideCompositeL = new TGeoCompositeShape("plateSideCompositeL","plateSideInComposite+plateSideOutComposite:transPlateSideOutL");
  TGeoCompositeShape * plateSideCompositeR = new TGeoCompositeShape("plateSideCompositeR","plateSideInComposite+plateSideOutComposite:transPlateSideOutR");
  
  TGeoVolume * plateL = new TGeoVolume("plateL",plateSideCompositeL); 
  TGeoVolume * plateR = new TGeoVolume("plateR",plateSideCompositeR); 
  
  Double_t offxPlateSide = dxEnclosureFrame/2 + dwPlateSideIn/2;
  Double_t offzPlateSide = offzEncFrameSide_inB;
  TGeoTranslation * transPlateL = new TGeoTranslation("transPlateL",-offxPlateSide,0,offzPlateSide);
  TGeoTranslation * transPlateR = new TGeoTranslation("transPlateR",offxPlateSide,0,offzPlateSide);
  transPlateL -> RegisterYourself();
  transPlateR -> RegisterYourself();
  
  Double_t offxThinWindowSide = dxEnclosureFrame/2 + dwPlateSideIn - dwThinWindow/2;
  Double_t offzThinWindowSide = offzPlateSide;
  TGeoTranslation * transThinWindowL = new TGeoTranslation("transThinWindowL",-offxThinWindowSide,0,offzThinWindowSide);
  TGeoTranslation * transThinWindowR = new TGeoTranslation("transThinWindowR",offxThinWindowSide,0,offzThinWindowSide);
  transThinWindowL -> RegisterYourself();
  transThinWindowR -> RegisterYourself();
  
  // front side plate with window to see the target motion from outside
  Double_t dzPlateSideF = 32.24;
  Double_t dwPlateSideF = 1.588;
  Double_t dyGrassWindow = 38.456;
  Double_t dzGrassWindow = 24.407;
  Double_t dwGrassWindow = 0.318;
  Double_t offwPlateOutToGrassWindow = 1.08;
  Double_t dwPlateSideFOut = offwPlateOutToGrassWindow;
  Double_t dwPlateSideFIn  = dwPlateSideF - dwPlateSideFOut;
  TGeoVolume * plateSideFOut = gGeoManager -> MakeBox("plateSideFOut",aluminium,dwPlateSideFOut/2,dyPlateSide/2,dzPlateSideF/2);
  Double_t dyPlateSideFOut_in = dyPlateSide - 2*8.814;
  Double_t dzPlateSideFOut_in = dzPlateSideF - 2*5.06;
  TGeoVolume * plateSideFOut_in = gGeoManager -> MakeBox("plateSideFOut_in",aluminium,dwPlateSideFOut/2+0.1,dyPlateSideFOut_in/2,dzPlateSideFOut_in/2);
  TGeoCompositeShape * plateSideFOutComposite = new TGeoCompositeShape("plateSideFOutComposite","plateSideFOut-plateSideFOut_in");
  
  TGeoVolume * plateSideFIn = gGeoManager -> MakeBox("plateSideFIn",aluminium,dwPlateSideFIn/2,dyPlateSide/2,dzPlateSideF/2);
  TGeoVolume * plateSideFIn_in = gGeoManager -> MakeBox("plateSideFIn_in",aluminium,dwPlateSideFIn/2+0.1,dyGrassWindow/2,dzGrassWindow/2);
  TGeoCompositeShape * plateSideFInComposite = new TGeoCompositeShape("plateSideFInComposite","plateSideFIn-plateSideFIn_in");

  Double_t offxPlateSideFOutToIn = dwPlateSideF/2;
  TGeoTranslation * transPlateSideFOutL = new TGeoTranslation("transPlateSideFOutL",-offxPlateSideFOutToIn,0,0);
  TGeoTranslation * transPlateSideFOutR = new TGeoTranslation("transPlateSideFOutR",offxPlateSideFOutToIn,0,0);
  transPlateSideFOutL -> RegisterYourself();
  transPlateSideFOutR -> RegisterYourself();
  TGeoCompositeShape * plateSideFCompositeL = new TGeoCompositeShape("plateSideFCompositeL","plateSideFInComposite+plateSideFOutComposite:transPlateSideFOutL");
  TGeoCompositeShape * plateSideFCompositeR = new TGeoCompositeShape("plateSideFCompositeR","plateSideFInComposite+plateSideFOutComposite:transPlateSideFOutR");
  
  TGeoVolume * plateFL = new TGeoVolume("plateFL",plateSideFCompositeL);
  TGeoVolume * plateFR = new TGeoVolume("plateFR",plateSideFCompositeR);

  Double_t offxPlateSideF = dxEnclosureFrame/2 + dwPlateSideFIn/2;
  Double_t offzPlateSideF = offzEncFrameSide_inF;
  TGeoTranslation * transPlateFL = new TGeoTranslation("transPlateFL",-offxPlateSideF,0,offzPlateSideF);
  TGeoTranslation * transPlateFR = new TGeoTranslation("transPlateFR",offxPlateSideF,0,offzPlateSideF);
  transPlateFL -> RegisterYourself();
  transPlateFR -> RegisterYourself();

  TGeoVolume * grassWindow = gGeoManager->MakeBox("grassWindow",vacuum,dwGrassWindow/2,dyGrassWindow/2,dzGrassWindow/2);
  Double_t offxGrassWindow = dxEnclosureFrame/2 + dwPlateSideFIn - dwGrassWindow/2;
  Double_t offzGrassWindow = offzPlateSideF;
  TGeoTranslation * transGrassWindowL = new TGeoTranslation("transGrassWindowL",-offxGrassWindow,0,offzGrassWindow);
  TGeoTranslation * transGrassWindowR = new TGeoTranslation("transGrassWindowR",offxGrassWindow,0,offzGrassWindow);
  transGrassWindowL -> RegisterYourself();
  transGrassWindowR -> RegisterYourself();

  // ----------------------------------------------------
  // DownStream plate
  // ----------------------------------------------------
  
  Double_t dxPlateD = 135.341; 
  Double_t dyPlateD = dyPlateSide;
  Double_t dwPlateDOut = dwPlateSideOut;
  Double_t dwPlateDIn = dwPlateSideIn;
  TGeoVolume * plateDOut = gGeoManager -> MakeBox("plateDOut",aluminium,dxPlateD/2,dyPlateD/2,dwPlateDOut/2);
  Double_t dxPlateDOut_in = dxPlateD - 2.*4.018;
  Double_t dyPlateDOut_in = dyPlateD - 2.*6.35;
  TGeoVolume * plateDOut_in = gGeoManager -> MakeBox("plateDOut_in",aluminium,dxPlateDOut_in/2,dyPlateDOut_in/2,dwPlateDOut/2+0.1);
  TGeoCompositeShape * plateDOutComposite = new TGeoCompositeShape("plateDOutComposite","plateDOut-plateDOut_in");

  TGeoVolume * plateDIn = gGeoManager -> MakeBox("plateDIn",aluminium,dxPlateD/2,dyPlateD/2,dwPlateDIn/2);
  TGeoVolume * plateDIn_in = gGeoManager -> MakeBox("plateDIn_in",aluminium,dxThinWindowD/2,dyThinWindow/2,dwPlateDIn/2+0.1);
  TGeoCompositeShape * plateDInComposite = new TGeoCompositeShape("plateDInComposite","plateDIn-plateDIn_in");

  Double_t offzPlateDOutToPlateDIn = dwPlate/2.;
  TGeoTranslation * transPlateDOut = new TGeoTranslation("transPlateDOut",0,0,offzPlateDOutToPlateDIn);
  transPlateDOut -> RegisterYourself();
  TGeoCompositeShape * plateDComposite = new TGeoCompositeShape("plateDComposite","plateDInComposite+plateDOutComposite:transPlateDOut");
  
  TGeoVolume * plateD = new TGeoVolume("plateD",plateDComposite);
  
  Double_t offzPlateD = dzEnclosureFrame/2 + dwPlateDIn/2;
  TGeoTranslation * transPlateD = new TGeoTranslation("transPlateD",0,0,-offzPlateD);
  transPlateD -> RegisterYourself();

  Double_t offzThinWindowD = dzEnclosureFrame/2 + dwPlateDIn - dwThinWindow/2;
  TGeoTranslation * transThinWindowD = new TGeoTranslation("transThinWindowD",0,0,-offzThinWindowD);
  transThinWindowD -> RegisterYourself();
  
  
  // ----------------------------------------------------
  // Upstream plate
  // ----------------------------------------------------
 
  
  // ----------------------------------------------------
  // p10 gas between the fieldcage and TPC enclosure
  //
  // made by subtracting all volumes within the iron frame.
  // subtracted volumes are ... 
  // active, cageFront, cageSide, frontWindow:transFrontWindow, frontWindowFrame, frontWindowCradle,
  // bottomPlate, backWindowFrame, backWindow:transbackwindow, enclosureFrame
  // topFrame, 
  // ----------------------------------------------------
  
  TGeoVolume * p10BoxVol = gGeoManager -> MakeBox("p10BoxVol",p10,dxEnclosureFrame/2.,dyEnclosureFrame/2.,dzEnclosureFrame/2.);

  //TGeoCompositeShape * p10gasComposite = new TGeoCompositeShape("p10gasComposite","p10BoxVol-activeComposite-encFrameComposite-cageFrontComposite-cageSideComposite-frontWindow:transFrontWindow-frontWindowCradleComposite-backWindowFrameComposite-topFrameComposite-bottomPlateComposite-frontWindowFrameComposite");
 TGeoCompositeShape * p10gasComposite = new TGeoCompositeShape("p10gasComposite","p10BoxVol-activeComposite-encFrameComposite-cageFrontComposite-cageSideComposite-frontWindow:transFrontWindow-frontWindowCradleComposite-backWindowFrameComposite-topFrameComposite-bottomPlateComposite-FWFR:transFWFR-FWFL:transFWFL-FWFT:transFWFT-FWFB:transFWFB-FWF2R:transFWF2R-FWF2L:transFWF2L");
  TGeoVolume * p10gas = new TGeoVolume("p10gas",p10gasComposite);

  // ----------------------------------------------------
  // Kyoto array plastics
  // dl: length, dw: width, dt: thickness
  // ----------------------------------------------------
  
  Double_t dlKyotoPla = 45.;
  Double_t dwKyotoPla = 5.;
  Double_t dtKyotoPla = 1.;
  Double_t drKyotoPlaHole = .1;
  TGeoVolume * kyotoPla = gGeoManager -> MakeBox("kyotoPla",polystyrene,dtKyotoPla/2,dlKyotoPla/2,dwKyotoPla/2);
  
  TGeoVolume * kyotoPla_in = gGeoManager -> MakeTube("kyotoPla_in",polystyrene,0,drKyotoPlaHole,dlKyotoPla/2+0.1);
  TGeoRotation * rotatKyotoPla_in = new TGeoRotation("rotatKyotoPla_in",0,90,0);
  TGeoCombiTrans * combiKyotoPla_in = new TGeoCombiTrans("combiKyotoPla_in",0,0,0,rotatKyotoPla_in);
  combiKyotoPla_in -> RegisterYourself();
  TGeoCompositeShape * kyotoPlaComposite = new TGeoCompositeShape("kyotoPlaComposite","kyotoPla-kyotoPla_in:combiKyotoPla_in");
  
  //TGeoVolume * kyoto = new TGeoVolume("kyoto",kyotoPlaComposite);
  TGeoVolume * kyoto[60];
  for(Int_t ikyoto=0; ikyoto<60; ikyoto++)
    kyoto[ikyoto] = new TGeoVolume(Form("kyoto%d",ikyoto),kyotoPlaComposite);


  Double_t offxKyotoPla = 75.8+dtKyotoPla/2.;
  Double_t offyKyotoPla = offyPadPlane-5.1-dlKyotoPla/2.;
  Double_t offzKyotoPla = offzPadPlane+dzPadPlane/2.-16.8-dwKyotoPla/2.; // the most upstream bar is at 16.8 cm downstream from pad plane coordinate.
  Double_t offzKyotoPlaToPla = 5.05;
  TGeoTranslation * transKyotoPlaL[30];
  TGeoTranslation * transKyotoPlaR[30];
  for(Int_t i=0; i<30; i++){
   transKyotoPlaL[i] = new TGeoTranslation(Form("transKyotoPlaL_%d",i),-offxKyotoPla,offyKyotoPla,offzKyotoPla-offzKyotoPlaToPla*i);
   transKyotoPlaR[i] = new TGeoTranslation(Form("transKyotoPlaR_%d",i),offxKyotoPla,offyKyotoPla,offzKyotoPla-offzKyotoPlaToPla*i);
   transKyotoPlaL[i] -> RegisterYourself(); 
   transKyotoPlaR[i] -> RegisterYourself(); 
  }


  // ----------------------------------------------------
  // KATANA veto plastic
  // ----------------------------------------------------
  
  Double_t dlKatanaV = 40.;
  Double_t dwKatanaV = 10.;
  Double_t dtKatanaV = .1;
  TGeoVolume * katanaVPlaL = gGeoManager -> MakeBox("katanaVPlaL",polyvinyltoluene,dwKatanaV/2,dlKatanaV/2,dtKatanaV/2);
  TGeoVolume * katanaVPlaM = gGeoManager -> MakeBox("katanaVPlaM",polyvinyltoluene,dwKatanaV/2,dlKatanaV/2,dtKatanaV/2);
  TGeoVolume * katanaVPlaR = gGeoManager -> MakeBox("katanaVPlaR",polyvinyltoluene,dwKatanaV/2,dlKatanaV/2,dtKatanaV/2);
  
  Double_t offxKatanaVPla = 21.6; // for 132Sn
  //Double_t offxKatanaVPla = 24.6; // for 108Sn
  Double_t offyKatanaVPla = offyPadPlane - 4. - dlKatanaV/2.;
  Double_t offzKatanaVPla = offzPadPlane+dzPadPlane/2.-186.7-dtKatanaV/2.;
  TGeoTranslation * transKatanaVPlaL = new TGeoTranslation("transKatanaVPlaL",offxKatanaVPla-dwKatanaV+1.2,offyKatanaVPla,offzKatanaVPla+1.);
  TGeoTranslation * transKatanaVPlaM = new TGeoTranslation("transKatanaVPlaM",offxKatanaVPla,offyKatanaVPla,offzKatanaVPla);
  TGeoTranslation * transKatanaVPlaR = new TGeoTranslation("transKatanaVPlaR",offxKatanaVPla+dwKatanaV-1.2,offyKatanaVPla,offzKatanaVPla+1.);
  transKatanaVPlaL -> RegisterYourself();
  transKatanaVPlaM -> RegisterYourself();
  transKatanaVPlaR -> RegisterYourself();
  


  // ----------------------------------------------------
  // air outside of TPC
  //
  // large box covers TPC, Kyoto, Katana
  // subtract enclosureFrame cuboid, plates, thin window, kyoto, katana -> outer air
  // ----------------------------------------------------
  
  Double_t dxOuterAir = dxTpc;
  Double_t dyOuterAir = dyEnclosureFrame; // make air just for plastics -> y of Air = TPC side plate
  Double_t dzOuterAir = dzTpc;
  TGeoVolume * outerAirBox = gGeoManager -> MakeBox("outerAirBox",air,dxOuterAir/2.,dyOuterAir/2.,dzOuterAir/2.);
  
  // subtract volume for TPC enclosure + plates
  TGeoVolume * tpcEncBox = gGeoManager -> MakeBox("tpcEncBox",air,(dxEnclosureFrame+dwPlate*2.)/2.,dyOuterAir/2.+0.5,(dzEnclosureFrame+dwPlate*2.)/2.);

  // subtract volume for kyoto array
  TGeoVolume * kyotoBox = gGeoManager -> MakeBox("kyotoBox",air,dtKyotoPla/2.,dlKyotoPla/2.,(dwKyotoPla+0.05)*30./2.);
  TGeoTranslation * transKyotoBoxL = new TGeoTranslation("transKyotoBoxL",-offxKyotoPla,offyKyotoPla,offzKyotoPla-offzKyotoPlaToPla*14.5);
  TGeoTranslation * transKyotoBoxR = new TGeoTranslation("transKyotoBoxR",offxKyotoPla,offyKyotoPla,offzKyotoPla-offzKyotoPlaToPla*14.5);
  transKyotoBoxL->RegisterYourself();
  transKyotoBoxR->RegisterYourself();

  TGeoCompositeShape * outerAirComposite = new TGeoCompositeShape("outerAirComposite","outerAirBox-tpcEncBox-kyotoBox:transKyotoBoxL-kyotoBox:transKyotoBoxR-katanaVPlaL:transKatanaVPlaL-katanaVPlaM:transKatanaVPlaM-katanaVPlaR:transKatanaVPlaR");
  TGeoVolume * outerAir = new TGeoVolume("outerAir",outerAirComposite);
  
  
  // ----------------------------------------------------
  // NeuLAND demonstrator
  //
  // ----------------------------------------------------
  
  auto boxNeuland = new TGeoBBox(252./2,252./2,100./2);
  TGeoVolume *neuland;
  if (boxNeuland -> IsRunTimeShape()) {
    neuland = gGeoManager -> MakeVolumeMulti("neuland", vacuum);
    neuland -> SetShape(boxNeuland);
  } else {
    neuland = new TGeoVolume("neuland", boxNeuland, vacuum);
  }
  
  Double_t distTargetZ = 8.9;
  Double_t distNeuland = 856.105 + 28.0 + 50.56/2.; //[cm]
  Double_t anglNeuland = 29.579;
  Double_t offxNeuland = distNeuland * sin( anglNeuland*TMath::DegToRad() ); 
  Double_t offyNeuland = 0.;   
  Double_t offzNeuland = distNeuland * cos( anglNeuland*TMath::DegToRad() ) - distTargetZ;

  TGeoRotation*   rotatNeuland = new TGeoRotation("rotatNeuland",90, anglNeuland, 90); //
  TGeoCombiTrans* combiNeuland = new TGeoCombiTrans("combiNeuland",offxNeuland,offyNeuland,offzNeuland,rotatNeuland);
  combiNeuland->RegisterYourself();


  Double_t dxNeuland = 4.7;
  Double_t dyNeuland = 250.;
  Double_t dzNeuland = 4.7;
  Double_t dxNeulandVeto = 32.;
  Double_t dyNeulandVeto = 190.;
  Double_t dzNeulandVeto = 1.;

  TGeoVolume * neulandBox[400];
  TGeoVolume * neulandVetoBox[8];
  for(Int_t i=0; i<400; i++)
  	neulandBox[i] = gGeoManager -> MakeBox(Form("neulandBox%d",i),polyvinyltoluene,dxNeuland/2.,dyNeuland/2.,dzNeuland/2.);
  for(Int_t i=0; i<8; i++)
     neulandVetoBox[i] = gGeoManager -> MakeBox(Form("neulandVetoBox%d",i),polyvinyltoluene,dxNeulandVeto/2.,dyNeulandVeto/2.,dzNeulandVeto/2.);

  Double_t offyNeulandLocal = -124.2;	// neuland bottom plastic position
  Double_t offxNeulandLocal = -122.185;
  Double_t offyNeulandNextBar   = 5.0;
  Double_t offxNeulandNextBar   = 5.0;
  Double_t offzNeulandNextLayer = 5.056;  // distance between layers
  
  TGeoCombiTrans * combiNeulandBar[400];
  TGeoRotation * rotateNeulandY = new TGeoRotation("rotateNeulandY",0,0,90);
  for(Int_t i=0; i<4; i++)
     for(Int_t j=0; j<50; j++){
	combiNeulandBar[i*100+j] = new TGeoCombiTrans(Form("combiNeulandBar%d",i*100+j),0,offyNeulandLocal+offyNeulandNextBar*j,offzNeulandNextLayer*i*2,rotateNeulandY);
	combiNeulandBar[i*100+50+j] = new TGeoCombiTrans(Form("combiNeulandBar%d",i*100+50+j),offxNeulandLocal+offxNeulandNextBar*j,0,offzNeulandNextLayer*(2*i+1),nullptr);

     }
  for(Int_t i=0; i<400; i++)
     combiNeulandBar[i]->RegisterYourself();
  
  Double_t offxVetoLocal = -109.975;
  Double_t offxVetoNextBarLocal = 31.528;
  Double_t offzVetoLocal[2] = {-36.358,-34.058};
  TGeoCombiTrans * combiNeulandVeto[8];
  TGeoRotation * rotateNeulandVeto = new TGeoRotation("rotateNeulandVeto",0,0,0);
  for(Int_t i=0; i<8; i++)
     combiNeulandVeto[i] = new TGeoCombiTrans(Form("combiNeulandVeto%d",i),offxVetoLocal+offxVetoNextBarLocal*i,0,offzVetoLocal[i%2],rotateNeulandVeto);
  for(Int_t i=0; i<8; i++)
     combiNeulandVeto[i]->RegisterYourself();

  for(Int_t i=0; i<400; i++)
	neuland -> AddNode(neulandBox[i],i+1,combiNeulandBar[i]);
  for(Int_t i=0; i<8; i++)
	neuland -> AddNode(neulandVetoBox[i],i+1,combiNeulandVeto[i]);
  
   
  // ----------------------------------------------------
  //  Set Medium
  // ----------------------------------------------------

  active            -> SetMedium(p10);
  cageSide          -> SetMedium(aluminium);
  cageFront         -> SetMedium(aluminium);
  frontWindow       -> SetMedium(kapton);
  frontWindowFrame  -> SetMedium(aluminium);
  frontWindowCradle -> SetMedium(vacuum);
  backWindow        -> SetMedium(kapton);
  backWindowFrame   -> SetMedium(vacuum);
  topFrame          -> SetMedium(vacuum);
  topPlate          -> SetMedium(aluminium);
  bottomPlate       -> SetMedium(vacuum);
  padPlane          -> SetMedium(pcb);
  padArray          -> SetMedium(copper);
  
  enclosureFrame    -> SetMedium(iron);
  plateL            -> SetMedium(aluminium);
  plateR            -> SetMedium(aluminium);
  plateFL           -> SetMedium(aluminium);
  plateFR           -> SetMedium(aluminium);
  plateD            -> SetMedium(aluminium);
  thinWindowLR      -> SetMedium(aluminium);
  thinWindowD       -> SetMedium(aluminium);
  
  p10gas            -> SetMedium(p10);

  for(Int_t ik=0; ik<60; ik++) kyoto[ik] -> SetMedium(polystyrene);
  katanaVPlaL       -> SetMedium(polyvinyltoluene);
  katanaVPlaM       -> SetMedium(polyvinyltoluene);
  katanaVPlaR       -> SetMedium(polyvinyltoluene);

  outerAir          -> SetMedium(air);

  // ----------------------------------------------------
  //  AddNode to TOP
  // ----------------------------------------------------

  top -> AddNode(tpc,1,combiTpc);
  
  top -> AddNode(neuland,1,combiNeuland);
 
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
  tpc -> AddNode(backWindow,1,transBackWindow);
  
  tpc -> AddNode(topFrame,1);
  
  tpc -> AddNode(topPlate,1,transTopPlateVolume);
  
  //tpc -> AddNode(padPlane,1,transPadPlane); // TODO decide whether to add pad-plane to node or not, overlap with topPlate
  //tpc -> AddNode(padArray,1,transPadArray); // TODO decide whether to add pad-plane to node or not, overlap with topPlate



  tpc -> AddNode(p10gas,1);

  tpc -> AddNode(enclosureFrame,1);

  tpc -> AddNode(plateL,1,transPlateL);
  tpc -> AddNode(plateR,1,transPlateR);
  tpc -> AddNode(plateD,1,transPlateD);
  tpc -> AddNode(thinWindowLR,1,transThinWindowL);
  tpc -> AddNode(thinWindowLR,1,transThinWindowR);
  tpc -> AddNode(thinWindowD,1,transThinWindowD);
  tpc -> AddNode(plateFL,1,transPlateFL);
  tpc -> AddNode(plateFR,1,transPlateFR);
  tpc -> AddNode(grassWindow,1,transGrassWindowL);
  tpc -> AddNode(grassWindow,1,transGrassWindowR);
 

  for(Int_t i=0; i<30; i++)
    tpc -> AddNode(kyoto[i],i+1,transKyotoPlaL[i]);
  for(Int_t i=0; i<30; i++)
    tpc -> AddNode(kyoto[i+30],i+1+30,transKyotoPlaR[i]);
  
  tpc -> AddNode(katanaVPlaL,1,transKatanaVPlaL); 
  tpc -> AddNode(katanaVPlaM,2,transKatanaVPlaM); 
  tpc -> AddNode(katanaVPlaR,3,transKatanaVPlaR); 

  tpc -> AddNode(outerAir,1);  



  // ----------------------------------------------------
  //  Visual Attributes 
  // ----------------------------------------------------

  Int_t transparency = 20;
  Int_t transparencyActive = 60 + transparency; if (transparencyActive > 100) transparencyActive = 100;
  Int_t transparencyWindow = 50 + transparency; if (transparencyWindow > 100) transparencyWindow = 100;
  Int_t transparencyWire   = 50 + transparency; if (transparencyWire   > 100) transparencyWire   = 100;
  Int_t transparencyCage   = 40 + transparency; if (transparencyCage   > 100) transparencyCage   = 100;

  //active            -> SetVisibility(false);
  active            -> SetTransparency(transparencyActive);
  cageFront         -> SetTransparency(transparencyCage);
  cageSide          -> SetTransparency(transparencyCage);
  frontWindow       -> SetTransparency(transparencyWindow);
  frontWindowFrame  -> SetTransparency(transparency);
  frontWindowCradle -> SetTransparency(transparency);
  bottomPlate       -> SetTransparency(transparency);
  backWindowFrame   -> SetTransparency(transparency);
  backWindow        -> SetTransparency(transparencyWindow);
  topFrame          -> SetTransparency(transparency);
  topPlate          -> SetTransparency(transparency);
  //padPlane          -> SetTransparency(transparency);
  //padArray          -> SetTransparency(transparency);


  enclosureFrame    -> SetTransparency(transparency);
  plateL            -> SetTransparency(transparency);
  plateR            -> SetTransparency(transparency);
  plateD            -> SetTransparency(transparency);
  thinWindowLR      -> SetTransparency(transparencyWindow);
  thinWindowD       -> SetTransparency(transparencyWindow);
  plateFL           -> SetTransparency(transparency);
  plateFR           -> SetTransparency(transparency);
  grassWindow       -> SetTransparency(transparencyWindow);

  for(Int_t ik=0; ik<60; ik++) kyoto[ik] -> SetTransparency(transparency);
  katanaVPlaL       -> SetTransparency(transparency);
  katanaVPlaM       -> SetTransparency(transparency);
  katanaVPlaR       -> SetTransparency(transparency);

  p10gas            -> SetTransparency(95);
  outerAir          -> SetTransparency(95);



  active            -> SetLineColor(kPink);
  cageFront         -> SetLineColor(kOrange+4);
  cageSide          -> SetLineColor(kOrange+4);
  frontWindow       -> SetLineColor(kOrange);
  frontWindowFrame  -> SetLineColor(10);
  frontWindowCradle -> SetLineColor(15);
  bottomPlate       -> SetLineColor(12);
  backWindowFrame   -> SetLineColor(13);
  backWindow        -> SetLineColor(kOrange);
  topFrame          -> SetLineColor(12);
  topPlate          -> SetLineColor(12);
  //padPlane          -> SetLineColor(kRed+4);
  //padArray          -> SetLineColor(kRed);

  p10gas            -> SetLineColor(kPink);

  enclosureFrame    -> SetLineColor(28);
  plateL            -> SetLineColor(12);
  plateR            -> SetLineColor(12);
  plateFL           -> SetLineColor(12);
  plateFR           -> SetLineColor(12);
  plateD            -> SetLineColor(12);
  thinWindowLR      -> SetLineColor(18);
  thinWindowD       -> SetLineColor(18);
  grassWindow       -> SetLineColor(10);

  for(Int_t ik=0; ik<60; ik++) kyoto[ik] -> SetLineColor(kAzure);
  katanaVPlaL       -> SetLineColor(kGreen);
  katanaVPlaM       -> SetLineColor(kGreen);
  katanaVPlaR       -> SetLineColor(kGreen);

  outerAir          -> SetLineColor(10);

  for(Int_t i=0; i<400; i++) neulandBox[i] -> SetLineColor(kMagenta);
  for(Int_t i=0; i<8; i++) neulandVetoBox[i] -> SetLineColor(kBlue);
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
  
  TString geoFileName    = dirGeom + "geomSpiRIT_nl.root";
  TString geoManFileName = dirGeom + "geomSpiRIT_nl.man.root";

  TFile *geoFile = new TFile(geoFileName,"recreate"); 
  top -> Write(); 
  geoFile -> Close(); 
  cout << geoFileName << " created." << endl;

  TFile *geoManFile = new TFile(geoManFileName,"recreate"); 
  gGeoManager -> Write();
  geoManFile -> Close(); 
  cout << geoManFileName << " created." << endl;

}
