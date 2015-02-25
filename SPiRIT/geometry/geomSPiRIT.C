#include "TGeoManager.h"
#include "TGeoMedium.h"
#include "TGeoMaterial.h"
#include "TGeoVolume.h"
//#include "TGeoVolumeAssembly.h"
#include <vector>

/**
 * Name begin with "s" stands for "size of"
 * Name begin with "r" stands for "radius of"
 */

void geomSPiRIT()
{
  new TGeoManager("SPiRIT","The SPiRIT Geometry");

  // ----------------------------------------------------
  //  Dimensions (cm)
  // ----------------------------------------------------

  /* 
   * Size of gas volume inside the field cage. 
   * Thickness of cage side(sCageWall) is not included.
   * height(y) is from cathode to pad-plane bottom 
   */ 
  TVector3 sActive(96.61, 51.01, 144.64);
  Double_t rCorner = 2.8495; // radius of active corner
  //TVector3 sPadPlane(86.4, 0.1, 134.4); // TODO : check height 
  //Double_t sCageWall = 0.1574;

  // ----------------------------------------------------
  //  Mediums
  // ----------------------------------------------------
  /*
  TGeoMedium *vacuum    = new TGeoMedium("vacuum"   ,1, new TGeoMaterial("vacuum"));
  TGeoMedium *p10       = new TGeoMedium("p10"      ,1, new TGeoMaterial("p10"));
  TGeoMedium *kapton    = new TGeoMedium("kapton"   ,1, new TGeoMaterial("kapton"));
  TGeoMedium *pcb       = new TGeoMedium("pcb"      ,1, new TGeoMaterial("pcb"));
  TGeoMedium *aluminium = new TGeoMedium("aluminium",1, new TGeoMaterial("aluminium"));
  TGeoMedium *copper    = new TGeoMedium("copper"   ,1, new TGeoMaterial("copper"));
  */

  TGeoMaterial *mat_vacuum = new TGeoMaterial("vacuum",0,0,0);
  TGeoMaterial *mat_p10 = new TGeoMaterial("p10",0,0,0);
  TGeoMaterial *mat_kapton = new TGeoMaterial("kapton",0,0,0);
  TGeoMaterial *mat_pcb = new TGeoMaterial("pcb",0,0,0);
  TGeoMaterial *mat_aluminium = new TGeoMaterial("aluminium",0,0,0);
  TGeoMaterial *mat_copper = new TGeoMaterial("copper",0,0,0);

  TGeoMedium *vacuum    = new TGeoMedium("vacuum"   ,1, mat_vacuum);
  TGeoMedium *p10       = new TGeoMedium("p10"      ,1, mat_p10);
  TGeoMedium *kapton    = new TGeoMedium("kapton"   ,1, mat_kapton);
  TGeoMedium *pcb       = new TGeoMedium("pcb"      ,1, mat_pcb);
  TGeoMedium *aluminium = new TGeoMedium("aluminium",1, mat_aluminium);
  TGeoMedium *copper    = new TGeoMedium("copper"   ,1, mat_copper);

  // ----------------------------------------------------
  //  Volumes
  // ----------------------------------------------------
  TGeoVolumeAssembly* top    = gGeoManager -> MakeVolumeAssembly("top");
  TGeoVolumeAssembly* tpc    = gGeoManager -> MakeVolumeAssembly("tpc");
  //TGeoVolumeAssembly* active = gGeoManager -> MakeVolumeAssembly("field_cage_in"); // active area (gas volume)
  //TGeoVolumeAssembly* cage   = gGeoManager -> MakeVolumeAssembly("cage");   // field cage

  TGeoVolume* active = gGeoManager -> MakeBox("field_cage_in",p10,200,200,200); // active area (gas volume)

  /** Active volume **/
  std::vector<TGeoVolume*> activeArray;

  TGeoVolume* activeCenter  = 0;
  TGeoVolume* activeTop     = 0;
  TGeoVolume* activeBottom  = 0;
  TGeoVolume* activeRight   = 0;
  TGeoVolume* activeLeft    = 0;
  TGeoVolume* activeCorner1 = 0;
  TGeoVolume* activeCorner2 = 0;
  TGeoVolume* activeCorner3 = 0;
  TGeoVolume* activeCorner4 = 0;

  activeCenter  = gGeoManager -> MakeBox("ActiveCenter",   p10, sActive.X()/2, sActive.Y()/2, sActive.Z()/2);
  activeTop     = gGeoManager -> MakeBox("ActiveTop",      p10, sActive.X()/2, sActive.Y()/2, rCorner/2);
  activeRight   = gGeoManager -> MakeBox("ActiveRight",    p10, rCorner/2,     sActive.Y()/2, sActive.Z()/2);
  activeCorner1 = gGeoManager -> MakeTubs("ActiveCorner1", p10, 0., rCorner, sActive.Y()/2, 0,   90.);
  activeCorner2 = gGeoManager -> MakeTubs("ActiveCorner2", p10, 0., rCorner, sActive.Y()/2, 90., 180.);
  activeCorner3 = gGeoManager -> MakeTubs("ActiveCorner3", p10, 0., rCorner, sActive.Y()/2, 180.,270.);
  activeCorner4 = gGeoManager -> MakeTubs("ActiveCorner4", p10, 0., rCorner, sActive.Y()/2, 270.,360.);
  activeBottom  = activeTop; activeBottom -> SetName("ActiveBottom");
  activeLeft    = activeRight; activeLeft -> SetName("ActiveLeft");

  activeArray.push_back(activeCenter);
  activeArray.push_back(activeTop);
  activeArray.push_back(activeBottom);
  activeArray.push_back(activeRight);
  activeArray.push_back(activeLeft);
  activeArray.push_back(activeCorner1);
  activeArray.push_back(activeCorner2);
  activeArray.push_back(activeCorner3);
  activeArray.push_back(activeCorner4);

  Double_t offXActive = sActive.X()/2+rCorner/2;
  Double_t offZActive = sActive.Z()/2+rCorner/2;

  TGeoTranslation* transATop    = new TGeoTranslation(0,0, offZActive);
  TGeoTranslation* transABottom = new TGeoTranslation(0,0,-offZActive);
  TGeoTranslation* transARight  = new TGeoTranslation( offXActive,0,0);
  TGeoTranslation* transALeft   = new TGeoTranslation(-offXActive,0,0);
  TGeoRotation*    rotCorner    = new TGeoRotation("rotCorner",0,90,0);
  TGeoCombiTrans*  combiCorner1 = new TGeoCombiTrans("combmiCorner1", sActive.X()/2,0, sActive.Z()/2,rotCorner);
  TGeoCombiTrans*  combiCorner2 = new TGeoCombiTrans("combmiCorner2",-sActive.X()/2,0, sActive.Z()/2,rotCorner);
  TGeoCombiTrans*  combiCorner3 = new TGeoCombiTrans("combmiCorner3",-sActive.X()/2,0,-sActive.Z()/2,rotCorner);
  TGeoCombiTrans*  combiCorner4 = new TGeoCombiTrans("combmiCorner4", sActive.X()/2,0,-sActive.Z()/2,rotCorner);

  active -> AddNode(activeCenter, 1);
  active -> AddNode(activeTop,    1, transATop);
  active -> AddNode(activeBottom, 1, transABottom);
  active -> AddNode(activeRight,  1, transARight);
  active -> AddNode(activeLeft,   1, transALeft);
  active -> AddNode(activeCorner1,1, combiCorner1);
  active -> AddNode(activeCorner2,1, combiCorner2);
  active -> AddNode(activeCorner3,1, combiCorner3);
  active -> AddNode(activeCorner4,1, combiCorner4);

  /*
  const Int_t nActives = 9;
  = { activeCenter,
      activeTop,
      activeBottom,
      activeRight,
      activeLeft,
      activeCorner1,
      activeCorner2,
      activeCorner3,
      activeCorner4 };
      */

  Int_t nActives = activeArray.size();
  for(Int_t iActive=0; iActive<nActives; iActive++)
  {
    activeArray[iActive] -> SetLineColor(kBlue-9);
  }



  gGeoManager->SetTopVolume(top);

  top -> AddNode(tpc,1);
  tpc -> AddNode(active,1);

  top->Draw("ogl");

  TFile *outfile = new TFile("geomSPiRIT.root","recreate"); 
  top->Write(); 
  outfile->Close(); 

  /*
  TGeoVolume *cage_volume      = 0; // gas volume
  TGeoVolume *cage_side_right  = 0; // right  side (normal to x-axis) seen from beam direction
  TGeoVolume *cage_side_left   = 0; // left   side (normal to x-axis) seen from beam direction
  TGeoVolume *cage_side_front  = 0; // front  side (normal to z-axis)
  TGeoVolume *cage_side_back   = 0; // back   side (normal to z-axis)
  TGeoVolume *cage_side_top    = 0; // top    side (normal to x-axis)
  TGeoVolume *cage_side_bottom = 0; // bottom side (normal to x-axis)

  TGeoVolume *padp = 0; // Pad Plane




  cage_volume       = gGeoManager -> MakeBox("FieldCage", p10, 
                                             sActive.X()/2, 
                                             sActive.Y()/2, 
                                             sActive.Z()/2);
  cage_side_right   = gGeoManager -> MakeBox("cage_side_right",pcb, // TODO : check material
                                             sCageWall/2, 
                                             sActive.Y()/2, 
                                             sActive.Z()/2);
  cage_side_front   = gGeoManager -> MakeBox("cage_side_front",pcb,
                                             
                                             

  cage_side_left    = cage_side_right; 
  cage_side_left -> SetName("cage_side_right");


  TGeoVolume *cageside = geom->MakeBox("cageside",pcbmvd,cagesidex/2.,cagesidey/2.,cagesidez/2.);






  padp = gGeoManager -> MakeBox("PadPlane",  pcb, sPadPlane.X()/2, sPadPlane.Y()/2, sPadPlane.Z()/2);
  */









/*

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
  */
 
//  return field_cage_in;
}
