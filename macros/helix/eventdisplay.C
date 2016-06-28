TTree* tree;
TClonesArray *hits = nullptr;
TClonesArray *tracks = nullptr;
TClonesArray *clusters = nullptr;
vector<TEveElement*> hitArray;
vector<TEveElement*> trackHitArray;
vector<TEveElement*> trackLineArray;
vector<TEveElement*> trackClusterArray;
int currentEvent = 0;

void draw(int eventID);
void next() { draw(currentEvent+1); }

void eventdisplay(TString in = "~/data_cosmics/run2771.helix.reco.root")
{
  auto file = new TFile(in);
  tree = (TTree *) file -> Get("cbmsim");
  tree -> SetBranchAddress("STHit", &hits);
  tree -> SetBranchAddress("STHitCluster", &clusters);
  tree -> SetBranchAddress("STHelixTrack", &tracks);

  TEveManager::Create();
  auto geomFile = new TFile("../../geometry/geomSpiRIT.man.root", "read"); 
  geomFile -> Get("SpiRIT");
  gEve -> AddGlobalElement(new TEveGeoTopNode(gGeoManager, gGeoManager->GetTopNode(), 1, 3, 10000));
  gEve -> FullRedraw3D(true);
  gEve -> GetDefaultGLViewer() -> CurrentCamera().RotateRad(-0.7, 1.1);

  draw(currentEvent);
}

void draw(int eventID)
{
  currentEvent = eventID;

  if (currentEvent >= tree -> GetEntries()) { cout << "end" << endl; return; }

  for (auto element : hitArray)
    gEve -> RemoveElement(element, gEve -> GetCurrentEvent());
  hitArray.clear();

  for (auto element : trackHitArray)
    gEve -> RemoveElement(element, gEve -> GetCurrentEvent());
  trackHitArray.clear();

  for (auto element : trackLineArray)
    gEve -> RemoveElement(element, gEve -> GetCurrentEvent());
  trackLineArray.clear();

  for (auto element : trackClusterArray)
    gEve -> RemoveElement(element, gEve -> GetCurrentEvent());
  trackClusterArray.clear();

  cout << "event " << eventID << endl;
  tree -> GetEntry(eventID);

  if (hits != nullptr) {
    auto hitSet = new TEvePointSet("hitSet", hits -> GetEntries());
    hitSet -> SetMarkerColor(kGray);
    hitSet -> SetMarkerSize(0.5);
    hitSet -> SetMarkerStyle(20);
    for (auto i = 0; i < hits -> GetEntries(); i++) {
      auto hit = (STHit *) hits -> At(i);
      TVector3 p = hit -> GetPosition();
      hitSet -> SetNextPoint(.1*p.X(), .1*p.Y(), .1*p.Z());
    }
    gEve -> AddElement(hitSet);
    hitArray.push_back(hitSet);
  }


  auto GetColor = [](int index) {
    Color_t colors[] = {kOrange, kTeal, kViolet, kSpring, kPink, kAzure};
    Color_t color = colors[index%6] + ((index/6)%20);
    return color;
  };


  for (auto iTrack = 0; iTrack < tracks->GetEntries(); iTrack++) {
    //if (iTrack != 6) continue;
    auto track = (STHelixTrack *) tracks -> At(iTrack);
    auto trackHitIDs = track -> GetHitIDArray();
    auto trackClusterIDs = track -> GetClusterIDArray();

    auto pointSet = new TEvePointSet(Form("hit%d",iTrack), track -> GetNumHits());
    pointSet -> SetMarkerColor(GetColor(iTrack));
    pointSet -> SetMarkerSize(1.5);
    pointSet -> SetMarkerStyle(20);
    for (auto hitID : *trackHitIDs) {
      auto p = ((STHit *) hits -> At(hitID)) -> GetPosition();
      pointSet -> SetNextPoint(.1*p.X(), .1*p.Y(), .1*p.Z());
    }
    //gEve -> AddElement(pointSet);
    //trackHitArray.push_back(pointSet);

    auto clusterSet = new TEvePointSet(Form("cluster%d",iTrack), track -> GetNumClusters());
    clusterSet -> SetMarkerColor(GetColor(iTrack));
    clusterSet -> SetMarkerSize(2);
    clusterSet -> SetMarkerStyle(20);
    for (auto clusterID : *trackClusterIDs) {
      auto p = ((STHitCluster *) clusters -> At(clusterID)) -> GetPosition();
      clusterSet -> SetNextPoint(.1*p.X(), .1*p.Y(), .1*p.Z());
    }
    gEve -> AddElement(clusterSet);
    trackClusterArray.push_back(clusterSet);

    auto line = new TEveLine(Form("line%d",iTrack), trackHitIDs->size());
    line -> SetLineColor(kRed);
    line -> SetLineWidth(2);

    for (auto i = -0.1; i <= 1.1; i+= 0.01) {
      auto q = track -> InterpolateByRatio(i);
      line -> SetNextPoint(.1*q.X(), .1*q.Y(), .1*q.Z());
    }
    gEve -> AddElement(line);
    trackLineArray.push_back(line);
  }

  gEve -> Redraw3D();
}
