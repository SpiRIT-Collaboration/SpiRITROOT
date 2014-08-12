#include <TMath.h>

//Currently replaced methane with Air
//Currently replaced Kapton with Copper

void smartgeometry()() {
  gStyle->SetCanvasPreferGL(true);
  gSystem->Load("libGeom");
  TGeoManager *geom = new TGeoManager("SPiRIT1","SPiRIT1");

  //defining materials
  //densities are in g/cm^3
  TGeoElementTable *eletable = gGeoManager->GetElementTable();
  TGeoElement *H = eletable->GetElement(1);
  TGeoElement *C = eletable->GetElement(6);
  TGeoElement *N = eletable->GetElement(7);
  TGeoElement *O = eletable->GetElement(8);
  TGeoElement *Si = eletable->GetElement(14);
  TGeoElement *Br = eletable->GetElement(35);
  TGeoMaterial *matvacuum = new TGeoMaterial("vacuum",0,0,0);
  TGeoMedium *vacuum = new TGeoMedium("vacuum",1,matvacuum);
  TGeoMixture *mixp10 = new TGeoMixture("p10",2,0.00066);
  mixp10->DefineElement(0,6,1);
  mixp10->DefineElement(1,1,4);
  TGeoMedium *p10 = new TGeoMedium("p10",1,mixp10);
  TGeoMixture *mixkapton = new TGeoMixture("kapton",4,1.42);
  mixkapton->DefineElement(0,H,0.026362);
  mixkapton->DefineElement(1,C,0.691133);
  mixkapton->DefineElement(2,N,0.073270);
  mixkapton->DefineElement(3,O,0.209235);
  TGeoMedium *kapton = new TGeoMedium("kapton",1,mixkapton);
  TGeoMixture *mixpcbmvd = new TGeoMixture("pcbmvd",5,1.7);
  mixpcbmvd->DefineElement(0,Si,0.180774);
  mixpcbmvd->DefineElement(1,O,0.405633);
  mixpcbmvd->DefineElement(2,C,0.278042);
  mixpcbmvd->DefineElement(3,H,0.0684428);
  mixpcbmvd->DefineElement(4,Br,0.671091);
  TGeoMedium *pcbmvd = new TGeoMedium("pcbmvd",1,mixpcbmvd);
  TGeoMaterial *matAluminum = new TGeoMaterial("Aluminum",26.98,13,2.7);
  TGeoMedium *Aluminum = new TGeoMedium("Aluminum",2,matAluminum);
  TGeoMaterial *matcopper = new TGeoMaterial("copper",63.546,29,8.96);
  TGeoMedium *copper = new TGeoMedium("copper",3,matcopper);
  
  //Make top container
  TGeoVolume *top = geom->MakeBox("top",vacuum,10000.,10000.,10000.);
  geom->SetTopVolume(top);

  //Make whole container
  //This is necessary to get the geometry to read in for some reason
  TGeoVolume *whole = geom->MakeBox("whole",vacuum,10000.,10000.,10000.);

  //Define the main active area (currently p10)
  //Along with the field cage this will be one of the main considerations for calculating translations
  TGeoVolume *active = geom->MakeBox("Active",vacuum,200.,200.,200.);
  Double_t actcenterx=90.96;
  Double_t actcentery=49.6;
  Double_t actcenterz=138.96;
  //Currently define the center of the main active area to be the center/origin
  //A rotation and translation will come later
  //The active area is defined by 5 rectangular regions and
  //4 rounded corners. Each corner is 1/4 of a cylinder
  //The actradius is the radius of this cylinder
  TGeoVolume *actcenter = geom->MakeBox("actcenter",p10,actcenterx/2.,actcentery/2.,actcenterz/2.);
  Double_t actradius=2.8495;
  Double_t actfrontx=actcenterx;
  Double_t actfronty=actcentery;
  Double_t actfrontz=actradius;
  TGeoVolume *actfront = geom->MakeBox("actfront",p10,actfrontx/2.,actfronty/2.,actradius/2.);
  Double_t tractfrontz=actfrontz/2.+actcenterz/2.;
  TGeoTranslation *tractfront = new TGeoTranslation(0.0,0.0,tractfrontz);
  TGeoTranslation *tractback = new TGeoTranslation(0.0,0.0,-tractfrontz);
  Double_t actsidex=actradius;
  Double_t actsidey=actcentery;
  Double_t actsidez=actcenterz;
  TGeoVolume *actside = geom->MakeBox("actside",p10,actsidex/2.,actsidey/2.,actsidez/2.);
  Double_t tractsidex=actcenterx/2.+actsidex/2.;
  TGeoTranslation *tractsidel = new TGeoTranslation(tractsidex,0.0,0.0);
  TGeoTranslation *tractsider = new TGeoTranslation(-tractsidex,0.0,0.0);
  TGeoVolume *actcorner1 = geom->MakeTubs("actcorner1",p10,0.0,actradius,actcentery/2.0,0.0,90.0);
  TGeoVolume *actcorner2 = geom->MakeTubs("actcorner2",p10,0.0,actradius,actcentery/2.0,90.0,180.0);
  TGeoVolume *actcorner3 = geom->MakeTubs("actcorner3",p10,0.0,actradius,actcentery/2.0,180.0,270.0);
  TGeoVolume *actcorner4 = geom->MakeTubs("actcorner4",p10,0.0,actradius,actcentery/2.0,270.0,360.0);
  Double_t trcornerx=actcenterx/2.;
  Double_t trcornerz=actcenterz/2.;
  TGeoRotation *rocorner = new TGeoRotation("rocorner",0.0,90.0,0.0);
  TGeoCombiTrans *cocorner1 = new TGeoCombiTrans("cocorner1",trcornerx,0.0,trcornerz,rocorner);
  TGeoCombiTrans *cocorner2 = new TGeoCombiTrans("cocorner2",-trcornerx,0.0,trcornerz,rocorner);
  TGeoCombiTrans *cocorner3 = new TGeoCombiTrans("cocorner3",-trcornerx,0.0,-trcornerz,rocorner);
  TGeoCombiTrans *cocorner4 = new TGeoCombiTrans("cocorner4",trcornerx,0.0,-trcornerz,rocorner);
  active->AddNode(actcenter,1);
  active->AddNode(actfront,1,tractfront);
  active->AddNode(actfront,2,tractback);
  active->AddNode(actside,1,tractsidel);
  active->AddNode(actside,2,tractsider);
  active->AddNode(actcorner1,1,cocorner1);
  active->AddNode(actcorner2,2,cocorner2);
  active->AddNode(actcorner3,3,cocorner3);
  active->AddNode(actcorner4,4,cocorner4);

  //Make the 4 corners (approximate) 2.8495
  Double_t coradius=3.007;
  TGeoVolume *corners = geom->MakeBox("corners",vacuum,200.,200.,200.);
  TGeoVolume *corner1 = geom->MakeTubs("corner1",Aluminum,actradius,coradius,actcentery/2.0,0.0,90.0);
  TGeoVolume *corner2 = geom->MakeTubs("corner2",Aluminum,actradius,coradius,actcentery/2.0,90.0,180.0);
  TGeoVolume *corner3 = geom->MakeTubs("corner3",Aluminum,actradius,coradius,actcentery/2.0,180.0,270.0);
  TGeoVolume *corner4 = geom->MakeTubs("corner4",Aluminum,actradius,coradius,actcentery/2.0,270.0,360.0);
  corners->AddNode(corner1,1,cocorner1);
  corners->AddNode(corner2,1,cocorner2);
  corners->AddNode(corner3,1,cocorner3);
  corners->AddNode(corner4,1,cocorner4);

  //Make the three walls of the field cage
  //This will be one of the primary ways that I build all of the other parts.
  //Many parts will reference these sizes.
  TGeoVolume *cage = geom->MakeBox("cage",vacuum,200.,200.,200.);
  //Cageside
  Double_t cagesidex=0.1575;
  Double_t cagesidey=actcentery;
  Double_t cagesidez=actcenterz;
  TGeoVolume *cageside = geom->MakeBox("cageside",pcbmvd,cagesidex/2.,cagesidey/2.,cagesidez/2.);
  Double_t trcagesidex=cagesidex/2.+actcenterx/2.+actsidex;
  TGeoTranslation *trcageright = new TGeoTranslation(trcagesidex,0.0,0.0);
  TGeoTranslation *trcageleft = new TGeoTranslation(trcagesidex,0.0,0.0);
  Double_t cagefrontsidex=40.48;
  Double_t cagefrontsidey=actcentery;
  Double_t cagefrontz=0.1575;
  Double_t cagefronttopx=actcenterx-2.0*cagefrontsidex;
  Double_t cagefronttopy=13.0;
  Double_t cagefrontbotx=cagefronttopx;
  Double_t cagefrontboty=19.6;
  Double_t trcagefrontsidex=cagefronttopx/2.+cagefrontsidex/2.;
  Double_t trcagefrontz=actcenterz/2.+actfrontz+cagefrontz/2.;
  Double_t trcagefronttopy=cagefrontsidey/2.-cagefronttopy/2.;
  Double_t trcagefrontboty=-cagesidey/2.+cagefrontboty/2.;
  TGeoVolume *cagefrontside = geom->MakeBox("cagefrontside",pcbmvd,cagefrontsidex/2.,cagefrontsidey/2.,cagefrontz/2.);
  TGeoTranslation *trcagefrontleft = new TGeoTranslation(trcagefrontsidex,0.0,trcagefrontz);
  TGeoTranslation *trcagefrontright = new TGeoTranslation(-trcagefrontsidex,0.0,trcagefrontz);
  TGeoVolume *cagefronttop = geom->MakeBox("cagefronttop",pcbmvd,cagefronttopx/2.,cagefronttopy/2.,cagefrontz/2.);
  TGeoTranslation *trcagefronttop = new TGeoTranslation(0.0,trcagefronttopy,trcagefrontz);
  TGeoVolume *cagefrontbottom = geom->MakeBox("cagefrontbottom",pcbmvd,cagefrontbotx/2.,cagefrontboty/2.,cagefrontz/2.);
  TGeoTranslation *trcagefrontbottom = new TGeoTranslation(0.0,trcagefrontboty,trcagefrontz);
  cage->AddNode(cageside,1,trcageright);
  cage->AddNode(cageside,2,trcageleft);
  cage->AddNode(cagefrontside,1,trcagefrontleft);
  cage->AddNode(cagefrontside,2,trcagefrontright);
  cage->AddNode(cagefronttop,1,trcagefronttop);
  cage->AddNode(cagefrontbottom,1,trcagefrontbottom);

  //Make front window inner frame and front window...kind of at the same time
  Double_t fwindowz=1.111;
  Double_t fwindowy=7.0;
  Double_t fwindowx1=5.73;
  Double_t fwindowx2=7.0;
  TGeoVolume *fwindow = geom->MakeTrd1("fwindow",kapton,fwindowx1/2.,fwindowx2/2.,fwindowy/2.0,fwindowz/2.0);

  TGeoVolume *fwinframe = geom->MakeBox("fwinframe",vacuum,200.,200.,200.);

  Double_t fwinframesidex=1.45;
  Double_t fwinframesidey=16.9;
  Double_t fwinframez=1.111;
  TGeoVolume *fwinframeside = geom->MakeBox("fwinframeside",Aluminum,fwinframesidex/2.,fwinframesidey/2.,fwinframez/2.);
  Double_t fwinframetopx=7.0;
  Double_t fwinframetopy=1.45;
  TGeoVolume *fwinframet = geom->MakeBox("fwinframet",Aluminum,fwinframetopx/2.,fwinframetopy/2.,fwinframez/2.);
  Double_t fwinframebotx=7.0;
  Double_t fwinframeboty=8.45;
  TGeoVolume *fwinframeb = geom->MakeBox("fwinframeb",Aluminum,fwinframebotx/2.,fwinframeboty/2.,fwinframez/2.);
  Double_t trfwinframesidex=fwinframesidex/2.+fwinframetopx/2.;
  Double_t trfwinframesidey=trcagefronttopy-cagefronttopy/2.0-0.05-fwinframesidey/2.;
  Double_t trfwinframetopy=trcagefronttopy-cagefronttopy/2.-0.05-fwinframetopy/2.;
  Double_t trfwinframeboty=trcagefronttopy-cagefronttopy/2.-0.05-fwinframetopy-fwindowy-fwinframeboty/2.;
  Double_t trfwindowy=trcagefronttopy-cagefronttopy/2.-0.05-fwinframetopy-fwindowy/2.;
  Double_t trfwindowz=actcenterz/2.+actfrontz+fwindowz/2.;
  Double_t trfwinframez=trfwindowz;
  TGeoTranslation *trfwinframel = new TGeoTranslation(trfwinframesidex,trfwinframesidey,0.0);
  TGeoTranslation *trfwinframer = new TGeoTranslation(-trfwinframesidex,trfwinframesidey,0.0);
  TGeoTranslation *trfwinframet = new TGeoTranslation(0.0,trfwinframetopy,0.0);
  TGeoTranslation *trfwinframeb = new TGeoTranslation(0.0,trfwinframeboty,0.0);
  Double_t trapbase=0.635;
  Double_t traptheta=TMath::RadToDeg()*TMath::ATan((trapbase/2.0)/fwinframez);
  TGeoVolume *fwintrapl = geom->MakeTrap("fwintrapl",Aluminum,fwinframez/2.0,-traptheta,0.0,fwindowy/2.0,trapbase/2.0,trapbase/2.0,0.0,fwindowy/2.0,0.0,0.0,0.0);
  TGeoVolume *fwintrapr = geom->MakeTrap("fwintrapr",Aluminum,fwinframez/2.0,traptheta,0.0,fwindowy/2.0,trapbase/2.0,trapbase/2.0,0.0,fwindowy/2.0,0.0,0.0,0.0);
  Double_t trtrapx=0.16-3.50125;
  TGeoTranslation *trfwintrapl = new TGeoTranslation(trtrapx,trfwindowy,0.0);
  TGeoTranslation *trfwintrapr = new TGeoTranslation(-trtrapx,trfwindowy,0.0);
  TGeoTranslation *trfwinframe = new TGeoTranslation(0.0,0.0,trfwinframez);
  TGeoTranslation *trfwindow = new TGeoTranslation(0.0,trfwindowy,trfwinframez);
  fwinframe->AddNode(fwinframeside,1,trfwinframel);
  fwinframe->AddNode(fwinframeside,2,trfwinframer);
  fwinframe->AddNode(fwinframet,1,trfwinframet);
  fwinframe->AddNode(fwinframeb,1,trfwinframeb);
  fwinframe->AddNode(fwintrapl,1,trfwintrapl);
  fwinframe->AddNode(fwintrapr,1,trfwintrapr);

  //Make copper strips
  TGeoVolume *allstrips = geom->MakeBox("allstrips",vacuum,200.,200.,200.);
  Double_t stripx=0.004;
  Double_t stripy=0.6;
  Double_t stripz=cagesidez;
  TGeoVolume *strip = geom->MakeBox("strip",copper,stripx/2.,stripy/2.,stripz/2.);
  Double_t halfstripy=0.3;
  TGeoVolume *halfstrip = geom->MakeBox("halfstrip",copper,stripx/2.,halfstripy/2.,stripz/2.);
  Double_t trstripoutx=actcenterx/2.+actsidex+cagesidex+stripx/2.;
  Double_t trstripinx=actcenterx/2.+actsidex-stripx/2.;
  Double_t trstripouty1=cagesidey/2.-0.2-stripy/2.;
  Double_t trstripiny1=trstripouty1-0.5;
  Double_t trhalfstripy=cagesidey/2.-halfstripy/2.;
  Double_t fstriplx=2.0*cagefrontsidex+cagefronttopx;
  TGeoVolume *fstripl = geom->MakeBox("fstripl",copper,fstriplx/2.,stripy/2.,stripx/2.);
  Double_t fstripsx=cagefrontsidex;
  TGeoVolume *fstrips = geom->MakeBox("fstrips",copper,fstripsx/2.,stripy/2.,stripx/2.);
  Double_t trfoutstripz=actcenterz/2.+actfrontz+cagefrontz+stripx/2.;
  Double_t trfstripsx=trcagefrontsidex;
  Double_t fhalfstripsx=fstriplx-2*fstripsx;
  TGeoVolume *fhalfstripl = geom->MakeBox("fhalfstripl",copper,fstriplx/2.,halfstripy/2.,stripx/2.);
  TGeoVolume *fhalfstrips = geom->MakeBox("fhalfstrips",copper,fhalfstripsx/2.,halfstripy/2.,stripx/2.);
  Double_t trfinstripz=actcenterz/2.+actfrontz-stripx/2.;
  Double_t trfhalfstripsyup=trstripiny1-1.0*12+halfstripy/2.;
  Double_t trfhalfstripsydown=trstripiny1-1.0*29-halfstripy/2.;
  for (Int_t i=0; i<49; i++) {
    allstrips->AddNode(strip,i+1,new TGeoTranslation(trstripoutx,trstripouty1-1.0*i,0.0));
    allstrips->AddNode(strip,i+1+49,new TGeoTranslation(trstripinx,trstripiny1-1.0*i,0.0));
    allstrips->AddNode(strip,i+1+2*49,new TGeoTranslation(-trstripoutx,trstripouty1-1.0*i,0.0));
    allstrips->AddNode(strip,i+1+3*49,new TGeoTranslation(-trstripinx,trstripiny1-1.0*i,0.0));
  }
  allstrips->AddNode(halfstrip,1,new TGeoTranslation(trstripinx,trhalfstripy,0.0));
  allstrips->AddNode(halfstrip,2,new TGeoTranslation(-trstripinx,trhalfstripy,0.0));
  for (Int_t i=0; i<49; i++) {
    if (i<13) allstrips->AddNode(fstripl,i+1,new TGeoTranslation(0.0,trstripouty1-1.0*i,trfoutstripz));
    if (i>=13 && i<30) {
      allstrips->AddNode(fstrips,i+1-13,new TGeoTranslation(trfstripsx,trstripouty1-1.0*i,trfoutstripz));
      allstrips->AddNode(fstrips,i+1+4,new TGeoTranslation(-trfstripsx,trstripouty1-1.0*i,trfoutstripz));
    }
    if (i>=30) allstrips->AddNode(fstripl,i+1-17,new TGeoTranslation(0.0,trstripouty1-1.0*i,trfoutstripz));

    if (i<12) allstrips->AddNode(fstripl,33+i+1,new TGeoTranslation(0.0,trstripiny1-1.0*i,trfinstripz));
    if (i>=12 && i<30) {
      allstrips->AddNode(fstrips,35+i-12,new TGeoTranslation(trfstripsx,trstripiny1-1.0*i,trfinstripz));
      allstrips->AddNode(fstrips,35+i+6,new TGeoTranslation(-trfstripsx,trstripiny1-1.0*i,trfinstripz));
	}
    if (i>=30) allstrips->AddNode(fstripl,33+i+1-18,new TGeoTranslation(0.0,trstripiny1-1.0*i,trfinstripz));
  }
  allstrips->AddNode(fhalfstripl,1,new TGeoTranslation(0.0,trhalfstripy,trfinstripz));
  allstrips->AddNode(fhalfstrips,1,new TGeoTranslation(0.0,trfhalfstripsyup,trfinstripz));
  allstrips->AddNode(fhalfstrips,2,new TGeoTranslation(0.0,trfhalfstripsydown,trfinstripz));  

  //Make the front window "cradle"
  TGeoVolume *fwincradle = geom->MakeBox("fwincradle",vacuum,200.0,200.0,200.0);
  Double_t fwincradletoptopx=6.35;
  Double_t fwincradletoptopy=9.19;
  Double_t fwincradlez=1.27;
  Double_t trfwincradlez=actcenterz/2.+actfrontz+cagefrontz+stripx+fwincradlez/2.;
  TGeoVolume *fwincradletoptop = geom->MakeBox("fwincradletoptop",Aluminum,fwincradletoptopx/2.,fwincradletoptopy/2.,fwincradlez/2.);
  Double_t trfwincradletoptopy=actcentery/2.-fwincradletoptopy/2.;
  TGeoTranslation *trfwincradletoptop = new TGeoTranslation(0.0,trfwincradletoptopy,trfwincradlez);
  Double_t fwincradlebotboty=15.79;
  TGeoVolume *fwincradlebottombottom = geom->MakeBox("fwincradlebottombottom",Aluminum,fwincradletoptopx/2.,fwincradlebotboty/2.,fwincradlez/2.);
  Double_t trfwincradlebotboty=-actcentery/2.+fwincradlebotboty/2.;
  TGeoTranslation *trfwincradlebottombottom = new TGeoTranslation(0.0,trfwincradlebotboty,trfwincradlez);
  Double_t fwincradlemiddlebottomx=17.62;
  Double_t fwincradlemiddlebottomy=5.31;
  TGeoVolume *fwincradlemiddlebottom = geom->MakeBox("fwincradlemiddlebottom",Aluminum,fwincradlemiddlebottomx/2.,fwincradlemiddlebottomy/2.,fwincradlez/2.);
  Double_t trfwincradlemiddlebottomy=trfwincradlebotboty+fwincradlebotboty/2.+fwincradlemiddlebottomy/2.;
  TGeoTranslation *trfwincradlemiddlebottom = new TGeoTranslation(0.0,trfwincradlemiddlebottomy,trfwincradlez);
  Double_t trfwincradlemiddlebottomy=trfwincradletoptopy-fwincradletoptopy/2.-fwincradlemiddlebottomy/2.;
  TGeoTranslation *trfwincradlemiddletop = new TGeoTranslation(0.0,trfwincradlemiddlebottomy,trfwincradlez);
  Double_t fwincradlemiddlesidex=5.31;
  Double_t fwincradlemiddlesidey=14.0;
  TGeoVolume *fwincradlemiddleside = geom->MakeBox("fwincradlemiddleside",Aluminum,fwincradlemiddlesidex/2.,fwincradlemiddlesidey/2.,fwincradlez/2.);
  Double_t trfwincradlemiddlesidex=fwincradlemiddlebottomx/2.-fwincradlemiddlesidex/2.;
  Double_t trfwincradlemiddlesidey=cagesidey/2.-fwincradletoptopy-fwincradlemiddlebottomy-fwincradlemiddlesidey/2.;
  TGeoTranslation *trfwincradlemiddleleft = new TGeoTranslation(trfwincradlemiddlesidex,trfwincradlemiddlesidey,trfwincradlez);
  TGeoTranslation *trfwincradlemiddleright = new TGeoTranslation(-trfwincradlemiddlesidex,trfwincradlemiddlesidey,trfwincradlez);

  fwincradle->AddNode(fwincradletoptop,1,trfwincradletoptop);
  fwincradle->AddNode(fwincradlebottombottom,1,trfwincradlebottombottom);
  fwincradle->AddNode(fwincradlemiddlebottom,1,trfwincradlemiddlebottom);
  fwincradle->AddNode(fwincradlemiddlebottom,2,trfwincradlemiddletop);
  fwincradle->AddNode(fwincradlemiddleside,1,trfwincradlemiddleleft);
  fwincradle->AddNode(fwincradlemiddleside,2,trfwincradlemiddleright);

  //Make Bottom Plate
  TGeoVolume *bottomplate = geom->MakeBox("bottomplate",vacuum,200.,200.,200.);
  Double_t bottomcenterx=94.6;
  Double_t bottomcentery=1.0541;
  Double_t bottomcenterz=149.86;
  TGeoVolume *bottomcenter = geom->MakeBox("bottomcenter",Aluminum,bottomcenterx/2.,bottomcentery/2.,bottomcenterz/2.);
  Double_t trbottomcentery=-cagesidey/2.-bottomcentery/2.;
  TGeoTranslation *trbottomcenter = new TGeoTranslation(0.0,trbottomcentery,0.0);
  Double_t bottomradius=3.5;
  Double_t bottomsidex=bottomradius;
  Double_t bottomsidey=bottomcentery;
  Double_t bottomsidez=bottomcenterz-2*bottomradius;
  TGeoVolume *bottomoutside = geom->MakeBox("bottomoutside",Aluminum,bottomsidex/2.,bottomsidey/2.,bottomsidez/2.);
  Double_t trbottomsidex=bottomcenterx/2.+bottomsidex/2.;
  TGeoTranslation *trbottomoutsidel = new TGeoTranslation(trbottomsidex,trbottomcentery,0.0);
  TGeoTranslation *trbottomoutsider = new TGeoTranslation(-trbottomsidex,trbottomcentery,0.0);
  TGeoVolume *bpcorner1 = geom->MakeTubs("bpcorner1",Aluminum,0.0,bottomradius,bottomcentery/2.0,0.0,90.0);
  TGeoVolume *bpcorner2 = geom->MakeTubs("bpcorner2",Aluminum,0.0,bottomradius,bottomcentery/2.0,90.0,180.0);
  TGeoVolume *bpcorner3 = geom->MakeTubs("bpcorner3",Aluminum,0.0,bottomradius,bottomcentery/2.0,180.0,270.0);
  TGeoVolume *bpcorner4 = geom->MakeTubs("bpcorner4",Aluminum,0.0,bottomradius,bottomcentery/2.0,270.0,360.0);
  TGeoCombiTrans *cobpcorner1 = new TGeoCombiTrans("cobpcorner1",bottomcenterx/2.0,trbottomcentery,bottomsidez/2.0,rocorner);
  TGeoCombiTrans *cobpcorner2 = new TGeoCombiTrans("cobpcorner2",-bottomcenterx/2.0,trbottomcentery,bottomsidez/2.0,rocorner);
  TGeoCombiTrans *cobpcorner3 = new TGeoCombiTrans("cobpcorner3",-bottomcenterx/2.0,trbottomcentery,-bottomsidez/2.0,rocorner);
  TGeoCombiTrans *cobpcorner4 = new TGeoCombiTrans("cobpcorner4",bottomcenterx/2.0,trbottomcentery,-bottomsidez/2.0,rocorner);
  bottomplate->AddNode(bottomcenter,1,trbottomcenter);
  bottomplate->AddNode(bottomoutside,1,trbottomoutsidel);
  bottomplate->AddNode(bottomoutside,2,trbottomoutsider);
  bottomplate->AddNode(bpcorner1,1,cobpcorner1);
  bottomplate->AddNode(bpcorner2,1,cobpcorner2);
  bottomplate->AddNode(bpcorner3,1,cobpcorner3);
  bottomplate->AddNode(bpcorner4,1,cobpcorner4);
  
  //Make back window frame
  Double_t backwindowtopx=89.6584;
  Double_t backwindowtopy=3.15;
  Double_t backwindowtopz=1.27;
  TGeoVolume *backwindowframe = geom->MakeBox("backwindowframe",vacuum,200.,200.,200.);
  TGeoVolume *backwindowtop = geom->MakeBox("backwindowtop",Aluminum,backwindowtopx/2.0,backwindowtopy/2.0,backwindowtopz/2.0);
  Double_t trbackwindowtopy=actcentery/2.0-backwindowtopy/2.0;
  TGeoTranslation *trbackwindowtop = new TGeoTranslation(0.0,trbackwindowtopy,0.0);
  backwindowframe->AddNode(backwindowtop,1,trbackwindowtop);
  Double_t backwindowbottomx=89.6584;
  Double_t backwindowbottomy=3.55;
  Double_t backwindowbottomz=1.27;
  TGeoVolume *backwindowbottom = geom->MakeBox("backwindowbottom",Aluminum,backwindowbottomx/2.0,backwindowbottomy/2.0,backwindowbottomz/2.0);
  Double_t trbackwindowbottomy=-actcentery/2.0+backwindowbottomy/2.0;
  TGeoTranslation *trbackwindowbottom = new TGeoTranslation(0.0,trbackwindowbottomy,0.0);
  backwindowframe->AddNode(backwindowbottom,1,trbackwindowbottom);
  Double_t backwindowsidex=2.4492;
  Double_t backwindowsidey=42.9;
  Double_t backwindowsidez=1.27;
  TGeoVolume *backwindowside = geom->MakeBox("backwindowside",Aluminum,backwindowsidex/2.0,backwindowsidey/2.0,backwindowsidez/2.0);
  Double_t trbackwindowsidex=backwindowtopx/2.0-backwindowsidex/2.0;
  Double_t trbackwindowsidey=(backwindowbottomy-backwindowtopy)/2.0;
  TGeoTranslation *trbackwindowleft = new TGeoTranslation(trbackwindowsidex,trbackwindowsidey,0.0);
  TGeoTranslation *trbackwindowright = new TGeoTranslation(-trbackwindowsidex,trbackwindowsidey,0.0);
  backwindowframe->AddNode(backwindowside,1,trbackwindowleft);
  backwindowframe->AddNode(backwindowside,1,trbackwindowright);
  Double_t trbackwindowframez=actcenterz/2.0+actradius+backwindowtopz/2.0;
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
  TGeoVolume *backwindowtop2 = geom->MakeBox("backwindowtop2",Aluminum,backwindowtop2x/2.0,backwindowtop2y/2.0,backwindowtop2z/2.0);
  Double_t trbackwindowtop2y=backwindowbottom2y/2.0+backwindowside2y/2.0;
  Double_t trbackwindowtop2z=-backwindowtop2z/2.0-backwindowtopz/2.0;
  TGeoTranslation *trbackwindowtop2 = new TGeoTranslation(0.0,trbackwindowtop2y,trbackwindowtop2z);
  backwindowframe->AddNode(backwindowtop2,1,trbackwindowtop2);

  TGeoVolume *backwindowbottom2 = geom->MakeBox("backwindowbottom2",Aluminum,backwindowbottom2x/2.0,backwindowbottom2y/2.0,backwindowbottom2z/2.0);
  Double_t trbackwindowbottom2y=-backwindowside2y/2.0-backwindowtop2y/2.0;
  Double_t trbackwindowbottom2z=trbackwindowtop2z;
  TGeoTranslation *trbackwindowbottom2 = new TGeoTranslation(0.0,trbackwindowbottom2y,trbackwindowbottom2z);
  backwindowframe->AddNode(backwindowbottom2,1,trbackwindowbottom2);

  TGeoVolume *backwindowside2 = geom->MakeBox("backwindowside2",Aluminum,backwindowside2x/2.0,backwindowside2y/2.0,backwindowside2z/2.0);
  Double_t trbackwindowside2x=backwindowtop2x/2.0-backwindowside2x/2.0;
  Double_t trbackwindowside2z=trbackwindowtop2z;
  Double_t trbackwindowside2y=(backwindowbottom2y-backwindowtop2y)/2.0;
  TGeoTranslation *trbackwindowleft2 = new TGeoTranslation(trbackwindowside2x,trbackwindowside2y,trbackwindowside2z);
  TGeoTranslation *trbackwindowright2 = new TGeoTranslation(-trbackwindowside2x,trbackwindowside2y,trbackwindowside2z);
  backwindowframe->AddNode(backwindowside2,1,trbackwindowleft2);
  backwindowframe->AddNode(backwindowside2,2,trbackwindowright2);

  //Making back window
  Double_t backwindowx=89.76;
  Double_t backwindowy=48.2475;
  Double_t backwindowz=0.0125;
  TGeoVolume *backwindow = geom->MakeBox("backwindow",kapton,backwindowx/2.0,backwindowy/2.0,backwindowz/2.0);
  Double_t trbackwindowz=-actcenterz/2.0-actradius-backwindowtopz-backwindowtop2z-backwindowz/2.0;
  TGeoTranslation *trbackwindow = new TGeoTranslation(0.0,0.0,trbackwindowz);
  
  //Making top frame
  TGeoVolume *topframe = geom->MakeBox("topframe",vacuum,200.,200.,200.);
  topframe->SetVisibility(kFALSE);
  Double_t topframesidex=17.5326;
  Double_t topframesidey=0.8763;
  Double_t topframesidez=160.0175;
  TGeoVolume *topframeside = geom->MakeBox("topframeside",Aluminum,topframesidex/2.0,topframesidey/2.0,topframesidez/2.0);
  Double_t topframefbx=95.8523;
  Double_t topframefby=0.8763;
  Double_t topframefbz=8.0594;
  TGeoVolume *topframefb = geom->MakeBox("topframefb",Aluminum,topframefbx/2.0,topframefby/2.0,topframefbz/2.0);
  Double_t trtopframesidex=topframefbx/2.0+topframesidex/2.0;
  Double_t trtopframesidey=topframefby/2.0+actcentery/2.0;
  TGeoTranslation *trtopframeleft = new TGeoTranslation(trtopframesidex,trtopframesidey,0.0);
  TGeoTranslation *trtopframeright = new TGeoTranslation(-trtopframesidex,trtopframesidey,0.0);
  Double_t trtopframefbz=topframesidez/2.0-topframefbz/2.0;
  TGeoTranslation *trtopframefront = new TGeoTranslation(0.0,trtopframesidey,trtopframefbz);
  TGeoTranslation *trtopframeback = new TGeoTranslation(0.0,trtopframesidey,-trtopframefbz);
  topframe->AddNode(topframeside,1,trtopframeleft);
  topframe->AddNode(topframeside,2,trtopframeright);
  topframe->AddNode(topframefb,1,trtopframefront);
  topframe->AddNode(topframefb,2,trtopframeback);
  //Making Lexan part (currently called Aluminum)
  Double_t topframefblexanx=126.6;
  Double_t topframefblexany=1.905;
  Double_t topframefblexanz=2.0;
  Double_t topframesidelexanx=2.0;
  Double_t topframesidelexany=1.905;
  Double_t topframesidelexanz=159.7;
  TGeoVolume *topframefblexan = geom->MakeBox("topframefblexan",Aluminum,topframefblexanx/2.0,topframefblexany/2.0,topframefblexanz/2.0);
  TGeoVolume *topframesidelexan = geom->MakeBox("topframesidelexan",Aluminum,topframesidelexanx/2.0,topframesidelexany/2.0,topframesidelexanz/2.0);
  Double_t trtopframesidelexanx=topframefblexanx/2.0+topframesidelexanx/2.0;
  Double_t trtopframesidelexany=actcentery/2.0+topframesidelexany/2.0+0.8763; //where does the last part come from???
  TGeoTranslation *trtopframeleftlexan = new TGeoTranslation(trtopframesidelexanx,trtopframesidelexany,0.0);
  TGeoTranslation *trtopframerightlexan = new TGeoTranslation(-trtopframesidelexanx,trtopframesidelexany,0.0);
  Double_t trtopframefblexanz=topframesidelexanz/2.0-topframefblexanz/2.0;
  TGeoTranslation *trtopframefrontlexan = new TGeoTranslation(0.0,trtopframesidelexany,trtopframefblexanz);
  TGeoTranslation *trtopframebacklexan = new TGeoTranslation(0.0,trtopframesidelexany,-trtopframefblexanz);
  topframe->AddNode(topframesidelexan,1,trtopframeleftlexan);
  topframe->AddNode(topframesidelexan,2,trtopframerightlexan);
  topframe->AddNode(topframefblexan,1,trtopframefrontlexan);
  topframe->AddNode(topframefblexan,2,trtopframebacklexan);

  //Making basic rib assembly
  TGeoVolume *ribmain = geom->MakeBox("ribmain",vacuum,200.,200.,200.);
  ribmain->SetVisibility(kFALSE);
  Double_t ribx=152.4;
  Double_t riby=10.9042;
  Double_t ribz=206.06;
  TGeoVolume *rib = geom->MakeBox("rib",Aluminum,ribx/2.0,riby/2.0,ribz/2.0);
  //Adding some lips to close area
  Double_t riblipsidex=10.74125;
  Double_t riblipsidey=3.01498;
  Double_t riblipsidez=206.06;
  TGeoVolume *riblipside = geom->MakeBox("riblipside",Aluminum,riblipsidex/2.0,riblipsidey/2.0,riblipsidez/2.0);

  //Squeak still needs to finish the rib assembly
  TGeoTranslation *trriblipsideleft = new TGeoTranslation(70.829375,-6.95959,0.0);
  TGeoTranslation *trriblipsideright = new TGeoTranslation(-70.829375,-6.95959,0.0);
  TGeoVolume *riblipfb = geom->MakeBox("riblipfb",Aluminum,130.9175/2.0,3.01498/2.0,23.02125/2.0);
  TGeoTranslation *trriblipfbfront = new TGeoTranslation(0.0,-6.95959,91.51938);
  TGeoTranslation *trriblipfbback = new TGeoTranslation(0.0,-6.95959,-91.51938);
  ribmain->AddNode(rib,1);
  ribmain->AddNode(riblipside,1,trriblipsideleft);
  ribmain->AddNode(riblipside,2,trriblipsideright);
  ribmain->AddNode(riblipfb,1,trriblipfbfront);
  ribmain->AddNode(riblipfb,1,trriblipfbback);
  TGeoTranslation *trrib = new TGeoTranslation(0.0,33.26708,0.0);

  //Making wire plane
  TGeoVolume *wireplane = geom->MakeBox("wireplane",vacuum,200.,200.,200.);
  wireplane->SetVisibility(kFALSE);
  //Outer Bars
  Double_t wpoutbarx=2.0;
  Double_t wpoutbary=1.4;
  Double_t wpoutbarz=145.6;
  TGeoVolume *wpoutbar = geom->MakeBox("wpoutbar",Aluminum,wpoutbarx/2.0,wpoutbary/2.0,wpoutbarz/2.0);
  //The spacing variables are measured from inside edge to inside edge
  Double_t wpoutspacing=118.32;
  Double_t wpmidspacing=113.52;
  Double_t wpinspacing=103.17;
  Double_t trwpoutbarx=wpoutspacing/2.0+wpoutbarx/2.0;
  TGeoTranslation *trwpoutbarleft = new TGeoTranslation(trwpoutbarx,0.0,0.0);
  TGeoTranslation *trwpoutbarright = new TGeoTranslation(-trwpoutbarx,0.0,0.0);
  wireplane->AddNode(wpoutbar,1,trwpoutbarleft);
  wireplane->AddNode(wpoutbar,1,trwpoutbarright);
  wpoutbar->SetLineColor(kBlue);
  //Middle Bars
  Double_t wpmidbarx=2.0;
  Double_t wpmidbary=0.8;
  Double_t wpmidbarz=145.6;
  TGeoVolume *wpmidbar = geom->MakeBox("wpmidbar",Aluminum,wpmidbarx/2.0,wpmidbary/2.0,wpmidbarz/2.0);
  Double_t trwpmidbarx=wpmidspacing/2.0+wpmidbarx/2.0;
  Double_t trwpmidbary=wpoutbary/2.0-wpmidbary/2.0;
  TGeoTranslation *trwpmidbarleft = new TGeoTranslation(trwpmidbarx,trwpmidbary,0.0);
  TGeoTranslation *trwpmidbarright = new TGeoTranslation(-trwpmidbarx,trwpmidbary,0.0);
  wireplane->AddNode(wpmidbar,1,trwpmidbarleft);
  wireplane->AddNode(wpmidbar,1,trwpmidbarright);
  wpmidbar->SetLineColor(kBlue);
  //Inner Bars
  Double_t wpinbarx=2.0;
  Double_t wpinbary=0.4;
  Double_t wpinbarz=145.6;
  TGeoVolume *wpinbar = geom->MakeBox("wpinbar",Aluminum,wpinbarx/2.0,wpinbary/2.0,wpinbarz/2.0);
  Double_t trwpinbarx=wpinspacing/2.0+wpinbarx/2.0;
  Double_t trwpinbary=wpoutbary/2.0-wpinbary/2.0;
  TGeoTranslation *trwpinbarleft = new TGeoTranslation(trwpinbarx,trwpinbary,0.0);
  TGeoTranslation *trwpinbarright = new TGeoTranslation(-trwpinbarx,trwpinbary,0.0);
  wireplane->AddNode(wpinbar,1,trwpinbarleft);
  wireplane->AddNode(wpinbar,1,trwpinbarright);
  wpinbar->SetLineColor(kBlue);
  //Wires
  //Squeak is down to here
  TGeoRotation *rowire = new TGeoRotation("rocorner",90.0,90.0,0.0);
  TGeoVolume *wpoutwire = geom->MakeTube("wpoutwire",copper,0.0,0.00375,122.32/2.0);
  TGeoVolume *wpmidwire = geom->MakeTube("wpmidwire",copper,0.0,0.00375,117.52/2.0);
  TGeoVolume *wpinwire = geom->MakeTube("wpinwire",copper,0.0,0.001,107.17/2.0);

  TGeoCombiTrans *cowireout[1455];
  TGeoCombiTrans *cowiremid[1455];
  TGeoCombiTrans *cowirein[363];

  for (Int_t i=0; i<1455; i++) {
    cowireout[i] = new TGeoCombiTrans("cowireout",0.0,-0.7127,72.75-0.1*i,rowire);
    wireplane->AddNode(wpoutwire,i+1,cowireout[i]);
    cowiremid[i] = new TGeoCombiTrans("cowiremid",0.0,-0.1127,72.75-0.1*i,rowire);
    wireplane->AddNode(wpmidwire,i+1,cowiremid[i]);
  }
  for (Int_t i=0; i<363; i++) {
    cowirein[i] = new TGeoCombiTrans("cowirein",0.0,0.2873,72.6-0.4*i,rowire);
    wireplane->AddNode(wpinwire,i+1,cowirein[i]);
  }
  TGeoTranslation *trwireplane = new TGeoTranslation(0.0,26.9575,4.804);



  //Make Pad Plane (Setting to be .062 inches (.15748 cm) thick
  TGeoVolume *padplane = geom->MakeBox("padplane",vacuum,200.,200.,200.);
  padplane->SetVisibility(kFALSE);
  Double_t padplanepcbx=86.6;
  Double_t padplanepcby=0.254;
  Double_t padplanepcbz=134.4;
  TGeoVolume *padplaneplane = geom->MakeBox("padplaneplane",pcbmvd,padplanepcbx/2.0,padplanepcby/2.0,padplanepcbz/2.0);
  padplane->AddNode(padplaneplane,1);
  Double_t padx=86.6;
  Double_t pady=0.01;
  Double_t padz=134.4;
  TGeoVolume *pad = geom->MakeBox("pad",copper,padx/2.0,pady/2.0,padz/2.0);
  Double_t trpady=padplanepcby/2.0+pady/2.0;
  TGeoTranslation *trpad = new TGeoTranslation(0.0,trpady,0.0);
  padplane->AddNode(pad,1,trpad);

  /* TGeoVolume *pad = geom->MakeBox("pad",copper,0.8/2.0,0.01/2.0,1.2/2.0); */
  /* pad->SetLineColor(kBlue); */
  /* padplaneplane->SetLineColor(kMagenta); */
  /* Int_t padrun=1; */
  /* TGeoTranslation *trpad[108][112]; */
  /* for (Int_t i=0; i<108; i++) { */
  /*   for (Int_t j=0; j<112; j++) { */
  /*     trpad[i][j] = new TGeoTranslation(42.8-0.8*i,-0.08374,66.6-1.2*j); */
  /*     padplane->AddNode(pad,padrun,trpad[i][j]); */
  /*     padrun++; */
  /*   } */
  /* } */
  TGeoTranslation *trpadplane = new TGeoTranslation(0.0,27.5305,5.08548); //need to verify z shift

  TGeoVolume *actup1 = geom->MakeBox("actup1",p10,95.8523/2.0,0.8763/2.0,143.8987/2.0);
  TGeoTranslation *tractup1 = new TGeoTranslation(0.0,25.23815,0.0);
  TGeoVolume *actup2 = geom->MakeBox("actup2",p10,126.6/2.0,1.905/2.0,154.7/2.0);
  TGeoTranslation *tractup2 = new TGeoTranslation(0.0,26.6288,0.0);
  active->AddNode(actup1,1,tractup1);
  active->AddNode(actup2,1,tractup2);

  /* padplane->SetLineColor(kMagenta); */
  /* rib->SetLineColor(kBlue); */
  /* riblipside->SetLineColor(kBlue); */
  /* riblipfb->SetLineColor(kMagenta); */
  
  //Add everything to the "top" drawing
  //Strips are added in the loops up above

  //uhhhhhhh
  //whole->AddNode(padplane,1,trpadplane); 

  //Set Object Colors
  cageside->SetLineColor(kRed);
  cagefronttop->SetLineColor(kRed);
  cagefrontside->SetLineColor(kRed);
  cagefrontbottom->SetLineColor(kRed);
  corner1->SetLineColor(kMagenta);
  corner2->SetLineColor(kMagenta);
  corner3->SetLineColor(kMagenta);
  corner4->SetLineColor(kMagenta);
  fwinframeside->SetLineColor(kBlue);
  fwinframet->SetLineColor(kBlue);
  fwinframeb->SetLineColor(kBlue);
  fwintrapl->SetLineColor(kBlue);
  fwintrapr->SetLineColor(kBlue);
  fwindow->SetLineColor(kOrange);
  strip->SetLineColor(kMagenta);
  halfstrip->SetLineColor(kMagenta);
  actup1->SetLineColor(kGreen);
  actup2->SetLineColor(kGreen);
  actcenter->SetLineColor(kGreen);
  actfront->SetLineColor(kGreen);
  actside->SetLineColor(kGreen);
  actcorner1->SetLineColor(kGreen);
  actcorner2->SetLineColor(kGreen);
  actcorner3->SetLineColor(kGreen);
  actcorner4->SetLineColor(kGreen);
  backwindow->SetLineColor(kBlue);
  backwindowtop2->SetLineColor(kMagenta);
  backwindowbottom2->SetLineColor(kMagenta);
  backwindowside2->SetLineColor(kMagenta);
  fwincradletoptop->SetLineColor(kBlue);
  fwincradlebottombottom->SetLineColor(kBlue);
  fwincradlemiddlebottom->SetLineColor(kBlue);
  fwincradlemiddleside->SetLineColor(kBlue);
  bottomcenter->SetLineColor(kBlue);
  bottomoutside->SetLineColor(kBlue);
  bpcorner1->SetLineColor(kBlue);
  bpcorner2->SetLineColor(kBlue);
  bpcorner3->SetLineColor(kBlue);
  bpcorner4->SetLineColor(kBlue);

  //works
  whole->AddNode(active,1);
  whole->AddNode(cage,1);
  whole->AddNode(corners,1);
  whole->AddNode(allstrips,1);
  whole->AddNode(bottomplate,1);
  whole->AddNode(fwinframe,1,trfwinframe);
  whole->AddNode(fwindow,1,trfwindow);
  whole->AddNode(fwincradle,1);
   whole->AddNode(backwindowframe,1,trbackwindowframe); 
   whole->AddNode(backwindow,1,trbackwindow); 
    whole->AddNode(topframe,1);
  whole->AddNode(wireplane,1,trwireplane);
  whole->AddNode(ribmain,1,trrib);

  //TGeoTranslation *trpadplane = new TGeoTranslation(0.0,27.5305,5.08548); //need to verify z shift
  TGeoRotation *rowhole = new TGeoRotation("rowhole",180.0,180.0,0.0);
  TGeoCombiTrans *cowhole = new TGeoCombiTrans("cowhole",0.0,-27.5305,5.08548+134.4/2.0,rowhole);
  top->AddNode(whole,1,cowhole);
  //top->AddNode(whole,1);

  //Set Visibility
  //Big Nodes
  top->SetVisibility(kFALSE);
  whole->SetVisibility(kFALSE);
  active->SetVisibility(kFALSE);
  fwinframe->SetVisibility(kFALSE);
  allstrips->SetVisibility(kFALSE);
  bottomplate->SetVisibility(kFALSE);
  backwindowframe->SetVisibility(kFALSE);

  //Little Elements
  //cageside->SetVisibility(kFALSE);
  //strip->SetVisibility(kFALSE);
  //halfstrip->SetVisibility(kFALSE);
  //fstripl->SetVisibility(kFALSE);
  //fstripltop->SetVisibility(kFALSE);
  //fstrips->SetVisibility(kFALSE);
  //wpoutwire->SetVisibility(kFALSE);
  //wpmidwire->SetVisibility(kFALSE);
  //wpinwire->SetVisibility(kFALSE);


  gGeoManager->CloseGeometry();
  
  top->Draw("");
  //top->Draw();

  //top->CheckGeometry();

   TFile *outfile = new TFile("testingsave_geom.root","recreate"); 
   //geom->Write();
   top->Write(); 
   outfile->Close(); 

}
