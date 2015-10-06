/**
 * @brief Event display task
 * @author JungWoo Lee (Korea Univ.)
 */

#include "FairRun.h"
#include "STEventDrawTask.hh"
#include "FairRootManager.h"
#include "FairRuntimeDb.h"

ClassImp(STEventDrawTask);

STEventDrawTask* STEventDrawTask::fInstance = 0;
STEventDrawTask* STEventDrawTask::Instance() { return fInstance; }

STEventDrawTask::STEventDrawTask()
{
  fInstance = this;

  fEventManager = NULL;
  fEventManagerEditor = NULL;

  fEvent = NULL;

  fMCHitArray           = NULL;
  fDriftedElectronArray = NULL;
  fEventArray           = NULL;
  fRiemannTrackArray    = NULL;
  fRawEventArray        = NULL;

  fRawEvent = NULL;

  fSet2dPlotRangeFlag = kFALSE;
  fSetDigiFileFlag    = kFALSE;

  fCurrentEvent = -2; // avoid -1, will break.
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

  for (Int_t i=0; i<6; i++)
  {
    fPointSet[i] = NULL;
    fThresholdMin[i] = 0;
    fThresholdMax[i] = 1.E10;
    fSetObject[i] = kFALSE;
  }

  fPointStyle[kMC] = kFullCircle;
  fPointSize[kMC]  = 1;
  fPointColor[kMC] = kGray+2;
  fRnrSelf[kMC]    = kFALSE;

  fPointStyle[kDigi] = 1;
  fPointSize[kDigi]  = 1.;
  fPointColor[kDigi] = kAzure-5;
  fRnrSelf[kDigi]    = kFALSE;

  fPointStyle[kHit] = kFullCircle;
  fPointSize[kHit]  = 0.5;
  fPointColor[kHit] = kRed;
  fRnrSelf[kHit]    = kTRUE;
  fSetObject[kHit]  = kTRUE;

  fPointStyle[kCluster] = kFullCircle;
  fPointSize[kCluster]  = 1.;
  fPointColor[kCluster] = kBlack;
  fRnrSelf[kCluster]    = kFALSE;
  fRnrSelf[kClusterBox] = kFALSE;


  fPointStyle[kRiemann] = kFullCircle;
  fPointSize[kRiemann]  = 1.0;
  fPointColor[kRiemann] = 0;
  fRnrSelf[kRiemann]    = kFALSE;
  fSetObject[kRiemann]  = kFALSE;

  fBoxClusterSet = NULL;
}

STEventDrawTask::~STEventDrawTask()
{
}

void 
STEventDrawTask::SetParContainers()
{
  fLogger -> Debug(MESSAGE_ORIGIN,"SetParContainers of STEventDrawTask");

  FairRun* run = FairRun::Instance();
  FairRuntimeDb* rtdb = run -> GetRuntimeDb();
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
  fEventArray           = (TClonesArray*) ioMan -> GetObject("STEvent");
  fRiemannTrackArray    = (TClonesArray*) ioMan -> GetObject("STRiemannTrack");
  fRawEventArray        = (TClonesArray*) ioMan -> GetObject("STRawEvent");

  gStyle -> SetPalette(55);
  fCvsPadPlane = fEventManager -> GetCvsPadPlane();
  fCvsPadPlane -> AddExec("ex", "STEventDrawTask::ClickSelectedPadPlane()");
  DrawPadPlane();

  fNTbs          = fPar -> GetNumTbs();
  fXPadPlane     = fPar -> GetPadPlaneX();
  fTBTime        = fPar -> GetTBTime();
  fDriftVelocity = fPar -> GetDriftVelocity();
  fDriftVelocity = fDriftVelocity/100.;

  fWindowTbStart = fPar -> GetWindowStartTb();
  fWindowTbEnd   = fPar -> GetWindowNumTbs() + fWindowTbStart;

  fWindowYStart = fWindowTbStart    * fTBTime * fDriftVelocity / 10.;
  fWindowYEnd   = fWindowTbEnd * fTBTime * fDriftVelocity / 10.;

  fCvsPad = fEventManager -> GetCvsPad();
  SetHistPad();

  return kSUCCESS;
}

