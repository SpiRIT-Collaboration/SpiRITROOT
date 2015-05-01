void simple_spirit_geometry()
{
  Double_t x = 96.61;
  Double_t y = 51.01;
  Double_t z = 144.64;

  TGeoManager* geoMan = new TGeoManager("SPiRIT","SPiRIT");

  TGeoMixture *mixGas = new TGeoMixture("p10", 1, 1);
  TGeoMedium *medGas = new TGeoMedium("med10", 1, mixGas);

  TGeoVolume* top = new TGeoVolumeAssembly("top");
  TGeoVolume* tpc = new TGeoVolumeAssembly("spirit");
  TGeoVolume* gas = geoMan -> MakeBox("field_cage_in", medGas, x/2, y/2, z/2);

  geoMan -> SetTopVolume(top);
  top -> AddNode(tpc,1);
  tpc -> AddNode(gas,1, new TGeoTranslation(0,-y/2,z/2));

  TFile* geoFile = new TFile("simple_spirit.root", "recreate");
  top -> Write();
  top -> Draw("ogl");
}
