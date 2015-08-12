/**
 * @brief Event display task
 * @author JungWoo Lee (Korea Univ.)
 */

#include "STEventDrawTask.hh"

ClassImp(STEventDrawTask);

STEventDrawTask* STEventDrawTask::fInstance=0;
STEventDrawTask* STEventDrawTask::Instance() { return fInstance; }

STEventDrawTask::STEventDrawTask()
{
  fEventManager = NULL;
  fEventManagerEditor = NULL;

  fMCHitArray           = NULL;
  fDriftedElectronArray = NULL;
  fHitArray             = NULL;
  fHitClusterArray      = NULL;
  fRiemannTrackArray    = NULL;
  fRawEventArray        = NULL;
  fKalmanArray          = NULL;

  fRawEvent = NULL;

  fSet2dPlotRangeFlag = kFALSE;
  fSetDigiFileFlag = kFALSE;

  fCurrentEvent = -1;
  fCurrentRow   = -1;
  fCurrentLayer = -1;

  fHistPad = NULL;
  fCvsPad  = NULL;
  fRangeMin = 0;
  fRangeMax = 0;

  fCvsPadPlane = NULL;
  fPadPlane    = NULL;
  fMinZ = 0;
  fMaxZ = 1344;
  fMinX = 432;
  fMaxX = -432;

  for (Int_t i=0; i<10; i++)
  {
    fPointSet[i] = NULL;
    fThresholdMin[i] = 0;
    fThresholdMax[i] = 1.E10;
  }

  // MC
  fPointStyle[0] = kFullCircle;
  fPointSize[0]  = 1;
  fPointColor[0] = kGray+2;
  fRnrSelf[0]    = kFALSE;

  // Digi
  fPointStyle[1] = 1;
  fPointSize[1]  = 1.;
  fPointColor[1] = kAzure-5;
  fRnrSelf[1]    = kFALSE;

  // Hit
  fPointStyle[2] = kFullCircle;
  fPointSize[2]  = 0.5;
  fPointColor[2] = kRed-7;
  fRnrSelf[2]    = kFALSE;

  // Cluster
  fPointStyle[3] = kFullCircle;
  fPointSize[3]  = 1.;
  fPointColor[3] = kBlack;
  fRnrSelf[3]    = kFALSE;
  fRnrSelf[9]    = kFALSE;

  // Riemann
  fPointStyle[4] = kFullCircle;
  fPointSize[4]  = 1.;
  fPointColor[4] = 0;
  fRnrSelf[4]    = kTRUE;

  fBoxClusterSet = NULL;
fInstance = this;
}

STEventDrawTask::~STEventDrawTask()
{
}

void 
STEventDrawTask::SetParContainers()
{
  fLogger -> Debug(MESSAGE_ORIGIN,"SetParContainers of STEventDrawTask");

  FairRunAna* ana = FairRunAna::Instance();
  FairRuntimeDb* rtdb = ana -> GetRuntimeDb();
  fPar = (STDigiPar*) rtdb -> getContainer("STDigiPar");
}

InitStatus 
STEventDrawTask::Init()
{
  fLogger -> Debug(MESSAGE_ORIGIN,"Init()");

  FairRootManager* ioMan = FairRootManager::Instance();
  fEventManager = STEventManager::Instance();

  fMCHitArray           = (TClonesArray*) ioMan -> GetObject("STMCPoint");
  fDriftedElectronArray = (TClonesArray*) ioMan -> GetObject("STDriftedElectron");
  fHitArray             = (TClonesArray*) ioMan -> GetObject("STEventH");
  fHitClusterArray      = (TClonesArray*) ioMan -> GetObject("STEventHC");
  fRiemannTrackArray    = (TClonesArray*) ioMan -> GetObject("STRiemannTrack");
  fKalmanArray          = (TClonesArray*) ioMan -> GetObject("STKalmanTrack");
  fRawEventArray        = (TClonesArray*) ioMan -> GetObject("STRawEvent");

  gStyle -> SetPalette(55);
  fCvsPadPlane = fEventManager -> GetCvsPadPlane();
  fCvsPadPlane -> AddExec("ex", "STEventDrawTask::ClickSelectedPadPlane()");
  DrawPadPlane();

  fNTbs          = fPar -> GetWindowNumTbs();
  fXPadPlane     = fPar -> GetPadPlaneX();
  fTBTime        = fPar -> GetTBTime();
  fDriftVelocity = fPar -> GetGas() -> GetDriftVelocity();
  fDriftVelocity = fDriftVelocity/100.;

  fCvsPad = fEventManager -> GetCvsPad();
  SetHistPad();

  return kSUCCESS;
}