void 
STEventDrawTask::Exec(Option_t* option)
{
  fLogger -> Debug(MESSAGE_ORIGIN,"Exec()");

  Reset();

  fEvent = (STEvent*) fEventArray -> At(0);

  if (fMCHitArray != NULL && fSetObject[kMC]) 
    DrawMCPoints();

  if (fDriftedElectronArray != NULL && fSetObject[kDigi]) 
    DrawDriftedElectrons();

  if (fSetObject[kHit])
    DrawHitPoints();

  if (fSetObject[kCluster] || fSetObject[kClusterBox])
    DrawHitClusterPoints();

  if (fRiemannTrackArray != NULL && fSetObject[kRiemann])
    DrawRiemannHits();

  DrawPad(fCurrentRow, fCurrentLayer);
  UpdatePadRange();

  gEve -> Redraw3D(kFALSE);
  UpdateCvsPadPlane();
}

void 
STEventDrawTask::DrawMCPoints()
{
  fLogger -> Debug(MESSAGE_ORIGIN,"Draw MC Points");

  Int_t nPoints = fMCHitArray -> GetEntries();

  fPointSet[kMC] = new TEvePointSet("MC", nPoints);
  fPointSet[kMC] -> SetOwnIds(kTRUE);
  fPointSet[kMC] -> SetMarkerColor(fPointColor[kMC]);
  fPointSet[kMC] -> SetMarkerSize(fPointSize[kMC]);
  fPointSet[kMC] -> SetMarkerStyle(fPointStyle[kMC]);

  for (Int_t iPoint=0; iPoint<nPoints; iPoint++)
  {
    STMCPoint *point = (STMCPoint*) fMCHitArray -> At(iPoint);
              
    fPointSet[kMC] -> SetNextPoint(point -> GetX(), 
                                   point -> GetY(),
                                   point -> GetZ());
  }

  fPointSet[kMC] -> SetRnrSelf(fRnrSelf[kMC]);
  gEve -> AddElement(fPointSet[kMC]);
}

void 
STEventDrawTask::DrawDriftedElectrons()
{
  fLogger -> Debug(MESSAGE_ORIGIN,"Draw Drifted Electrons");

  Int_t nPoints = fDriftedElectronArray -> GetEntries();

  fPointSet[kDigi] = new TEvePointSet("DriftedElectron", nPoints);
  fPointSet[kDigi] -> SetOwnIds(kTRUE);
  fPointSet[kDigi] -> SetMarkerColor(fPointColor[kDigi]);
  fPointSet[kDigi] -> SetMarkerSize(fPointSize[kDigi]);
  fPointSet[kDigi] -> SetMarkerStyle(fPointStyle[kDigi]);

  for(Int_t iPoint=0; iPoint<nPoints; iPoint++)
  {
    STDriftedElectron *electron = (STDriftedElectron*) fDriftedElectronArray -> At(iPoint);

    Double_t x = electron -> GetX() + electron -> GetDiffusedX();
    Double_t y = electron -> GetHitY() + fDriftVelocity*(electron -> GetDiffusedTime() + electron -> GetDriftTime());
    Double_t z = electron -> GetZ() + electron -> GetDiffusedZ();

    fPointSet[kDigi] -> SetNextPoint(x/10., y/10., z/10.);
  }

  fPointSet[kDigi] -> SetRnrSelf(fRnrSelf[kDigi]);
  gEve -> AddElement(fPointSet[kDigi]);
}

