TTree* tree;
TClonesArray *hits = nullptr;
TClonesArray *tracks = nullptr;
vector<TEveElement*> eveArray;
int currentEvent = 0;

void draw(int eventID);
void next() { draw(currentEvent+1); }

void eventdisplay(TString in = "~/data_cosmics/run2771.helix.reco.root")
{
  auto file = new TFile(in);
  tree = (TTree *) file -> Get("cbmsim");
  tree -> SetBranchAddress("STHit", &hits);
  tree -> SetBranchAddress("STHelixTrack", &tracks);

  TEveManager::Create();
  auto geomFile = new TFile("../../geometry/geomSpiRIT.man.root", "read"); 
  geomFile -> Get("SpiRIT");
  gEve -> AddGlobalElement(new TEveGeoTopNode(gGeoManager, gGeoManager->GetTopNode(), 1, 3, 10000));

  draw(currentEvent);
}

void draw(int eventID)
{
  currentEvent = eventID;

  if (currentEvent >= tree -> GetEntries()) { cout << "end" << endl; return; }

  for (auto element : eveArray) {
    cout << element -> GetElementName() << endl;
    gEve -> RemoveElement(element, gEve -> GetCurrentEvent());
  }
  eveArray.clear();

  cout << eventID << endl;
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
    eveArray.push_back(hitSet);
  }


  auto GetColor = [](int index) {
    Color_t colors[] = {kOrange, kTeal, kViolet, kSpring, kPink, kAzure};
    Color_t color = colors[index%6] + ((index/6)%20);
    return color;
  };


  for (auto iTrack = 0; iTrack < tracks->GetEntries(); iTrack++) {
    auto track = (STHelixTrack *) tracks -> At(iTrack);
    auto trackHits = track -> GetHitArray();

    auto pointSet = new TEvePointSet(Form("hits%d",iTrack), track -> GetNumHits());
    pointSet -> SetMarkerColor(GetColor(iTrack));
    pointSet -> SetMarkerSize(1.5);
    pointSet -> SetMarkerStyle(20);
    for (auto hit : *trackHits) {
      auto p = hit -> GetPosition();
      pointSet -> SetNextPoint(.1*p.X(), .1*p.Y(), .1*p.Z());
    } 
    gEve -> AddElement(pointSet);
    eveArray.push_back(pointSet);

    auto line = new TEveLine(Form("line%d",iTrack), trackHits->size()); 
    line -> SetLineColor(kRed);
    line -> SetLineWidth(2);

    for (auto i = -0.1; i <= 1.1; i+= 0.01) {
      auto q = track -> InterpolateByRatio(i);
      if (!(abs(q.X()) > 500 || abs(q.Z()-750) > 750 || abs(q.Y()+300) > 300))
        line -> SetNextPoint(.1*q.X(), .1*q.Y(), .1*q.Z());
    } 
    gEve -> AddElement(line);
    eveArray.push_back(line);
  }

  gEve -> FullRedraw3D(true);
}