void 
STEventDrawTask::Exec(Option_t* option)
{
  fLogger -> Debug(MESSAGE_ORIGIN,"Exec()");

  Reset();

  if (fMCHitArray) DrawMCPoints();
  if (fDriftedElectronArray) DrawDriftedElectrons();
  if (fHitArray) DrawHitPoints();
  if (fHitClusterArray) DrawHitClusterPoints();
  if (fRiemannTrackArray) DrawRiemannHits();

  gEve -> Redraw3D(kFALSE);
  UpdateCvsPadPlane();
}

void 
STEventDrawTask::DrawMCPoints()
{
  fLogger -> Debug(MESSAGE_ORIGIN,"Draw MC Points");

  Int_t nPoints = fMCHitArray -> GetEntries();

  fPointSet[0] = new TEvePointSet("MC", nPoints, TEvePointSelectorConsumer::kTVT_XYZ);
  fPointSet[0] -> SetOwnIds(kTRUE);
  fPointSet[0] -> SetMarkerColor(fPointColor[0]);
  fPointSet[0] -> SetMarkerSize(fPointSize[0]);
  fPointSet[0] -> SetMarkerStyle(fPointStyle[0]);

  for (Int_t iPoint=0; iPoint<nPoints; iPoint++)
  {
    STMCPoint *point = (STMCPoint*) fMCHitArray -> At(iPoint);
              
    fPointSet[0] -> SetNextPoint(point -> GetX(), point -> GetY(), point -> GetZ());
    fPointSet[0] -> SetPointId(new TNamed(Form("MC %d",iPoint),""));
  }

  fPointSet[0] -> SetRnrSelf(fRnrSelf[0]);
  gEve -> AddElement(fPointSet[0]);
}

void 
STEventDrawTask::DrawDriftedElectrons()
{
  fLogger -> Debug(MESSAGE_ORIGIN,"Draw Drifted Electrons");

  Int_t nPoints = fDriftedElectronArray -> GetEntries();

  fPointSet[1] = new TEvePointSet("DriftedElectron", nPoints, TEvePointSelectorConsumer::kTVT_XYZ);
  fPointSet[1] -> SetOwnIds(kTRUE);
  fPointSet[1] -> SetMarkerColor(fPointColor[1]);
  fPointSet[1] -> SetMarkerSize(fPointSize[1]);
  fPointSet[1] -> SetMarkerStyle(fPointStyle[1]);

  for(Int_t iPoint=0; iPoint<nPoints; iPoint++)
  {
    STDriftedElectron *electron = (STDriftedElectron*) fDriftedElectronArray -> At(iPoint);

    Double_t x = electron -> GetX() + electron -> GetDiffusedX();
    Double_t y = electron -> GetHitY() + fDriftVelocity*(electron -> GetDiffusedTime() + electron -> GetDriftTime());
    Double_t z = electron -> GetZ() + electron -> GetDiffusedZ();

    fPointSet[1] -> SetNextPoint(x/10.,y/10.,z/10.);
    fPointSet[1] -> SetPointId(new TNamed(Form("DrftEl %d",iPoint),""));
  }
  fPointSet[1] -> SetRnrSelf(fRnrSelf[1]);
  gEve -> AddElement(fPointSet[1]);
}