void 
STEventDrawTask::DrawHitPoints()
{
  fLogger -> Debug(MESSAGE_ORIGIN,"DrawHitPoints()");

  if (fEvent == NULL) 
    return;

  Int_t nHits = fEvent -> GetNumHits();

  fPointSet[kHit] = new TEvePointSet("Hit", nHits);
  fPointSet[kHit] -> SetOwnIds(kTRUE);
  fPointSet[kHit] -> SetMarkerColor(fPointColor[kHit]);
  fPointSet[kHit] -> SetMarkerSize(fPointSize[kHit]);
  fPointSet[kHit] -> SetMarkerStyle(fPointStyle[kHit]);

  for (Int_t iHit=0; iHit<nHits; iHit++)
  {
    STHit hit = fEvent -> GetHitArray() -> at(iHit);

    if (hit.GetCharge() < fThresholdMin[kHit] || 
        hit.GetCharge() > fThresholdMax[kHit])
      continue;

    TVector3 position = hit.GetPosition();

    if (hit.GetTb() < fWindowTbStart || hit.GetTb() > fWindowTbEnd)
      continue;

    Double_t y = position.Y()/10.;
    /*
    if (y > -fWindowYStart || y < -fWindowYEnd)
      continue;
    */
    y += fWindowYStart;

    fPointSet[kHit] -> SetNextPoint(position.X()/10.,
                                    y,
                                    position.Z()/10.);

    fPadPlane -> Fill(-position.X(), position.Z(), hit.GetCharge());
  }

  fPointSet[kHit] -> SetRnrSelf(fRnrSelf[kHit]);
  gEve -> AddElement(fPointSet[kHit]);
}

void 
STEventDrawTask::DrawHitClusterPoints()
{
  fLogger -> Debug(MESSAGE_ORIGIN,"DrawHitClusterPoints()");

  if (fEvent == NULL) 
    return;

  Int_t nClusters = fEvent -> GetNumClusters();

  if (fSetObject[kClusterBox]) {
    fBoxClusterSet = new TEveBoxSet("BoxCluster");
    fBoxClusterSet -> UseSingleColor();
    fBoxClusterSet -> SetMainColor(kBlue);
    fBoxClusterSet -> SetMainTransparency(30);
    fBoxClusterSet -> Reset(TEveBoxSet::kBT_AABox, kFALSE, 64);
  }

  if (fSetObject[kCluster]) {
    fPointSet[kCluster] = new TEvePointSet("HitCluster", nClusters);
    fPointSet[kCluster] -> SetOwnIds(kTRUE);
    fPointSet[kCluster] -> SetMarkerColor(fPointColor[kCluster]);
    fPointSet[kCluster] -> SetMarkerSize(fPointSize[kCluster]);
    fPointSet[kCluster] -> SetMarkerStyle(fPointStyle[kCluster]);
  }

  fLogger -> Debug(MESSAGE_ORIGIN,Form("Number of clusters: %d",nClusters));
  for (Int_t iCluster=0; iCluster<nClusters; iCluster++)
  {
    STHitCluster cluster = fEvent -> GetClusterArray() -> at(iCluster);

    if (cluster.GetCharge() < fThresholdMin[kCluster] || 
        cluster.GetCharge() > fThresholdMax[kCluster])
      continue;

    TVector3 position = cluster.GetPosition();

    Double_t y = position.Y()/10.;
    if (y > -fWindowYStart || y < -fWindowYEnd)
      continue;
    y += fWindowYStart;

    TVector3 sigma = cluster.GetPosSigma();

    if (fSetObject[kCluster]) {
      fPointSet[kCluster] -> SetNextPoint(position.X()/10.,
                                          y,
                                          position.Z()/10.);
    }

    Double_t xS =  sigma.X()/10.;
    Double_t yS =  sigma.Y()/10.;
    Double_t zS =  sigma.Z()/10.;

    Double_t xP =  position.X()/10. - xS/2.;
    Double_t yP =  y                - yS/2.;
    Double_t zP =  position.Z()/10. - zS/2.;

    if (fSetObject[kClusterBox]) {
      fBoxClusterSet -> AddBox(xP, yP, zP, xS, yS, zS);
      fBoxClusterSet -> DigitValue(cluster.GetCharge());
    }
  }

  if (fSetObject[kCluster]) {
    fPointSet[kCluster] -> SetRnrSelf(fRnrSelf[kCluster]);
    gEve -> AddElement(fPointSet[kCluster]);
  }

  if (fSetObject[kClusterBox]) {
    fBoxClusterSet -> SetRnrSelf(fRnrSelf[kClusterBox]);
    gEve -> AddElement(fBoxClusterSet);
  }
}

