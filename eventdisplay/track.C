void track()
{
  TEveManager::Create();

  TFile* file = new TFile("../geometry/geomSpiRIT.man.root", "read");
  if (file -> IsZombie()){
    cout << "geometry file is zombie!" << endl;
    return;
  }

  file -> Get("SpiRIT");
  TGeoNode* geoNode = gGeoManager -> GetTopNode();
  TEveGeoTopNode* topNode 
    = new TEveGeoTopNode(gGeoManager, geoNode, 1, 3, 10000);
  gEve -> AddGlobalElement(topNode);

  TChain *fChain = new TChain("cbmsim");
  fChain -> AddFile("../macros/data/cocktail.mc.root");

  TClonesArray *trackListClonesArray = (TClonesArray*) fChain -> GetBranch("GeoTracks");
  TObjArray *trackListArray = new TObjArray();

  TGeoTrack *geoTrack = (TGeoTrack*) trackListClonesArray -> At(0);
  TParticle *particle = (TParticle*) geoTrack -> GetParticle();

  Double_t energy = particle -> Energy();
  Double_t minEnergy = TMath::Min(energy, 0.);
  Double_t manEnergy = TMath::Max(energy, 100.);

  TEveTrackPropagator *trackPropagator = new TEveTrackPropagator();
  TEveTrackList *trackList = new TEveTrackList(particle -> GetName(), trackPropagator);
  trackList -> SetMainColor(kBlue);
  trackListArray -> Add(trackList);
  gEve -> AddElement(trackList, new TEveEventManager);
  
  TEveTrack *track = new TEveTrack(particle, geoTrack -> GetPDG(), trackPropagator);
  track -> SetLineColor(kBlue);

  Int_t numPointsInGeoTrack = geoTrack -> GetNpoints();

  for (Int_t iPoint = 0; iPoint < numPointsInGeoTrack; iPoint++)
  {
    const Double_t *point = geoTrack -> GetPoint(iPoint);
    track -> SetPoint(iPoint, point[0], point[1], point[2]);
    TEveVector pos = TEveVector(point[0], point[1], point[2]);
    TEvePathMark* path = new TEvePathMark();
    path -> fV = pos;
    path -> fTime = point[3];
    if(iPoint == 0) {
      TEveVector Mom = TEveVector(particle -> Px(), particle -> Py(),particle -> Pz());
      path -> fP=Mom;
    }
    track -> AddPathMark(*path);

    delete path;
  }

  trackList -> AddElement(track);

  gEve -> Redraw3D();
}