void 
STEventDrawTask::DrawHitPoints()
{
  fLogger -> Debug(MESSAGE_ORIGIN,"DrawPoints()");

  STEvent* event = (STEvent*) fHitArray -> At(0);
  Int_t nHits = event -> GetNumHits();

  fPointSet[2] = new TEvePointSet("Hit",nHits, TEvePointSelectorConsumer::kTVT_XYZ);
  fPointSet[2] -> SetOwnIds(kTRUE);
  fPointSet[2] -> SetMarkerColor(fPointColor[2]);
  fPointSet[2] -> SetMarkerSize(fPointSize[2]);
  fPointSet[2] -> SetMarkerStyle(fPointStyle[2]);

  for (Int_t iHit=0; iHit<nHits; iHit++)
  {
    STHit hit = event -> GetHitArray() -> at(iHit);

    if (hit.GetCharge() < fThresholdMin[2] || hit.GetCharge() > fThresholdMax[2])
      continue;

    TVector3 position = hit.GetPosition();
    fPointSet[2] -> SetNextPoint(position.X()/10.,position.Y()/10.,position.Z()/10.);
    fPointSet[2] -> SetPointId(new TNamed(Form("Hit %d",iHit),""));

    fPadPlane -> Fill(-position.X(), position.Z(), hit.GetCharge());
  }

  fPointSet[2] -> SetRnrSelf(fRnrSelf[2]);
  gEve -> AddElement(fPointSet[2]);
}

void 
STEventDrawTask::DrawHitClusterPoints()
{
  fLogger -> Debug(MESSAGE_ORIGIN,"DrawHitClusterPoints()");
  STEvent* event = (STEvent*) fHitClusterArray -> At(0);
  Int_t nClusters = event -> GetNumClusters();

  fBoxClusterSet = new TEveBoxSet("BoxCluster");
  fBoxClusterSet -> UseSingleColor();
  fBoxClusterSet -> SetMainColor(kBlue);
  fBoxClusterSet -> SetMainTransparency(30);
  fBoxClusterSet -> Reset(TEveBoxSet::kBT_AABox, kFALSE, 64);

  fPointSet[3] = new TEvePointSet("HitCluster", nClusters, TEvePointSelectorConsumer::kTVT_XYZ);
  fPointSet[3] -> SetOwnIds(kTRUE);
  fPointSet[3] -> SetMarkerColor(fPointColor[3]);
  fPointSet[3] -> SetMarkerSize(fPointSize[3]);
  fPointSet[3] -> SetMarkerStyle(fPointStyle[3]);

  fLogger -> Debug(MESSAGE_ORIGIN,Form("Number of clusters: %d",nClusters));
  for (Int_t iCluster=0; iCluster<nClusters; iCluster++)
  {
    STHitCluster cluster = event -> GetClusterArray() -> at(iCluster);

    if (cluster.GetCharge() < fThresholdMin[3] || cluster.GetCharge() > fThresholdMax[3])
      continue;

    TVector3 position = cluster.GetPosition();
    TVector3 sigma = cluster.GetPosSigma();
    fPointSet[3] -> SetNextPoint(position.X()/10.,position.Y()/10.,position.Z()/10.);
    fPointSet[3] -> SetPointId(new TNamed(Form("HitCluster %d",iCluster),""));

    Double_t xS =  sigma.X()/10.;
    Double_t yS =  sigma.Y()/10.;
    Double_t zS =  sigma.Z()/10.;

    Double_t xP =  position.X()/10. - xS/2.;
    Double_t yP =  position.Y()/10. - yS/2.;
    Double_t zP =  position.Z()/10. - zS/2.;

    fBoxClusterSet -> AddBox(xP, yP, zP, xS, yS, zS);
    fBoxClusterSet -> DigitValue(cluster.GetCharge());
  }

  fPointSet[3] -> SetRnrSelf(fRnrSelf[3]);
  fBoxClusterSet -> SetRnrSelf(fRnrSelf[9]);

  gEve -> AddElement(fPointSet[3]);
  gEve -> AddElement(fBoxClusterSet);
}