void 
STEventDrawTask::DrawRiemannHits()
{
  fLogger -> Debug(MESSAGE_ORIGIN,"DrawRiemannHits()");

  STRiemannTrack* track = NULL;
  STHit* rHit = NULL;
  TEvePointSet* riemannPointSet = NULL;

  if (fEvent == NULL) 
    return;

  Int_t nTracks = fRiemannTrackArray -> GetEntries();

  for (Int_t iTrack=0; iTrack<nTracks; iTrack++) 
  {
    track = (STRiemannTrack*) fRiemannTrackArray -> At(iTrack);
    Int_t nClusters = track -> GetNumHits();

    riemannPointSet = new TEvePointSet(Form("RiemannTrack_%d", iTrack), 1, TEvePointSelectorConsumer::kTVT_XYZ);
    riemannPointSet -> SetMarkerColor(GetRiemannColor(iTrack));
    riemannPointSet -> SetMarkerSize(fPointSize[kRiemann]);
    riemannPointSet -> SetMarkerStyle(fPointStyle[kRiemann]);

    for (Int_t iCluster=0; iCluster<nClusters; iCluster++)
    {
      rHit = track -> GetHit(iCluster) -> GetHit();

      if (rHit -> GetCharge() < fThresholdMin[kRiemann] || 
          rHit -> GetCharge() > fThresholdMax[kRiemann])
        continue;

      Int_t id = rHit -> GetClusterID();
      STHitCluster oCluster = fEvent -> GetClusterArray() -> at(id);

      TVector3 position = oCluster.GetPosition();

      Double_t y = position.Y()/10.;
      if (y > -fWindowYStart || y < -fWindowYEnd)
        continue;
      y += fWindowYStart;

      riemannPointSet -> SetNextPoint(position.X()/10.,
                                      y,
                                      position.Z()/10.);
    }

    riemannPointSet -> SetRnrSelf(fRnrSelf[kRiemann]);
    gEve -> AddElement(riemannPointSet);
    fRiemannSetArray.push_back(riemannPointSet);
  }
}

