#include "TGeoManager.h" 
#include "TGeoMedium.h"
#include "TGeoMaterial.h"
#include "TGeoVolume.h"
#include <vector>

/*
 * TODO : variables 
 * TODO : Make volume in union volume using TGeoCompositeShape
 * - active            : ok
 * - cageFront         : ok
 * - cageSide          : ok
 * - cageCorner        : ok
 * - frontWindow       : ok
 * - frontWindowFrame  :
 * - copperStrips      :
 * - frontWindowCradle :
 * - bottomPlate       :
 * - backWindowFrame   :
 * - backWindow        :
 * - topFrame          :
 * - ribmain           :
 * - wirePlane         :
 * - padPlane          :

 * TODO : check overlabs
*/

void geomSpiRIT()
{
  TString dir = gSystem -> Getenv("VMCWORKDIR");
  TString dirGeom = dir + "/geometry/";

  // ----------------------------------------------------
  //  Dimensions (cm)
  // ----------------------------------------------------
  /* 
   * "Active"
   * Size of gas volume inside the field cage. 
   * Thickness of cage side(tCageFrame) is not included.
   */ 
  Double_t xCage = 96.61;  ///< x-length of cage volume
  Double_t yCage = 51.01;  ///< y-length of cage volume, from cathode top to pad-plane bottom 
  Double_t zCage = 144.64; ///< z-length of cage volume

  TVector3 sTOP(200., yCage/2., 200.); ///< size of top volume
  TVector3 sTPC(200., yCage/2., 200.); ///< size of tpc volume

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
  TGeoVolume* top = gGeoManager -> MakeBox("top",vacuum,sTOP.X(),sTOP.Y(),sTOP.Z());
  gGeoManager -> SetTopVolume(top);

  // ----------------------------------------------------
  //  TPC
  // ----------------------------------------------------
  TGeoVolume* tpc = gGeoManager -> MakeBox("tpc",vacuum,sTPC.X(),sTPC.Y(),sTPC.Z());

  TGeoRotation*   rotatTPC = new TGeoRotation("rotatTPC",180.0,180.0,0.0);
  TGeoCombiTrans* combiTPC  = new TGeoCombiTrans("combiTPC",0.0,-27.5305,5.08548+134.4/2.0,rotatTPC);

  // ----------------------------------------------------
  //  Active Volume (Union)
  // ----------------------------------------------------
  TGeoVolume* activeCenter  = gGeoManager -> MakeBox("activeCenter",p10,xActiveC/2,yActiveC/2,zActiveC/2);
  TGeoVolume* activeTop     = gGeoManager -> MakeBox("activeTop",   p10,xActiveC/2,yActiveC/2,rActiveCorner/2);
  TGeoVolume* activeBottom  = gGeoManager -> MakeBox("activeBottom",p10,xActiveC/2,yActiveC/2,rActiveCorner/2);
  TGeoVolume* activeRight   = gGeoManager -> MakeBox("activeRight", p10,rActiveCorner/2,yActiveC/2,zActiveC/2);
  TGeoVolume* activeLeft    = gGeoManager -> MakeBox("activeLeft",  p10,rActiveCorner/2,yActiveC/2,zActiveC/2);
  TGeoVolume* activeCorner1 = gGeoManager -> MakeTubs("activeCorner1",p10,0.,rActiveCorner,yActiveC/2,0,   90. );
  TGeoVolume* activeCorner2 = gGeoManager -> MakeTubs("activeCorner2",p10,0.,rActiveCorner,yActiveC/2,90., 180.);
  TGeoVolume* activeCorner3 = gGeoManager -> MakeTubs("activeCorner3",p10,0.,rActiveCorner,yActiveC/2,180.,270.);
  TGeoVolume* activeCorner4 = gGeoManager -> MakeTubs("activeCorner4",p10,0.,rActiveCorner,yActiveC/2,270.,360.);

  Double_t offXActive = xActiveC/2+rActiveCorner/2;
  Double_t offZActive = zActiveC/2+rActiveCorner/2;

  TGeoTranslation* transActiveTop     = new TGeoTranslation("transActiveTop",   0,0, offZActive);
  TGeoTranslation* transActiveBottom  = new TGeoTranslation("transActiveBottom",0,0,-offZActive);
  TGeoTranslation* transActiveRight   = new TGeoTranslation("transActiveRight",  offXActive,0,0);
  TGeoTranslation* transActiveLeft    = new TGeoTranslation("transActiveLeft",  -offXActive,0,0);
  TGeoRotation*    rotatCorner        = new TGeoRotation("rotatCorner",0,90,0);
  TGeoCombiTrans*  combiActiveCorner1 = new TGeoCombiTrans("combiActiveCorner1", xActiveC/2,0, zActiveC/2,rotatCorner);
  TGeoCombiTrans*  combiActiveCorner2 = new TGeoCombiTrans("combiActiveCorner2",-xActiveC/2,0, zActiveC/2,rotatCorner);
  TGeoCombiTrans*  combiActiveCorner3 = new TGeoCombiTrans("combiActiveCorner3",-xActiveC/2,0,-zActiveC/2,rotatCorner);
  TGeoCombiTrans*  combiActiveCorner4 = new TGeoCombiTrans("combiActiveCorner4", xActiveC/2,0,-zActiveC/2,rotatCorner);

  transActiveTop     -> RegisterYourself();
  transActiveBottom  -> RegisterYourself();
  transActiveRight   -> RegisterYourself();
  transActiveLeft    -> RegisterYourself();
  combiActiveCorner1 -> RegisterYourself();
  combiActiveCorner2 -> RegisterYourself();
  combiActiveCorner3 -> RegisterYourself();
  combiActiveCorner4 -> RegisterYourself();

  TGeoCompositeShape* activeComposite
    = new TGeoCompositeShape("activeComposite","activeCenter+activeTop:transActiveTop+activeBottom:transActiveBottom+activeRight:transActiveRight+activeLeft:transActiveLeft+activeCorner1:combiActiveCorner1+activeCorner2:combiActiveCorner2+activeCorner3:combiActiveCorner3+activeCorner4:combiActiveCorner4");
  TGeoVolume *active = new TGeoVolume("field_cage_in",activeComposite);
  active -> SetMedium(p10);

  // ----------------------------------------------------
  //  Field Cage Corner Frame (what is this ???)
  // ----------------------------------------------------
  TGeoVolume *cageCorner1 = gGeoManager->MakeTubs("cageCorner1",aluminium,rActiveCorner,rCageCorner,yActiveC/2,0  ,90 );
  TGeoVolume *cageCorner2 = gGeoManager->MakeTubs("cageCorner2",aluminium,rActiveCorner,rCageCorner,yActiveC/2,90 ,180);
  TGeoVolume *cageCorner3 = gGeoManager->MakeTubs("cageCorner3",aluminium,rActiveCorner,rCageCorner,yActiveC/2,180,270);
  TGeoVolume *cageCorner4 = gGeoManager->MakeTubs("cageCorner4",aluminium,rActiveCorner,rCageCorner,yActiveC/2,270,360);

  TGeoCompositeShape* cageCornerComposite = new TGeoCompositeShape("cageCornerComposite", "cageCorner1:combiActiveCorner1+cageCorner2:combiActiveCorner2+cageCorner3:combiActiveCorner3+cageCorner4:combiActiveCorner4");
  TGeoVolume *cageCorner = new TGeoVolume("cageCorner",cageCornerComposite);
  cageCorner -> SetMedium(aluminium);

  // ----------------------------------------------------
  //  Field Cage Frame
  // ----------------------------------------------------
  Double_t xCageFrontRL = 40.48;
  Double_t xCageFrontTB = xActiveC - 2*xCageFrontRL;
  Double_t yCageFrontT  = 13.0;
  Double_t yCageFrontB  = 19.6;

  TGeoVolume* cageRight  = gGeoManager -> MakeBox("cageRight" ,pcb,tCageFrame/2,yActiveC/2,zActiveC/2);
  TGeoVolume* cageLeft   = gGeoManager -> MakeBox("cageLeft"  ,pcb,tCageFrame/2,yActiveC/2,zActiveC/2);
  TGeoVolume* cageFrontR = gGeoManager -> MakeBox("cageFrontR",pcb,xCageFrontRL/2,yActiveC/2,tCageFrame/2);  ///< right section of field front cage frame
  TGeoVolume* cageFrontL = gGeoManager -> MakeBox("cageFrontL",pcb,xCageFrontRL/2,yActiveC/2,tCageFrame/2);  ///< left section of field front cage frame
  TGeoVolume* cageFrontT = gGeoManager -> MakeBox("cageFrontT",pcb,xCageFrontTB/2,yCageFrontT/2,tCageFrame/2); ///< top section of field front cage frame
  TGeoVolume* cageFrontB = gGeoManager -> MakeBox("cageFrontB",pcb,xCageFrontTB/2,yCageFrontB/2,tCageFrame/2); ///< bottom section of field front cage frame

  Double_t offXCageSide   = tCageFrame/2+xActiveC/2+rActiveCorner;
  Double_t offZCageFront  =  zActiveC/2+rActiveCorner+tCageFrame/2;
  Double_t offXCageFrontR =  xActiveC/2-xCageFrontRL/2;
  Double_t offXCageFrontL = -xActiveC/2+xCageFrontRL/2;
  Double_t offYCageFrontT =  yActiveC/2-yCageFrontT/2;
  Double_t offYCageFrontB = -yActiveC/2+yCageFrontB/2;

  TGeoTranslation* transCageRight  = new TGeoTranslation("transCageRight", offXCageSide,0,0);
  TGeoTranslation* transCageLeft   = new TGeoTranslation("transCageLeft", -offXCageSide,0,0);
  TGeoTranslation* transCageFrontR = new TGeoTranslation("transCageFrontR",offXCageFrontR,0,offZCageFront);
  TGeoTranslation* transCageFrontL = new TGeoTranslation("transCageFrontL",offXCageFrontL,0,offZCageFront);
  TGeoTranslation* transCageFrontT = new TGeoTranslation("transCageFrontT",0,offYCageFrontT,offZCageFront);
  TGeoTranslation* transCageFrontB = new TGeoTranslation("transCageFrontB",0,offYCageFrontB,offZCageFront);

  transCageRight  -> RegisterYourself();
  transCageLeft   -> RegisterYourself();
  transCageFrontR -> RegisterYourself();
  transCageFrontL -> RegisterYourself();
  transCageFrontT -> RegisterYourself();
  transCageFrontB -> RegisterYourself();

  TGeoCompositeShape* cageSideComposite = new TGeoCompositeShape("cageSideComposite","cageRight:transCageRight+cageLeft:transCageLeft");
  TGeoVolume *cageSide = new TGeoVolume("cageSide",cageSideComposite);
  cageSide -> SetMedium(pcb);

  TGeoCompositeShape* cageFrontComposite = new TGeoCompositeShape("cageFrontComposite","cageFrontR:transCageFrontR+cageFrontL:transCageFrontL+cageFrontT:transCageFrontT+cageFrontB:transCageFrontB");
  TGeoVolume *cageFront = new TGeoVolume("cageFront",cageFrontComposite);
  cageFront -> SetMedium(pcb);

  // ----------------------------------------------------
  //  Front Widow & Front Window Frame (from Mike's macro)
  // ----------------------------------------------------
  Double_t fwindowz=0.005;
  Double_t fwindowy=7.0;
  Double_t fwindowx1=5.73;
  Double_t fwindowx2=7.0;
  Double_t fwinframesidex=1.45;
  Double_t fwinframesidey=16.9;
  Double_t fwinframez=0.005;
  Double_t fwinframetopx=7.0;
  Double_t fwinframetopy=1.45;
  Double_t fwinframebotx=7.0;
  Double_t fwinframeboty=8.45;
  Double_t trfwinframesidex=fwinframesidex/2.+fwinframetopx/2.;
  Double_t trfwinframesidey=offYCageFrontT-yCageFrontT/2.0-0.05-fwinframesidey/2.;
  Double_t trfwinframetopy=offYCageFrontT-yCageFrontT/2.-0.05-fwinframetopy/2.;
  Double_t trfwinframeboty=offYCageFrontT-yCageFrontT/2.-0.05-fwinframetopy-fwindowy-fwinframeboty/2.;
  Double_t trfwindowy=offYCageFrontT-yCageFrontT/2.-0.05-fwinframetopy-fwindowy/2.;
  Double_t trfwindowz=zActiveC/2.+rActiveCorner+fwindowz/2.;
  Double_t trfwinframez=trfwindowz;
  Double_t trapbase=0.635;
  Double_t traptheta=TMath::RadToDeg()*TMath::ATan((trapbase/2.0)/fwinframez);
  Double_t trtrapx=0.16-3.50125;

  TGeoVolume *frontWindowFrameR = gGeoManager->MakeBox("frontWindowFrameR",aluminium,fwinframesidex/2.,fwinframesidey/2.,fwinframez/2.);
  TGeoVolume *frontWindowFrameL = gGeoManager->MakeBox("frontWindowFrameL",aluminium,fwinframesidex/2.,fwinframesidey/2.,fwinframez/2.);
  TGeoVolume *frontWindowFrameT = gGeoManager->MakeBox("frontWindowFrameT",aluminium,fwinframetopx/2.,fwinframetopy/2.,fwinframez/2.);
  TGeoVolume *frontWindowFrameB = gGeoManager->MakeBox("frontWindowFrameB",aluminium,fwinframebotx/2.,fwinframeboty/2.,fwinframez/2.);
  TGeoVolume *frontWindowTrapR  = gGeoManager->MakeTrap("frontWindowTrapR",aluminium,fwinframez/2.0,traptheta,0.0,fwindowy/2.0,trapbase/2.0,trapbase/2.0,0.0,fwindowy/2.0,0.0,0.0,0.0);
  TGeoVolume *frontWindowTrapL  = gGeoManager->MakeTrap("frontWindowTrapL",aluminium,fwinframez/2.0,-traptheta,0.0,fwindowy/2.0,trapbase/2.0,trapbase/2.0,0.0,fwindowy/2.0,0.0,0.0,0.0);

  TGeoTranslation *trfwinframel = new TGeoTranslation("transFrontWindowFrameL",trfwinframesidex,trfwinframesidey,trfwinframez);
  TGeoTranslation *trfwinframer = new TGeoTranslation("transFrontWindowFrameR",-trfwinframesidex,trfwinframesidey,trfwinframez);
  TGeoTranslation *trfwinframet = new TGeoTranslation("transFrontWindowFrameT",0.0,trfwinframetopy,trfwinframez);
  TGeoTranslation *trfwinframeb = new TGeoTranslation("transFrontWindowFrameB",0.0,trfwinframeboty,trfwinframez);
  TGeoTranslation *trfwintrapl  = new TGeoTranslation("transFrontWindowTrapL",trtrapx,trfwindowy,trfwinframez);
  TGeoTranslation *trfwintrapr  = new TGeoTranslation("transFrontWindowTrapR",-trtrapx,trfwindowy,trfwinframez);

  trfwinframel -> RegisterYourself();
  trfwinframer -> RegisterYourself();
  trfwinframet -> RegisterYourself();
  trfwinframeb -> RegisterYourself();
  trfwintrapl  -> RegisterYourself();
  trfwintrapr  -> RegisterYourself();

  TGeoVolume *frontWindow = gGeoManager->MakeTrd1("frontWindow",kapton,fwindowx1/2.,fwindowx2/2.,fwindowy/2.0,fwindowz/2.0);
  TGeoTranslation *trfwindow = new TGeoTranslation(0.0,trfwindowy,trfwinframez);

  TGeoCompositeShape* frontWindowFrameComposite = new TGeoCompositeShape("frontWindowFrameComposite","frontWindowFrameR:transFrontWindowFrameR+frontWindowFrameL:transFrontWindowFrameL+frontWindowFrameT:transFrontWindowFrameT+frontWindowFrameB:transFrontWindowFrameB+frontWindowTrapR:transFrontWindowTrapR+frontWindowTrapL:transFrontWindowTrapL");
  TGeoVolume *frontWindowFrame = new TGeoVolume("frontWindowFrame",frontWindowFrameComposite);
  frontWindowFrame -> SetMedium(aluminium);

  // ----------------------------------------------------
  //  Copper Strips (from Mike's macro)
  // ----------------------------------------------------
  Double_t stripx=0.004;
  Double_t stripy=0.6;
  Double_t stripz=zActiveC;
  Double_t halfstripy=0.3;
  Double_t trstripoutx=xActiveC/2.+rActiveCorner+tCageFrame+stripx/2.;
  Double_t trstripinx=xActiveC/2.+rActiveCorner-stripx/2.;
  Double_t trstripouty1=yActiveC/2.-0.2-stripy/2.;
  Double_t trstripiny1=trstripouty1-0.5;
  Double_t trhalfstripy=yActiveC/2.-halfstripy/2.;
  Double_t fstriplx=2.0*xCageFrontRL+xCageFrontTB;
  Double_t fstripsx=xCageFrontRL;
  Double_t trfoutstripz=zActiveC/2.+rActiveCorner+tCageFrame+stripx/2.;
  Double_t trfstripsx=offXCageFrontR;
  Double_t fhalfstripsx=fstriplx-2*fstripsx;

  Double_t trfinstripz=zActiveC/2.+rActiveCorner-stripx/2.;
  Double_t trfhalfstripsyup=trstripiny1-1.0*12+halfstripy/2.;
  Double_t trfhalfstripsydown=trstripiny1-1.0*29-halfstripy/2.;

  TGeoVolume *copperStrips = gGeoManager -> MakeBox("copperStrips",vacuum,200.,200.,200.);
              copperStrips -> SetVisibility(kFALSE);
  TGeoVolume *strip        = gGeoManager -> MakeBox("strip",copper,stripx/2.,stripy/2.,stripz/2.);
  TGeoVolume *halfstrip    = gGeoManager -> MakeBox("halfstrip",copper,stripx/2.,halfstripy/2.,stripz/2.);
  TGeoVolume *fstripl      = gGeoManager -> MakeBox("fstripl",copper,fstriplx/2.,stripy/2.,stripx/2.);
  TGeoVolume *fstrips      = gGeoManager -> MakeBox("fstrips",copper,fstripsx/2.,stripy/2.,stripx/2.);
  TGeoVolume *fhalfstripl  = gGeoManager -> MakeBox("fhalfstripl",copper,fstriplx/2.,halfstripy/2.,stripx/2.);
  TGeoVolume *fhalfstrips  = gGeoManager -> MakeBox("fhalfstrips",copper,fhalfstripsx/2.,halfstripy/2.,stripx/2.);

  Int_t colorStrip = kOrange+1;
  strip       -> SetLineColor(colorStrip);
  halfstrip   -> SetLineColor(colorStrip);
  fstripl     -> SetLineColor(colorStrip);
  fstrips     -> SetLineColor(colorStrip);
  fhalfstripl -> SetLineColor(colorStrip);
  fhalfstrips -> SetLineColor(colorStrip);

  for (Int_t i=0; i<49; i++) {
    copperStrips->AddNode(strip,i+1,new TGeoTranslation(trstripoutx,trstripouty1-1.0*i,0.0));
    copperStrips->AddNode(strip,i+1+49,new TGeoTranslation(trstripinx,trstripiny1-1.0*i,0.0));
    copperStrips->AddNode(strip,i+1+2*49,new TGeoTranslation(-trstripoutx,trstripouty1-1.0*i,0.0));
    copperStrips->AddNode(strip,i+1+3*49,new TGeoTranslation(-trstripinx,trstripiny1-1.0*i,0.0));
  }

  copperStrips->AddNode(halfstrip,1,new TGeoTranslation(trstripinx,trhalfstripy,0.0));
  copperStrips->AddNode(halfstrip,2,new TGeoTranslation(-trstripinx,trhalfstripy,0.0));

  for (Int_t i=0; i<49; i++) {
    if (i<13) copperStrips->AddNode(fstripl,i+1,new TGeoTranslation(0.0,trstripouty1-1.0*i,trfoutstripz));
    if (i>=13 && i<30) {
      copperStrips->AddNode(fstrips,i+1-13,new TGeoTranslation(trfstripsx,trstripouty1-1.0*i,trfoutstripz));
      copperStrips->AddNode(fstrips,i+1+4,new TGeoTranslation(-trfstripsx,trstripouty1-1.0*i,trfoutstripz));
    }
    if (i>=30) copperStrips->AddNode(fstripl,i+1-17,new TGeoTranslation(0.0,trstripouty1-1.0*i,trfoutstripz));

    if (i<12) copperStrips->AddNode(fstripl,33+i+1,new TGeoTranslation(0.0,trstripiny1-1.0*i,trfinstripz));
    if (i>=12 && i<30) {
      copperStrips->AddNode(fstrips,35+i-12,new TGeoTranslation(trfstripsx,trstripiny1-1.0*i,trfinstripz));
      copperStrips->AddNode(fstrips,35+i+6,new TGeoTranslation(-trfstripsx,trstripiny1-1.0*i,trfinstripz));
	}
    if (i>=30) copperStrips->AddNode(fstripl,33+i+1-18,new TGeoTranslation(0.0,trstripiny1-1.0*i,trfinstripz));
  }

  copperStrips->AddNode(fhalfstripl,1,new TGeoTranslation(0.0,trhalfstripy,trfinstripz));
  copperStrips->AddNode(fhalfstrips,1,new TGeoTranslation(0.0,trfhalfstripsyup,trfinstripz));
  copperStrips->AddNode(fhalfstrips,2,new TGeoTranslation(0.0,trfhalfstripsydown,trfinstripz));  

  // ----------------------------------------------------
  //  Front Window Cradle (from Mike's macro)
  // ----------------------------------------------------
  Double_t fwincradletoptopx=6.35;
  Double_t fwincradletoptopy=9.19;
  Double_t fwincradlez=1.27;
  Double_t trfwincradlez=zActiveC/2.+rActiveCorner+tCageFrame+stripx+fwincradlez/2.;
  Double_t trfwincradletoptopy=yActiveC/2.-fwincradletoptopy/2.;
  Double_t fwincradlebotboty=15.79;
  Double_t trfwincradlebotboty=-yActiveC/2.+fwincradlebotboty/2.;
  Double_t fwincradlemiddlebottomx=17.62;
  Double_t fwincradlemiddlebottomy=5.31;
  Double_t trfwincradlemiddlebottomy=trfwincradlebotboty+fwincradlebotboty/2.+fwincradlemiddlebottomy/2.;
  Double_t trfwincradlemiddletopy=trfwincradletoptopy-fwincradletoptopy/2.-fwincradlemiddlebottomy/2.;
  Double_t fwincradlemiddlesidex=5.31;
  Double_t fwincradlemiddlesidey=14.0;
  Double_t trfwincradlemiddlesidex=fwincradlemiddlebottomx/2.-fwincradlemiddlesidex/2.;
  Double_t trfwincradlemiddlesidey=yActiveC/2.-fwincradletoptopy-fwincradlemiddlebottomy-fwincradlemiddlesidey/2.;

  TGeoVolumeAssembly *frontWindowCradle = gGeoManager->MakeVolumeAssembly("frontWindowCradle");
                      frontWindowCradle -> SetMedium(vacuum);
  TGeoVolume *fwincradletoptop       = gGeoManager->MakeBox("fwincradletoptop",aluminium,fwincradletoptopx/2.,fwincradletoptopy/2.,fwincradlez/2.);
  TGeoVolume *fwincradlebottombottom = gGeoManager->MakeBox("fwincradlebottombottom",aluminium,fwincradletoptopx/2.,fwincradlebotboty/2.,fwincradlez/2.);
  TGeoVolume *fwincradlemiddlebottom = gGeoManager->MakeBox("fwincradlemiddlebottom",aluminium,fwincradlemiddlebottomx/2.,fwincradlemiddlebottomy/2.,fwincradlez/2.);
  TGeoVolume *fwincradlemiddleside   = gGeoManager->MakeBox("fwincradlemiddleside",aluminium,fwincradlemiddlesidex/2.,fwincradlemiddlesidey/2.,fwincradlez/2.);

  //Int_t colorCradle = kBlue+3;
  Int_t colorCradle = 15;
  fwincradletoptop       -> SetLineColor(colorCradle);
  fwincradlebottombottom -> SetLineColor(colorCradle);
  fwincradlemiddlebottom -> SetLineColor(colorCradle);
  fwincradlemiddleside   -> SetLineColor(colorCradle);

  TGeoTranslation *trfwincradletoptop = new TGeoTranslation(0.0,trfwincradletoptopy,trfwincradlez);
  TGeoTranslation *trfwincradlebottombottom = new TGeoTranslation(0.0,trfwincradlebotboty,trfwincradlez);
  TGeoTranslation *trfwincradlemiddlebottom = new TGeoTranslation(0.0,trfwincradlemiddlebottomy,trfwincradlez);
  TGeoTranslation *trfwincradlemiddletop = new TGeoTranslation(0.0,trfwincradlemiddletopy,trfwincradlez);
  TGeoTranslation *trfwincradlemiddleleft = new TGeoTranslation(trfwincradlemiddlesidex,trfwincradlemiddlesidey,trfwincradlez);
  TGeoTranslation *trfwincradlemiddleright = new TGeoTranslation(-trfwincradlemiddlesidex,trfwincradlemiddlesidey,trfwincradlez);

  frontWindowCradle->AddNode(fwincradletoptop,1,trfwincradletoptop);
  frontWindowCradle->AddNode(fwincradlebottombottom,1,trfwincradlebottombottom);
  frontWindowCradle->AddNode(fwincradlemiddlebottom,1,trfwincradlemiddlebottom);
  frontWindowCradle->AddNode(fwincradlemiddlebottom,2,trfwincradlemiddletop);
  frontWindowCradle->AddNode(fwincradlemiddleside,1,trfwincradlemiddleleft);
  frontWindowCradle->AddNode(fwincradlemiddleside,2,trfwincradlemiddleright);

  // ----------------------------------------------------
  //  Bottom Plate (from Mike's macro)
  // ----------------------------------------------------
  //TGeoVolume *bottomPlate = gGeoManager->MakeBox("bottomPlate",vacuum,200.,200.,200.);
  TGeoVolumeAssembly *bottomPlate = gGeoManager->MakeVolumeAssembly("bottomPlate");
  bottomPlate -> SetMedium(vacuum);
  bottomPlate -> SetVisibility(kFALSE);
  Double_t bottomcenterx=94.6;
  Double_t bottomcentery=1.0541;
  Double_t bottomcenterz=149.86;
  TGeoVolume *bottomcenter = gGeoManager->MakeBox("bottomcenter",aluminium,bottomcenterx/2.,bottomcentery/2.,bottomcenterz/2.);
  Double_t trbottomcentery=-yActiveC/2.-bottomcentery/2.;
  TGeoTranslation *trbottomcenter = new TGeoTranslation(0.0,trbottomcentery,0.0);
  Double_t bottomradius=3.5;
  Double_t bottomsidex=bottomradius;
  Double_t bottomsidey=bottomcentery;
  Double_t bottomsidez=bottomcenterz-2*bottomradius;
  TGeoVolume *bottomoutr = gGeoManager->MakeBox("bottomoutr",aluminium,bottomsidex/2.,bottomsidey/2.,bottomsidez/2.);
  TGeoVolume *bottomoutl = gGeoManager->MakeBox("bottomoutl",aluminium,bottomsidex/2.,bottomsidey/2.,bottomsidez/2.);
  Double_t trbottomsidex=bottomcenterx/2.+bottomsidex/2.;
  TGeoTranslation *trbottomoutsidel = new TGeoTranslation(trbottomsidex,trbottomcentery,0.0);
  TGeoTranslation *trbottomoutsider = new TGeoTranslation(-trbottomsidex,trbottomcentery,0.0);
  TGeoVolume *bpcorner1 = gGeoManager->MakeTubs("bpcorner1",aluminium,0.0,bottomradius,bottomcentery/2.0,0.0,90.0);
  TGeoVolume *bpcorner2 = gGeoManager->MakeTubs("bpcorner2",aluminium,0.0,bottomradius,bottomcentery/2.0,90.0,180.0);
  TGeoVolume *bpcorner3 = gGeoManager->MakeTubs("bpcorner3",aluminium,0.0,bottomradius,bottomcentery/2.0,180.0,270.0);
  TGeoVolume *bpcorner4 = gGeoManager->MakeTubs("bpcorner4",aluminium,0.0,bottomradius,bottomcentery/2.0,270.0,360.0);
  TGeoCombiTrans *cobpcorner1 = new TGeoCombiTrans("cobpcorner1",bottomcenterx/2.0,trbottomcentery,bottomsidez/2.0,rotatCorner);
  TGeoCombiTrans *cobpcorner2 = new TGeoCombiTrans("cobpcorner2",-bottomcenterx/2.0,trbottomcentery,bottomsidez/2.0,rotatCorner);
  TGeoCombiTrans *cobpcorner3 = new TGeoCombiTrans("cobpcorner3",-bottomcenterx/2.0,trbottomcentery,-bottomsidez/2.0,rotatCorner);
  TGeoCombiTrans *cobpcorner4 = new TGeoCombiTrans("cobpcorner4",bottomcenterx/2.0,trbottomcentery,-bottomsidez/2.0,rotatCorner);

  bottomPlate->AddNode(bottomcenter,1,trbottomcenter);
  bottomPlate->AddNode(bottomoutl,1,trbottomoutsidel);
  bottomPlate->AddNode(bottomoutr,1,trbottomoutsider);
  bottomPlate->AddNode(bpcorner1,1,cobpcorner1);
  bottomPlate->AddNode(bpcorner2,1,cobpcorner2);
  bottomPlate->AddNode(bpcorner3,1,cobpcorner3);
  bottomPlate->AddNode(bpcorner4,1,cobpcorner4);

  Int_t colorBottomPlate = 18;
  bottomcenter->SetLineColor(colorBottomPlate);
  bottomoutr->SetLineColor(colorBottomPlate);
  bottomoutl->SetLineColor(colorBottomPlate);
  bpcorner1->SetLineColor(colorBottomPlate);
  bpcorner2->SetLineColor(colorBottomPlate);
  bpcorner3->SetLineColor(colorBottomPlate);
  bpcorner4->SetLineColor(colorBottomPlate);

  // ----------------------------------------------------
  //  Back Window Frame (from Mike's macro)
  // ----------------------------------------------------
  Double_t backwindowtopx=89.6584;
  Double_t backwindowtopy=3.15;
  Double_t backwindowtopz=1.27;
  //TGeoVolume *backWindowFrame = gGeoManager->MakeBox("backWindowFrame",vacuum,200.,200.,200.);
  TGeoVolumeAssembly *backWindowFrame = gGeoManager->MakeVolumeAssembly("backWindowFrame");
  backWindowFrame -> SetMedium(vacuum);
  backWindowFrame->SetVisibility(kFALSE);
  TGeoVolume *backwindowtop = gGeoManager->MakeBox("backwindowtop",aluminium,backwindowtopx/2.0,backwindowtopy/2.0,backwindowtopz/2.0);
  Double_t trbackwindowtopy=yActiveC/2.0-backwindowtopy/2.0;
  TGeoTranslation *trbackwindowtop = new TGeoTranslation(0.0,trbackwindowtopy,0.0);
  Double_t backwindowbottomx=89.6584;
  Double_t backwindowbottomy=3.55;
  Double_t backwindowbottomz=1.27;
  TGeoVolume *backwindowbottom = gGeoManager->MakeBox("backwindowbottom",aluminium,backwindowbottomx/2.0,backwindowbottomy/2.0,backwindowbottomz/2.0);
  Double_t trbackwindowbottomy=-yActiveC/2.0+backwindowbottomy/2.0;
  TGeoTranslation *trbackwindowbottom = new TGeoTranslation(0.0,trbackwindowbottomy,0.0);
  Double_t backwindowsidex=2.4492;
  Double_t backwindowsidey=42.9;
  Double_t backwindowsidez=1.27;
  TGeoVolume *backwindowside = gGeoManager->MakeBox("backwindowside",aluminium,backwindowsidex/2.0,backwindowsidey/2.0,backwindowsidez/2.0);
  Double_t trbackwindowsidex=backwindowtopx/2.0-backwindowsidex/2.0;
  Double_t trbackwindowsidey=(backwindowbottomy-backwindowtopy)/2.0;
  TGeoTranslation *trbackwindowleft = new TGeoTranslation(trbackwindowsidex,trbackwindowsidey,0.0);
  TGeoTranslation *trbackwindowright = new TGeoTranslation(-trbackwindowsidex,trbackwindowsidey,0.0);
  Double_t trbackwindowframez=zActiveC/2.0+rActiveCorner+backwindowtopz/2.0;
  TGeoTranslation *trbackwindowframe = new TGeoTranslation(0.0,0.0,-trbackwindowframez);
  
  Double_t backwindowtop2x=89.76;
  Double_t backwindowtop2y=4.5;
  Double_t backwindowtop2z=0.635;
  Double_t backwindowside2x=4.5;
  Double_t backwindowside2y=38.9;
  Double_t backwindowside2z=0.635;
  Double_t backwindowbottom2x=89.76;
  Double_t backwindowbottom2y=4.8475;
  Double_t backwindowbottom2z=0.635;
  TGeoVolume *backwindowtop2 = gGeoManager->MakeBox("backwindowtop2",aluminium,backwindowtop2x/2.0,backwindowtop2y/2.0,backwindowtop2z/2.0);
  Double_t trbackwindowtop2y=backwindowbottom2y/2.0+backwindowside2y/2.0;
  Double_t trbackwindowtop2z=-backwindowtop2z/2.0-backwindowtopz/2.0;
  TGeoTranslation *trbackwindowtop2 = new TGeoTranslation(0.0,trbackwindowtop2y,trbackwindowtop2z);

  TGeoVolume *backwindowbottom2 = gGeoManager->MakeBox("backwindowbottom2",aluminium,backwindowbottom2x/2.0,backwindowbottom2y/2.0,backwindowbottom2z/2.0);
  Double_t trbackwindowbottom2y=-backwindowside2y/2.0-backwindowtop2y/2.0;
  Double_t trbackwindowbottom2z=trbackwindowtop2z;
  TGeoTranslation *trbackwindowbottom2 = new TGeoTranslation(0.0,trbackwindowbottom2y,trbackwindowbottom2z);

  TGeoVolume *backwindowside2 = gGeoManager->MakeBox("backwindowside2",aluminium,backwindowside2x/2.0,backwindowside2y/2.0,backwindowside2z/2.0);
  Double_t trbackwindowside2x=backwindowtop2x/2.0-backwindowside2x/2.0;
  Double_t trbackwindowside2z=trbackwindowtop2z;
  Double_t trbackwindowside2y=(backwindowbottom2y-backwindowtop2y)/2.0;
  TGeoTranslation *trbackwindowleft2 = new TGeoTranslation(trbackwindowside2x,trbackwindowside2y,trbackwindowside2z);
  TGeoTranslation *trbackwindowright2 = new TGeoTranslation(-trbackwindowside2x,trbackwindowside2y,trbackwindowside2z);

  backWindowFrame->AddNode(backwindowtop,1,trbackwindowtop);
  backWindowFrame->AddNode(backwindowbottom,1,trbackwindowbottom);
  backWindowFrame->AddNode(backwindowside,1,trbackwindowleft);
  backWindowFrame->AddNode(backwindowside,1,trbackwindowright);
  backWindowFrame->AddNode(backwindowtop2,1,trbackwindowtop2);
  backWindowFrame->AddNode(backwindowbottom2,1,trbackwindowbottom2);
  backWindowFrame->AddNode(backwindowside2,1,trbackwindowleft2);
  backWindowFrame->AddNode(backwindowside2,2,trbackwindowright2);

  Int_t colorBackWindow = 13;
  Int_t colorBackWindow2 = 15;
  backwindowtop     -> SetLineColor(colorBackWindow);
  backwindowbottom  -> SetLineColor(colorBackWindow);
  backwindowside    -> SetLineColor(colorBackWindow);
  backwindowside    -> SetLineColor(colorBackWindow);
  backwindowtop2    -> SetLineColor(colorBackWindow2);
  backwindowbottom2 -> SetLineColor(colorBackWindow2);
  backwindowside2   -> SetLineColor(colorBackWindow2);
  backwindowside2   -> SetLineColor(colorBackWindow2);

  // ----------------------------------------------------
  //  Back Window (from Mike's macro)
  // ----------------------------------------------------
  Double_t backwindowx=89.76;
  Double_t backwindowy=48.2475;
  Double_t backwindowz=0.0125; // TODO
  TGeoVolume *backWindow = gGeoManager->MakeBox("backWindow",kapton,backwindowx/2.0,backwindowy/2.0,backwindowz/2.0);
  Double_t trbackwindowz=-zActiveC/2.0-rActiveCorner-backwindowtopz-backwindowtop2z-backwindowz/2.0;
  TGeoTranslation *trbackwindow = new TGeoTranslation(0.0,0.0,trbackwindowz);
  
  // ----------------------------------------------------
  //  Top Frame (from Mike's macro)
  // ----------------------------------------------------
  //TGeoVolume *topFrame = gGeoManager->MakeBox("topFrame",vacuum,200.,200.,200.);
  TGeoVolumeAssembly *topFrame = gGeoManager->MakeVolumeAssembly("topFrame");
  topFrame -> SetMedium(vacuum);
  topFrame->SetVisibility(kFALSE);
  Double_t topframesidex=17.5326;
  Double_t topframesidey=0.8763;
  Double_t topframesidez=160.0175;
  TGeoVolume *topframeside = gGeoManager->MakeBox("topframeside",aluminium,topframesidex/2.0,topframesidey/2.0,topframesidez/2.0);
  Double_t topframefbx=95.8523;
  Double_t topframefby=0.8763;
  Double_t topframefbz=8.0594;
  TGeoVolume *topframefb = gGeoManager->MakeBox("topframefb",aluminium,topframefbx/2.0,topframefby/2.0,topframefbz/2.0);
  Double_t trtopframesidex=topframefbx/2.0+topframesidex/2.0;
  Double_t trtopframesidey=topframefby/2.0+yActiveC/2.0;
  TGeoTranslation *trtopframeleft = new TGeoTranslation(trtopframesidex,trtopframesidey,0.0);
  TGeoTranslation *trtopframeright = new TGeoTranslation(-trtopframesidex,trtopframesidey,0.0);
  Double_t trtopframefbz=topframesidez/2.0-topframefbz/2.0;
  TGeoTranslation *trtopframefront = new TGeoTranslation(0.0,trtopframesidey,trtopframefbz);
  TGeoTranslation *trtopframeback = new TGeoTranslation(0.0,trtopframesidey,-trtopframefbz);
  topFrame->AddNode(topframeside,1,trtopframeleft);
  topFrame->AddNode(topframeside,2,trtopframeright);
  topFrame->AddNode(topframefb,1,trtopframefront);
  topFrame->AddNode(topframefb,2,trtopframeback);

  Int_t colorTopFrame = 12;
  topframeside -> SetLineColor(colorTopFrame);
  topframeside -> SetLineColor(colorTopFrame);
  topframefb   -> SetLineColor(colorTopFrame);
  topframefb   -> SetLineColor(colorTopFrame);

  //Making Lexan part (currently called Aluminum)
  Double_t topframefblexanx=126.6;
  Double_t topframefblexany=1.905;
  Double_t topframefblexanz=2.0;
  Double_t topframesidelexanx=2.0;
  Double_t topframesidelexany=1.905;
  Double_t topframesidelexanz=159.7;
  TGeoVolume *topframefblexan = gGeoManager->MakeBox("topframefblexan",aluminium,topframefblexanx/2.0,topframefblexany/2.0,topframefblexanz/2.0);
  TGeoVolume *topframesidelexan = gGeoManager->MakeBox("topframesidelexan",aluminium,topframesidelexanx/2.0,topframesidelexany/2.0,topframesidelexanz/2.0);
  Double_t trtopframesidelexanx=topframefblexanx/2.0+topframesidelexanx/2.0;
  Double_t trtopframesidelexany=yActiveC/2.0+topframesidelexany/2.0+0.8763; //where does the last part come from???
  TGeoTranslation *trtopframeleftlexan = new TGeoTranslation(trtopframesidelexanx,trtopframesidelexany,0.0);
  TGeoTranslation *trtopframerightlexan = new TGeoTranslation(-trtopframesidelexanx,trtopframesidelexany,0.0);
  Double_t trtopframefblexanz=topframesidelexanz/2.0-topframefblexanz/2.0;
  TGeoTranslation *trtopframefrontlexan = new TGeoTranslation(0.0,trtopframesidelexany,trtopframefblexanz);
  TGeoTranslation *trtopframebacklexan = new TGeoTranslation(0.0,trtopframesidelexany,-trtopframefblexanz);
  topFrame->AddNode(topframesidelexan,1,trtopframeleftlexan);
  topFrame->AddNode(topframesidelexan,2,trtopframerightlexan);
  topFrame->AddNode(topframefblexan,1,trtopframefrontlexan);
  topFrame->AddNode(topframefblexan,2,trtopframebacklexan);

  Int_t colorTopLexan = 12;
  topframesidelexan -> SetLineColor(colorTopLexan);
  topframesidelexan -> SetLineColor(colorTopLexan);
  topframefblexan -> SetLineColor(colorTopLexan);
  topframefblexan -> SetLineColor(colorTopLexan);

  // ----------------------------------------------------
  //  Rib (from Mike's macro)
  // ----------------------------------------------------
  //TGeoVolume *ribmain = gGeoManager->MakeBox("ribmain",vacuum,200.,200.,200.); // TODO : find appropriate name
  TGeoVolumeAssembly *ribmain = gGeoManager->MakeVolumeAssembly("ribmain");
  ribmain -> SetMedium(vacuum);
  ribmain->SetVisibility(kFALSE);
  Double_t ribx=152.4;
  Double_t riby=10.9042;
  Double_t ribz=206.06;
  TGeoVolume *rib = gGeoManager->MakeBox("rib",aluminium,ribx/2.0,riby/2.0,ribz/2.0);
  //Adding some lips to close area
  Double_t riblipsidex=10.74125;
  Double_t riblipsidey=3.01498;
  Double_t riblipsidez=206.06;
  TGeoVolume *riblipside = gGeoManager->MakeBox("riblipside",aluminium,riblipsidex/2.0,riblipsidey/2.0,riblipsidez/2.0);

  //Squeak still needs to finish the rib assembly
  TGeoTranslation *trriblipsideleft = new TGeoTranslation(70.829375,-6.95959,0.0);
  TGeoTranslation *trriblipsideright = new TGeoTranslation(-70.829375,-6.95959,0.0);
  TGeoVolume *riblipfb = gGeoManager->MakeBox("riblipfb",aluminium,130.9175/2.0,3.01498/2.0,23.02125/2.0);
  TGeoTranslation *trriblipfbfront = new TGeoTranslation(0.0,-6.95959,91.51938);
  TGeoTranslation *trriblipfbback = new TGeoTranslation(0.0,-6.95959,-91.51938);

  Int_t colorRib = 18;
  rib        -> SetLineColor(colorRib);
  riblipside -> SetLineColor(colorRib);
  riblipfb   -> SetLineColor(colorRib);

  ribmain->AddNode(rib,1);
  //ribmain->AddNode(riblipside,1,trriblipsideleft);
  //ribmain->AddNode(riblipside,2,trriblipsideright);
  //ribmain->AddNode(riblipfb,1,trriblipfbfront);
  //ribmain->AddNode(riblipfb,1,trriblipfbback);
  //TGeoTranslation *trrib = new TGeoTranslation(0.0,33.26708,0.0);
  TGeoTranslation *trrib = new TGeoTranslation(0.0,yActiveC/2+riby/2,0.0);

  // ----------------------------------------------------
  //  Wire Plane (from Mike's macro)
  // ----------------------------------------------------
  //TGeoVolume *wirePlane = gGeoManager->MakeBox("wirePlane",vacuum,200.,200.,200.);
  TGeoVolumeAssembly *wirePlane = gGeoManager->MakeVolumeAssembly("wirePlane");
  wirePlane -> SetMedium(vacuum);
  wirePlane->SetVisibility(kFALSE);
  //Outer Bars
  Double_t wpoutbarx=2.0;
  Double_t wpoutbary=1.4;
  Double_t wpoutbarz=145.6;
  TGeoVolume *wpoutbar = gGeoManager->MakeBox("wpoutbar",aluminium,wpoutbarx/2.0,wpoutbary/2.0,wpoutbarz/2.0);
  //The spacing variables are measured from inside edge to inside edge
  Double_t wpoutspacing=118.32;
  Double_t wpmidspacing=113.52;
  Double_t wpinspacing=103.17;
  Double_t trwpoutbarx=wpoutspacing/2.0+wpoutbarx/2.0;
  TGeoTranslation *trwpoutbarleft = new TGeoTranslation(trwpoutbarx,0.0,0.0);
  TGeoTranslation *trwpoutbarright = new TGeoTranslation(-trwpoutbarx,0.0,0.0);
  wirePlane->AddNode(wpoutbar,1,trwpoutbarleft);
  wirePlane->AddNode(wpoutbar,1,trwpoutbarright);
  wpoutbar->SetLineColor(kBlue);
  //Middle Bars
  Double_t wpmidbarx=2.0;
  Double_t wpmidbary=0.8;
  Double_t wpmidbarz=145.6;
  TGeoVolume *wpmidbar = gGeoManager->MakeBox("wpmidbar",aluminium,wpmidbarx/2.0,wpmidbary/2.0,wpmidbarz/2.0);
  Double_t trwpmidbarx=wpmidspacing/2.0+wpmidbarx/2.0;
  Double_t trwpmidbary=wpoutbary/2.0-wpmidbary/2.0;
  TGeoTranslation *trwpmidbarleft = new TGeoTranslation(trwpmidbarx,trwpmidbary,0.0);
  TGeoTranslation *trwpmidbarright = new TGeoTranslation(-trwpmidbarx,trwpmidbary,0.0);
  wirePlane->AddNode(wpmidbar,1,trwpmidbarleft);
  wirePlane->AddNode(wpmidbar,1,trwpmidbarright);
  wpmidbar->SetLineColor(kBlue);
  //Inner Bars
  Double_t wpinbarx=2.0;
  Double_t wpinbary=0.4;
  Double_t wpinbarz=145.6;
  TGeoVolume *wpinbar = gGeoManager->MakeBox("wpinbar",aluminium,wpinbarx/2.0,wpinbary/2.0,wpinbarz/2.0);
  Double_t trwpinbarx=wpinspacing/2.0+wpinbarx/2.0;
  Double_t trwpinbary=wpoutbary/2.0-wpinbary/2.0;
  TGeoTranslation *trwpinbarleft = new TGeoTranslation(trwpinbarx,trwpinbary,0.0);
  TGeoTranslation *trwpinbarright = new TGeoTranslation(-trwpinbarx,trwpinbary,0.0);
  wirePlane->AddNode(wpinbar,1,trwpinbarleft);
  wirePlane->AddNode(wpinbar,1,trwpinbarright);
  wpinbar->SetLineColor(kBlue);
  //Wires
  //Squeak is down to here
  TGeoRotation *rowire = new TGeoRotation("rocorner",90.0,90.0,0.0);
  TGeoVolume *wpoutwire = gGeoManager->MakeTube("wpoutwire",copper,0.0,0.00375,122.32/2.0);
  TGeoVolume *wpmidwire = gGeoManager->MakeTube("wpmidwire",copper,0.0,0.00375,117.52/2.0);
  TGeoVolume *wpinwire = gGeoManager->MakeTube("wpinwire",copper,0.0,0.001,107.17/2.0);

  Int_t colorWireBar = 12;
  wpoutbar  -> SetLineColor(colorWireBar);
  wpmidbar  -> SetLineColor(colorWireBar);
  wpinbar   -> SetLineColor(colorWireBar);
  Int_t colorWire= kYellow-9;
  wpoutwire -> SetLineColor(colorWire);
  wpmidwire -> SetLineColor(colorWire);
  wpinwire  -> SetLineColor(colorWire);

  TGeoCombiTrans *cowireout[1455];
  TGeoCombiTrans *cowiremid[1455];
  TGeoCombiTrans *cowirein[363];

  for (Int_t i=0; i<1455; i++) {
    cowireout[i] = new TGeoCombiTrans("cowireout",0.0,-0.7127,72.75-0.1*i,rowire);
    cowiremid[i] = new TGeoCombiTrans("cowiremid",0.0,-0.1127,72.75-0.1*i,rowire);
    //wirePlane->AddNode(wpoutwire,i+1,cowireout[i]);
    //wirePlane->AddNode(wpmidwire,i+1,cowiremid[i]);
  }
  for (Int_t i=0; i<363; i++) {
    cowirein[i] = new TGeoCombiTrans("cowirein",0.0,0.2873,72.6-0.4*i,rowire);
    //wirePlane->AddNode(wpinwire,i+1,cowirein[i]);
  }
  TGeoTranslation *trwireplane = new TGeoTranslation(0.0,26.9575,4.804);

  // ----------------------------------------------------
  //  Pad Plane (from Mike's macro)
  // ----------------------------------------------------
  //Make Pad Plane (Setting to be .062 inches (.15748 cm) thick
  //TGeoVolume *padPlane = gGeoManager->MakeBox("padPlane",vacuum,200.,200.,200.);
  TGeoVolumeAssembly *padPlane = gGeoManager->MakeVolumeAssembly("padPlane");
  padPlane -> SetMedium(vacuum);
  padPlane->SetVisibility(kFALSE);
  Double_t padplanepcbx=86.6;
  Double_t padplanepcby=0.254;
  Double_t padplanepcbz=134.4;
  TGeoVolume *padplaneplane = gGeoManager->MakeBox("padplaneplane",pcb,padplanepcbx/2.0,padplanepcby/2.0,padplanepcbz/2.0);
  Int_t colorPad = kRed+4;
  padplaneplane -> SetLineColor(colorPad);
  padPlane->AddNode(padplaneplane,1);
  Double_t padx=86.6;
  Double_t pady=0.01;
  Double_t padz=134.4;
  TGeoVolume *pad = gGeoManager->MakeBox("pad",copper,padx/2.0,pady/2.0,padz/2.0);
  Double_t trpady=padplanepcby/2.0+pady/2.0;
  TGeoTranslation *trpad = new TGeoTranslation(0.0,trpady,0.0);
  //padPlane->AddNode(pad,1,trpad);

  /* TGeoVolume *pad = gGeoManager->MakeBox("pad",copper,0.8/2.0,0.01/2.0,1.2/2.0); */
  /* pad->SetLineColor(kBlue); */
  /* padplaneplane->SetLineColor(kMagenta); */
  /* Int_t padrun=1; */
  /* TGeoTranslation *trpad[108][112]; */
  /* for (Int_t i=0; i<108; i++) { */
  /*   for (Int_t j=0; j<112; j++) { */
  /*     trpad[i][j] = new TGeoTranslation(42.8-0.8*i,-0.08374,66.6-1.2*j); */
  /*     padPlane->AddNode(pad,padrun,trpad[i][j]); */
  /*     padrun++; */
  /*   } */
  /* } */
  TGeoTranslation *trpadplane = new TGeoTranslation(0.0,27.5305,5.08548); //need to verify z shift

  // ----------------------------------------------------
  //  Addition to Active Volume (from Mike's macro)
  // ----------------------------------------------------
  /*
  TGeoVolume *actup1 = gGeoManager->MakeBox("actup1",p10,95.8523/2.0,0.8763/2.0,143.8987/2.0);
  TGeoTranslation *tractup1 = new TGeoTranslation(0.0,25.23815,0.0);
  TGeoVolume *actup2 = gGeoManager->MakeBox("actup2",p10,126.6/2.0,1.905/2.0,154.7/2.0);
  TGeoTranslation *tractup2 = new TGeoTranslation(0.0,26.6288,0.0);
  active->AddNode(actup1,1,tractup1);
  active->AddNode(actup2,1,tractup2);
  */













  // ----------------------------------------------------
  //  AddNode to TOP
  // ----------------------------------------------------
  top -> AddNode(tpc,1,combiTPC);
 
  // ----------------------------------------------------
  //  AddNode to TPC
  // ----------------------------------------------------
  tpc -> AddNode(active,1);
  tpc -> AddNode(cageFront,1);
  tpc -> AddNode(cageSide,1);
  tpc -> AddNode(cageCorner,1);
  tpc -> AddNode(frontWindow,1,trfwindow);
  tpc -> AddNode(frontWindowFrame,1);
  //tpc -> AddNode(copperStrips,1);
  tpc -> AddNode(frontWindowCradle,1);
  tpc -> AddNode(bottomPlate,1);
  tpc -> AddNode(backWindowFrame,1,trbackwindowframe);
  tpc -> AddNode(backWindow,1,trbackwindow);
  tpc -> AddNode(topFrame,1);
  tpc -> AddNode(ribmain,1,trrib);
  tpc -> AddNode(wirePlane,1,trwireplane);
  //tpc -> AddNode(padPlane,1,trpadplane);

  // ----------------------------------------------------
  //  Visual Attributes 
  // ----------------------------------------------------
  Int_t transparency = 70;
  Int_t transparencyActive = 70 + transparency; if(transparencyActive>100) transparencyActive = 100;
  Int_t transparencyWindow = 50 + transparency; if(transparencyWindow>100) transparencyWindow = 100;
  Int_t transparencyWire   = 50 + transparency; if(transparencyWire  >100) transparencyWire   = 100;

  active            -> SetVisibility(kFALSE);
  cageFront         -> SetTransparency(transparency);
  cageSide          -> SetTransparency(transparency);
  cageCorner        -> SetTransparency(transparency);
  frontWindow       -> SetTransparency(transparencyWindow);
  frontWindowFrame  -> SetTransparency(transparency);
  copperStrips      -> SetTransparency(transparency);
  frontWindowCradle -> SetTransparency(transparency);
  bottomPlate       -> SetTransparency(transparency);
  backWindowFrame   -> SetTransparency(transparency);
  backWindow        -> SetTransparency(transparencyWindow);
  topFrame          -> SetTransparency(transparency);
  ribmain           -> SetTransparency(transparency);
  wirePlane         -> SetTransparency(transparencyWire);
  padPlane          -> SetTransparency(transparency);

  active            -> SetLineColor(kBlue-10);
  //cageFront         -> SetLineColor(kOrange-7);
  cageFront         -> SetLineColor(kBlue-4);
  //cageFront         -> SetLineColor(kOrange-4);
  cageSide          -> SetLineColor(18);
  cageCorner        -> SetLineColor(18);
  frontWindow       -> SetLineColor(18);
  frontWindowFrame  -> SetLineColor(0);
  copperStrips      -> SetLineColor(colorStrip);
  frontWindowCradle -> SetLineColor(colorCradle);
  bottomPlate       -> SetLineColor(colorBottomPlate);
  backWindowFrame   -> SetLineColor(colorBackWindow);
  backWindow        -> SetLineColor(12);
  topFrame          -> SetLineColor(colorTopFrame);
  ribmain           -> SetLineColor(colorRib);
  wirePlane         -> SetLineColor(colorWire);
  padPlane          -> SetLineColor(colorPad);

  // ----------------------------------------------------
  //  End of Building Geometry
  // ----------------------------------------------------
  gGeoManager -> CloseGeometry();
  top->Draw("ogl");

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