void 
STEventDrawTask::DrawRiemannHits()
{
  fLogger -> Debug(MESSAGE_ORIGIN,"DrawRiemannHits()");

  STRiemannTrack* track = NULL;
  STHitCluster* rCluster = NULL;
  TEvePointSet* riemannPointSet = NULL;

  STEvent* event = (STEvent*) fHitClusterArray -> At(0);

  Int_t nTracks = fRiemannTrackArray -> GetEntries();
  for (Int_t iTrack=0; iTrack<nTracks; iTrack++) 
  {
    track = (STRiemannTrack*) fRiemannTrackArray -> At(iTrack);

    Int_t nClusters = track -> GetNumHits();
    riemannPointSet = new TEvePointSet(Form("RiemannTrack_%d", iTrack), nClusters, TEvePointSelectorConsumer::kTVT_XYZ);
    riemannPointSet -> SetMarkerColor(GetRiemannColor(iTrack));
    riemannPointSet -> SetMarkerSize(fPointSize[4]);
    riemannPointSet -> SetMarkerStyle(fPointStyle[4]);

    for (Int_t iCluster=0; iCluster<nClusters; iCluster++)
    {
      rCluster = track -> GetHit(iCluster) -> GetCluster();

      if (rCluster -> GetCharge() < fThresholdMin[4] || rCluster -> GetCharge() > fThresholdMax[4])
        continue;

      Int_t id = rCluster -> GetClusterID();
      STHitCluster oCluster = event -> GetClusterArray() -> at(id);

      TVector3 position = oCluster.GetPosition();
      riemannPointSet -> SetNextPoint(position.X()/10.,position.Y()/10.,position.Z()/10.);
      riemannPointSet -> SetPointId(new TNamed(Form("RiemanCluster %d",iCluster),""));
    }

    riemannPointSet -> SetRnrSelf(fRnrSelf[4]);
    gEve -> AddElement(riemannPointSet);
    fRiemannSetArray.push_back(riemannPointSet);
  }
}

void 
STEventDrawTask::SetSelfRiemannSet(Int_t iRiemannSet, Bool_t offElse)
{
  Int_t nRiemannSets = fRiemannSetArray.size();

  if (iRiemannSet==-1) 
  {
    if (!offElse)
    {
      for (Int_t i=0; i<nRiemannSets; i++)
      {
        TEvePointSet* pointSet = fRiemannSetArray[i];
        pointSet -> SetRnrSelf(kTRUE);
      }
    }
    else
    {
      for (Int_t i=0; i<nRiemannSets; i++)
      {
        TEvePointSet* pointSet = fRiemannSetArray[i];
        pointSet -> SetRnrSelf(kFALSE);
      }
    }
  }

  else 
  {
    for (Int_t i=0; i<nRiemannSets; i++)
    {
      TEvePointSet* pointSet = fRiemannSetArray[i];
      if (i==iRiemannSet) pointSet -> SetRnrSelf(kTRUE);
      else if (offElse) pointSet -> SetRnrSelf(kFALSE);
    }
  }
}

void 
STEventDrawTask::SetRendering(STEveObject eveObj, Bool_t rnr, Double_t thresholdMin, Double_t thresholdMax)
{
  fRnrSelf[eveObj] = rnr;

  if (thresholdMin != -1) 
    fThresholdMin[eveObj] = thresholdMin;

  if (thresholdMin != -1) 
    fThresholdMax[eveObj] = thresholdMax;
}


void 
STEventDrawTask::SetAttributes(STEveObject eveObj, Style_t style, Size_t size, Color_t color)
{
  if (style != -1) 
    fPointStyle[eveObj] = style;

  if (size != -1) 
    fPointSize[eveObj] = size;

  if (color != -1) 
    fPointColor[eveObj] = color;
}

void
STEventDrawTask::Reset()
{
  fLogger -> Debug(MESSAGE_ORIGIN,"Reset PointSets");

  for(Int_t i=0; i<10; i++)
  {
    if(fPointSet[i]) 
    {
      fPointSet[i] -> Reset();
      gEve -> RemoveElement(fPointSet[i], fEventManager);
    }
  }

  if (fBoxClusterSet) {
    fBoxClusterSet -> Reset();
    gEve -> RemoveElement(fBoxClusterSet, fEventManager);
  }

  fLogger -> Debug(MESSAGE_ORIGIN,"Reset RiemannTracks");

  TEvePointSet* pointSet;
  Int_t nRiemannTracks = fRiemannSetArray.size();

  if (nRiemannTracks != 0) 
  {
    for (Int_t i=0; i<nRiemannTracks; i++)
    {
      pointSet = fRiemannSetArray[i];
      gEve -> RemoveElement(pointSet, fEventManager);
    }
    fRiemannSetArray.clear();
  }

  if (fPadPlane != NULL)
    fPadPlane -> Reset("");
}