void 
STEventDrawTask::SetSelfRiemannSet(Int_t iRiemannSet, Bool_t offElse)
{
  Int_t nRiemannSets = fRiemannSetArray.size();

  if (iRiemannSet == -1) 
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
  fSetObject[eveObj] = kTRUE; 
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
STEventDrawTask::SetObject(STEveObject eveObj, Bool_t set)
{
  fSetObject[eveObj] = set;
}


void
STEventDrawTask::Reset()
{
  fLogger -> Debug(MESSAGE_ORIGIN,"Reset Eve");

  for(Int_t i=0; i<6; i++) 
  {
    if(fPointSet[i]) {
      fPointSet[i] -> Reset();
      gEve -> RemoveElement(fPointSet[i], fEventManager);
      fPointSet[i] = NULL;
    }
  }


  if (fBoxClusterSet) {
    fBoxClusterSet -> Reset();
    gEve -> RemoveElement(fBoxClusterSet, fEventManager);
  }


  Int_t nRiemannTracks = fRiemannSetArray.size();
  for (Int_t i=0; i<nRiemannTracks; i++) 
  {
    TEvePointSet* pointSet = fRiemannSetArray[i];
    gEve -> RemoveElement(pointSet, fEventManager);
  }
  fRiemannSetArray.clear();

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
  fLogger -> Debug(MESSAGE_ORIGIN,"Updating pad plane.");

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
STEventDrawTask::DrawPadByPosition(Double_t x, Double_t z)
{
  Int_t row = (-x+fXPadPlane/2)/8;
  Int_t layer = z/12;

  DrawPad(row, layer);
}

void
STEventDrawTask::DrawPad(Int_t row, Int_t layer)
{
  if (!fRawEventArray) return;

  Int_t currentEvent = fEventManager -> GetCurrentEvent();
  if (fCurrentEvent != currentEvent && 
      row==fCurrentRow && layer==fCurrentLayer) return;

  fCurrentRow = row;
  fCurrentLayer = layer;
  fEventManagerEditor -> SetRowLayer(row, layer);
  if (currentEvent!=fCurrentEvent) 
  {
    fCurrentEvent = currentEvent;
    fRawEvent = (STRawEvent*) fRawEventArray -> At(0);
  }
  STPad* pad = fRawEvent -> GetPad(row, layer);
  if (pad == NULL) 
    return;
  Double_t* adc = pad -> GetADC();

  fHistPad -> SetTitle(Form("row: %d, layer: %d",row, layer));
  fMaxAdcCurrentPad = 0;
  for (Int_t tb=0; tb<fNTbs; tb++) {
    Double_t val = adc[tb];
    if (val > fMaxAdcCurrentPad) fMaxAdcCurrentPad = val;
    fHistPad -> SetBinContent(tb+1, val);
  }

  fHistPad -> SetMaximum(fMaxAdcCurrentPad * 1.1);

  Int_t nHits = fEvent -> GetNumHits();

  fCvsPad -> cd();
  fHistPad -> Draw();
  fCvsPad -> Modified();
  fCvsPad -> Update();
}

void
STEventDrawTask::UpdatePadRange()
{
  fHistPad -> GetXaxis() -> SetRangeUser(fWindowTbStart, fWindowTbEnd);
  fCvsPad -> Modified();
  fCvsPad -> Update();
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
  fLogger -> Warning(MESSAGE_ORIGIN, "This method will be removed in next version. Please use SetThresholdRange() instead.");

  fThresholdMin[kHit] = min;
  fThresholdMax[kHit] = max;

  fThresholdMin[kCluster] = min;
  fThresholdMax[kCluster] = max;
}

void 
STEventDrawTask::SetThresholdRange(STEveObject eve, Double_t min, Double_t max) 
{ 
  fThresholdMin[eve] = min;
  fThresholdMax[eve] = max;
}

void 
STEventDrawTask::SetWindowRange(Int_t start, Int_t end) 
{ 
  fWindowTbStart = start;
  fWindowTbEnd   = end;

  fWindowYStart = fWindowTbStart    * fTBTime * fDriftVelocity / 10.;
  fWindowYEnd   = fWindowTbEnd * fTBTime * fDriftVelocity / 10.;
}

void 
STEventDrawTask::SetWindow(Int_t start, Int_t num) 
{ 
  fWindowTbStart = start;
  fWindowTbEnd   = start + num;

  fWindowYStart = fWindowTbStart    * fTBTime * fDriftVelocity / 10.;
  fWindowYEnd   = fWindowTbEnd * fTBTime * fDriftVelocity / 10.;
}

void 
STEventDrawTask::SetEventManagerEditor(STEventManagerEditor* pointer)
{ 
  fEventManagerEditor = pointer; 
} 

Int_t STEventDrawTask::GetNRiemannSet()   { return fRiemannSetArray.size(); }
Int_t STEventDrawTask::GetWindowTbStart() { return fWindowTbStart; }
Int_t STEventDrawTask::GetWindowTbEnd()   { return fWindowTbEnd; }
