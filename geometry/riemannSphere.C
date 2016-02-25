void riemannSphere(Double_t radius = 50)
{
  Double_t padplaneHeight = 0.1;

  zSize = 150.;
  xSize = 50.;

  new TGeoManager("riemannSphere","riemannSphere");

  TGeoMedium *vacuum = new TGeoMedium("vacuum" ,1, new TGeoMaterial("vacuum"));

  TGeoVolume* top = gGeoManager -> MakeBox("top",vacuum,1000,1000,1000);
  gGeoManager -> SetTopVolume(top);

  TGeoVolume* padplane = gGeoManager -> MakeBox("padplane",vacuum,xSize,padplaneHeight,zSize/2);
  TGeoTranslation* transPP = new TGeoTranslation("transPP",0,0,zSize/2);
  padplane -> SetLineColor(18);
  padplane -> SetTransparency(90); 
  top -> AddNode(padplane, 1, transPP);

  TGeoVolume* riemannSphere = gGeoManager -> MakeSphere("riemannSphere",vacuum,0,radius);
  TGeoRotation*   rotatRS = new TGeoRotation("rotatRS",0,90.,0);
  TGeoCombiTrans* combiRS = new TGeoCombiTrans("combiRS",0,radius,0,rotatRS);
  riemannSphere -> SetLineColor(kBlue-4);
  riemannSphere -> SetTransparency(90); 
  top -> AddNode(riemannSphere, 1, combiRS);

  top->Draw("ogl");

  TString dir = gSystem->Getenv("VMCWORKDIR");
  TString geoFileName = dir + "/geometry/riemannSphere.root";

  TFile *geoFile = new TFile(geoFileName,"recreate"); 
  //top -> Write(); 
  gGeoManager -> Write(); 
  geoFile -> Close(); 
}