void
STEventDrawTask::Set2DPlotRange(Int_t uaIdx)
{
  if (uaIdx%100<0 || uaIdx%100>11 || uaIdx/100<0 || uaIdx/100>3) 
  {
    fLogger -> Error(MESSAGE_ORIGIN, 
      "2DPlotRange should be ABB ( A = [0, 3], BB = [00, 11] )!");
    return;
  }

  fMinZ = (uaIdx/100)*12*7*4;
  fMaxZ = (uaIdx/100 + 1)*12*7*4;
  fMinX = (uaIdx%100)*8*9 - 432;
  fMaxX = (uaIdx%100 + 1)*8*9 - 432;

  fSet2dPlotRangeFlag = kTRUE;
}

void 
STEventDrawTask::SetHistPad()
{
  if (fHistPad)
  {
    fHistPad -> Reset();
    return;
  }

  fCvsPad -> cd();
  fHistPad = new TH1D("Pad","",fNTbs,0,fNTbs);
  fHistPad -> SetLineColor(9);
  fHistPad -> SetFillColor(9);
  fHistPad -> SetFillStyle(3002);
  fHistPad -> GetXaxis() -> SetTickLength(0.01);
  fHistPad -> GetXaxis() -> SetTitle("time bucket");
  fHistPad -> GetXaxis() -> CenterTitle();
  fHistPad -> GetXaxis() -> SetLabelSize(0.05);
  fHistPad -> GetXaxis() -> SetTitleSize(0.05);
  fHistPad -> GetYaxis() -> SetTickLength(0.01);
  fHistPad -> GetYaxis() -> SetTitle("adc");
  fHistPad -> GetYaxis() -> CenterTitle();
  fHistPad -> GetYaxis() -> SetLabelSize(0.05);
  fHistPad -> GetYaxis() -> SetTitleSize(0.05);
  fHistPad -> SetMinimum(0);
  fHistPad -> SetMaximum(4095);
  fHistPad -> SetStats(0);
  fHistPad -> Draw();
  fCvsPad -> SetGridy();
  fCvsPad -> SetGridx();
}

void
STEventDrawTask::DrawPadPlane()
{
  if (fPadPlane) 
  {
    fPadPlane -> Reset();
    return;
  }

  fCvsPadPlane -> cd();
  fPadPlane = new TH2D("padplane", "", 108, -432, 432, 112, 0, 1344);
  fPadPlane -> GetXaxis() -> SetTickLength(0.01);
  fPadPlane -> GetXaxis() -> SetTitle("-x (mm)");
  fPadPlane -> GetXaxis() -> CenterTitle();
  fPadPlane -> GetYaxis() -> SetTickLength(0.01);
  fPadPlane -> GetYaxis() -> SetTitle("z (mm)");
  fPadPlane -> GetYaxis() -> SetTitleOffset(1.3);
  fPadPlane -> GetYaxis() -> CenterTitle();
  fPadPlane -> SetMinimum(0);
  fPadPlane -> SetMaximum(4095);
  fPadPlane -> SetStats(0);
  fPadPlane -> Draw("colz");
  fCvsPadPlane -> SetGridy();
  fCvsPadPlane -> SetGridx();

  /*
  for (Int_t i = 0; i < 107; i++) {
    Int_t z[2] = {-432 + (i + 1)*8, -432 + (i + 1)*8};
    Int_t x[2] = {0, 1344};
    TGraph *line = new TGraph(2, z, x);
    line -> SetMarkerStyle(1);
    if ((i + 1)%9 == 0)
        line -> SetLineStyle(1);
    else
        line -> SetLineStyle(3);
    line -> Draw("same");
  }

  for (Int_t i = 0; i < 111; i++) {
    Int_t z[2] = {-432, 432};
    Int_t x[2] = {(i + 1)*12, (i + 1)*12};
    TGraph *line = new TGraph(2, z, x);
    line -> SetMarkerStyle(1);
    if ((i + 1)%7 == 0)
        line -> SetLineStyle(1);
    else
        line -> SetLineStyle(3);
    line -> Draw("same");
  }
  */
}

void 
STEventDrawTask::UpdateCvsPadPlane()
{
  fCvsPadPlane -> Modified();
  fCvsPadPlane -> Update();

  TPaletteAxis *paxis 
    = (TPaletteAxis *) fPadPlane -> GetListOfFunctions() -> FindObject("palette");

  if (paxis) {
    if (fSet2dPlotRangeFlag) {
      paxis -> SetX1NDC(0.940);
      paxis -> SetX2NDC(0.955);
      paxis -> SetLabelSize(0.08);
      paxis -> GetAxis() -> SetTickSize(0.01);
    } else {
      paxis -> SetX1NDC(0.905);
      paxis -> SetX2NDC(0.94);
    }

    fCvsPadPlane -> Modified();
    fCvsPadPlane -> Update();
  }
}

Color_t 
STEventDrawTask::GetRiemannColor(Int_t index)
{
  Color_t color;
  Color_t colors[] = {kRed, kBlue, kMagenta, kCyan, kGreen, kYellow, kPink, kSpring, kAzure, kOrange, kViolet, kTeal};

  Int_t remainder = index%12;
  color = colors[remainder];

  Int_t quotient  = index/12;
  Int_t offColor  = (quotient%10) -5;
  color += offColor;

  return color;
}

void
STEventDrawTask::DrawPad(Int_t row, Int_t layer)
{
  if (!fRawEventArray) return;
  if (row==fCurrentRow && layer==fCurrentLayer) return;

  fCurrentRow = row;
  fCurrentLayer = layer;
  fEventManagerEditor -> SetRowLayer(row, layer);
  Int_t currentEvent = fEventManager -> GetCurrentEvent();
  if (currentEvent!=fCurrentEvent) 
  {
    fCurrentEvent = currentEvent;
    fRawEvent = (STRawEvent*) fRawEventArray -> At(0);
  }
  STPad* pad = fRawEvent -> GetPad(row, layer);
  if (!pad) return;
  Double_t* adc = pad -> GetADC();

  fHistPad -> SetTitle(Form("row: %d, layer: %d",row, layer));
  for (Int_t tb=0; tb<fNTbs; tb++)
    fHistPad -> SetBinContent(tb+1, adc[tb]);


  STEvent* event = (STEvent*) fHitArray -> At(0);
  Int_t nHits = event -> GetNumHits();

  fCvsPad -> cd();
  fHistPad -> Draw();
  fCvsPad -> Modified();
  fCvsPad -> Update();
}

void 
STEventDrawTask::DrawPadByPosition(Double_t x, Double_t z)
{
  Int_t row = (-x+fXPadPlane/2)/8;
  Int_t layer = z/12;

  DrawPad(row, layer);
}

void
STEventDrawTask::ClickSelectedPadPlane()
{
  TObject* select = ((TCanvas*)gPad) -> GetClickSelected();
  if (!select) return;

  if (select -> InheritsFrom(TH1::Class()))
  {
    TH2D* hist = (TH2D*) select;

    Int_t uniqueID = gPad -> GetUniqueID();
    Int_t xEvent = gPad -> GetEventX();
    Int_t yEvent = gPad -> GetEventY();

    gPad -> SetUniqueID(yEvent);

    Float_t xAbs = gPad -> AbsPixeltoX(xEvent);
    Float_t yAbs = gPad -> AbsPixeltoY(yEvent);
    Double_t xOnClick = gPad -> PadtoX(xAbs);
    Double_t yOnClick = gPad -> PadtoY(yAbs);

    Int_t bin = hist -> FindBin(xOnClick,yOnClick);
    Double_t content = hist -> GetBinContent(bin);

    STEventDrawTask::Instance() -> DrawPadByPosition(xOnClick,yOnClick);
  }

  ((TCanvas*)gPad) -> SetClickSelected(0);
}

void 
STEventDrawTask::SetRange(Double_t min, Double_t max) 
{ 
  fLogger -> Warning(MESSAGE_ORIGIN, "This method will be removed in next version. Please use SetRendering() instead.");

  fThresholdMin[2] = min;
  fThresholdMax[2] = max;

  fThresholdMin[3] = min;
  fThresholdMax[3] = max;
}

void 
STEventDrawTask::SetEventManagerEditor(STEventManagerEditor* pointer)
{ 
  fEventManagerEditor = pointer; 
} 

Int_t STEventDrawTask::GetNRiemannSet() { return fRiemannSetArray.size(); }
