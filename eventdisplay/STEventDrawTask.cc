/**
 * @brief Event display task
 * @author JungWoo Lee (Korea Univ.)
 */

#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

#include "STEventDrawTask.hh"
#include "STPad.hh"

#include "TEveManager.h"
#include "TPaletteAxis.h"
#include "TStyle.h"

#include <iostream>

using namespace std;

ClassImp(STEventDrawTask);

STEventDrawTask* STEventDrawTask::fInstance=0;
STEventDrawTask* STEventDrawTask::Instance()
{
  return fInstance;
}

STEventDrawTask::STEventDrawTask()
: fCurrentEvent(-1),
  fCurrentRow(-1),
  fCurrentLayer(-1),
  fSet2dPlotRangeFlag(kFALSE),
  fSetDigiFileFlag(kFALSE),
  fPar(0),
  fNTbs(0),
  fXPadPlane(0),
  fTBTime(0),
  fDriftVelocity(0),
  //fDigiTree(0),
  //fRawEventArray(0),
  fRawEvent(0),
  fHistPad(0),
  fHitArray(0),
  fHitClusterArray(0),
  fRiemannTrackArray(0),
  fRawEventArray(0),
  fKalmanArray(0),
  fEventManager(0),
  fEventManagerEditor(0),
  fThreshold(0),
  fRangeMin(0),
  fRangeMax(0),
  fHitSet(0),
  fHitColor(kPink),
  fHitSize(1),
  fHitStyle(6),
  fBoxClusterSet(0),
  fBoxClusterColor(kBlue),
  fBoxClusterTransparency(30),
  fHitClusterSet(0),
  fHitClusterColor(kGray+3),
  fHitClusterSize(1),
  fHitClusterStyle(7),
  fRiemannSetArray(0),
  fRiemannColor(kBlue),
  fRiemannSize(2.0),
  fRiemannStyle(kOpenCircle),
  fCvsPadPlane(0),
  fPadPlane(0),
  fMinZ(0),
  fMaxZ(1344),
  fMinX(432),
  fMaxX(-432),
  fCvsPad(0)
{
  fInstance = this;

  for(Int_t i=0; i<20; i++)
    fGraphHitTb[i] = new TGraph();
}

STEventDrawTask::~STEventDrawTask()
{
}

void 
STEventDrawTask::SetParContainers()
{
  fLogger->Debug(MESSAGE_ORIGIN,"SetParContainers of STEventDrawTask");

  FairRunAna* ana = FairRunAna::Instance();
  FairRuntimeDb* rtdb = ana->GetRuntimeDb();
  fPar = (STDigiPar*) rtdb->getContainer("STDigiPar");
}


InitStatus 
STEventDrawTask::Init()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Init()");
  FairRootManager* ioMan = FairRootManager::Instance();
  fEventManager = STEventManager::Instance();

  fHitArray = (TClonesArray*) ioMan -> GetObject("STEventH");
  if(fHitArray) LOG(INFO)<<"Hit Found."<<FairLogger::endl;

  fHitClusterArray = (TClonesArray*) ioMan -> GetObject("STEventHC");
  if(fHitClusterArray) 
    LOG(INFO)<<"Hit Cluster Found."<<FairLogger::endl;
  else
    LOG(INFO)<<"Hit Cluster Not Found."<<FairLogger::endl;

  fRiemannTrackArray = (TClonesArray*) ioMan -> GetObject("STRiemannTrack");
  if(fRiemannTrackArray) 
    LOG(INFO)<<"Riemann Track Found."<<FairLogger::endl;
  else
    LOG(INFO)<<"Riemann Track Not Found."<<FairLogger::endl;

  fKalmanArray = (TClonesArray*) ioMan -> GetObject("STKalmanTrack");
  if(fKalmanArray) 
    LOG(INFO)<<"Kalman Track Found."<<FairLogger::endl;
  else
    LOG(INFO)<<"Kalman Track Not Found."<<FairLogger::endl;

  fRawEventArray = (TClonesArray*) ioMan -> GetObject("STRawEvent");
  if(fRawEventArray) 
    LOG(INFO)<<"Raw Event Found."<<FairLogger::endl;
  else
    LOG(INFO)<<"Raw Event Not Found."<<FairLogger::endl;

  gStyle -> SetPalette(55);
  fCvsPadPlane = fEventManager -> GetCvsPadPlane();
  fCvsPadPlane -> AddExec("ex","STEventDrawTask::ClickSelectedPadPlane()");
  DrawPadPlane();

  fNTbs = fPar -> GetNumTbs();
  fXPadPlane = fPar -> GetPadPlaneX();

  fTBTime = fPar -> GetTBTime();
  fDriftVelocity  = fPar -> GetGas() -> GetDriftVelocity();
  fDriftVelocity  = fDriftVelocity/100;

  fCvsPad = fEventManager -> GetCvsPad();
  SetHistPad();

  return kSUCCESS;
}

void 
STEventDrawTask::Exec(Option_t* option)
{
  fLogger->Debug(MESSAGE_ORIGIN,"Exec()");
  Reset();

  if(fHitArray) DrawHitPoints();
  if(fHitClusterArray) DrawHitClusterPoints();
  if(fRiemannTrackArray) DrawRiemannHits();

  gEve -> Redraw3D(kFALSE);
  UpdateCvsPadPlane();
}

void 
STEventDrawTask::DrawHitPoints()
{
  fLogger->Debug(MESSAGE_ORIGIN,"DrawPoints()");
  STEvent* event = (STEvent*) fHitArray -> At(0);
  Int_t nHits = event -> GetNumHits();

  fHitSet = new TEvePointSet("Hit",nHits, TEvePointSelectorConsumer::kTVT_XYZ);
  fHitSet -> SetOwnIds(kTRUE);
  fHitSet -> SetMarkerColor(fHitColor);
  fHitSet -> SetMarkerSize(fHitSize);
  fHitSet -> SetMarkerStyle(fHitStyle);

  for(Int_t iHit=0; iHit<nHits; iHit++)
  {
    STHit hit = event -> GetHitArray() -> at(iHit);
    if(hit.GetCharge()<fThreshold) continue;
    if(hit.GetCharge()<fRangeMin||hit.GetCharge()>fRangeMax) continue;
    TVector3 position = hit.GetPosition();
    fHitSet -> SetNextPoint(position.X()/10.,position.Y()/10.,position.Z()/10.);
    fHitSet -> SetPointId(new TNamed(Form("Hit %d",iHit),""));

    fPadPlane -> Fill(-position.X(), position.Z(), hit.GetCharge());
    //fPadPlane -> Fill(position.Z(), position.X(), hit.GetCharge());
  }
  fHitSet -> SetRnrSelf(kFALSE);
  gEve -> AddElement(fHitSet);
}

void 
STEventDrawTask::DrawHitClusterPoints()
{
  fLogger->Debug(MESSAGE_ORIGIN,"DrawHitClusterPoints()");
  STEvent* event = (STEvent*) fHitClusterArray -> At(0);
  Int_t nClusters = event -> GetNumClusters();

  fBoxClusterSet = new TEveBoxSet("BoxCluster");
  fBoxClusterSet -> UseSingleColor();
  fBoxClusterSet -> SetMainColor(fBoxClusterColor);
  fBoxClusterSet -> SetMainTransparency(fBoxClusterTransparency);
  fBoxClusterSet ->Reset(TEveBoxSet::kBT_AABox, kFALSE, 64);

  fHitClusterSet = new TEvePointSet("HitCluster",nClusters, TEvePointSelectorConsumer::kTVT_XYZ);
  fHitClusterSet -> SetOwnIds(kTRUE);
  fHitClusterSet -> SetMarkerColor(fHitClusterColor);
  fHitClusterSet -> SetMarkerSize(fHitClusterSize);
  fHitClusterSet -> SetMarkerStyle(fHitClusterStyle);

  fLogger->Debug(MESSAGE_ORIGIN,Form("Number of clusters: %d",nClusters));
  for(Int_t iCluster=0; iCluster<nClusters; iCluster++)
  {
    STHitCluster cluster = event -> GetClusterArray() -> at(iCluster);
    if(cluster.GetCharge()<fThreshold) continue;
    if(cluster.GetCharge()<fRangeMin||cluster.GetCharge()>fRangeMax) continue;
    TVector3 position = cluster.GetPosition();
    TVector3 sigma = cluster.GetPosSigma();
    fHitClusterSet -> SetNextPoint(position.X()/10.,position.Y()/10.,position.Z()/10.);
    fHitClusterSet -> SetPointId(new TNamed(Form("HitCluster %d",iCluster),""));

    Double_t xS =  sigma.X()/10.;
    Double_t yS =  sigma.Y()/10.;
    Double_t zS =  sigma.Z()/10.;

    Double_t xP =  position.X()/10. - xS/2.;
    Double_t yP =  position.Y()/10. - yS/2.;
    Double_t zP =  position.Z()/10. - zS/2.;

    fBoxClusterSet -> AddBox(xP, yP, zP, xS, yS, zS);
    fBoxClusterSet -> DigitValue(cluster.GetCharge());
  }
  gEve -> AddElement(fHitClusterSet);
  gEve -> AddElement(fBoxClusterSet);
}

void 
STEventDrawTask::DrawRiemannHits()
{
  fLogger->Debug(MESSAGE_ORIGIN,"DrawRiemannHits()");
  STRiemannTrack* track = 0;
  STHitCluster* rCluster = 0;
  TEvePointSet* riemannClusterSet = 0;

  STEvent* event = (STEvent*) fHitClusterArray -> At(0);

  Int_t nTracks = fRiemannTrackArray -> GetEntries();
  for(Int_t iTrack=0; iTrack<nTracks; iTrack++) 
  {
    track = (STRiemannTrack*) fRiemannTrackArray -> At(iTrack);

    Int_t nClusters = track -> GetNumHits();
    riemannClusterSet = new TEvePointSet(Form("RiemannTrack_%d",iTrack),nClusters,TEvePointSelectorConsumer::kTVT_XYZ);
    riemannClusterSet -> SetMarkerColor(GetRiemannColor(iTrack));
    riemannClusterSet -> SetMarkerSize(fRiemannSize);
    riemannClusterSet -> SetMarkerStyle(fRiemannStyle);
    for(Int_t iCluster=0; iCluster<nClusters; iCluster++)
    {
      rCluster = track -> GetHit(iCluster) -> GetCluster();
      //if((rCluster -> GetCharge()) < fThreshold) continue;
      Int_t id = rCluster -> GetClusterID();
      STHitCluster oCluster = event -> GetClusterArray() -> at(id);

      TVector3 position = oCluster.GetPosition();
      riemannClusterSet -> SetNextPoint(position.X()/10.,position.Y()/10.,position.Z()/10.);
      riemannClusterSet -> SetPointId(new TNamed(Form("RiemanCluster %d",iCluster),""));
    }
    gEve -> AddElement(riemannClusterSet);
    fRiemannSetArray.push_back(riemannClusterSet);
  }
}

void 
STEventDrawTask::SetSelfRiemannSet(Int_t iRiemannSet, Bool_t offElse)
{
  Int_t nRiemannSets = fRiemannSetArray.size();

  if(iRiemannSet==-1) 
  {
    if(!offElse)
    {
      for(Int_t i=0; i<nRiemannSets; i++)
      {
        TEvePointSet* pointSet = fRiemannSetArray[i];
        pointSet -> SetRnrSelf(kTRUE);
      }
    }
    else
    {
      for(Int_t i=0; i<nRiemannSets; i++)
      {
        TEvePointSet* pointSet = fRiemannSetArray[i];
        pointSet -> SetRnrSelf(kFALSE);
      }
    }
  }

  else 
  {
    for(Int_t i=0; i<nRiemannSets; i++)
    {
      TEvePointSet* pointSet = fRiemannSetArray[i];
      if(i==iRiemannSet) pointSet -> SetRnrSelf(kTRUE);
      else if(offElse) pointSet -> SetRnrSelf(kFALSE);
    }
  }

}

void
STEventDrawTask::Reset()
{
  if(fHitSet) {
    fHitSet -> Reset();
    gEve -> RemoveElement(fHitSet, fEventManager);
  }

  if(fHitClusterSet) {
    fHitClusterSet -> Reset();
    gEve -> RemoveElement(fHitClusterSet, fEventManager);
  }

  if(fBoxClusterSet) {
    fBoxClusterSet -> Reset();
    gEve -> RemoveElement(fBoxClusterSet, fEventManager);
  }

  Int_t nRiemannTracks = fRiemannSetArray.size();
  TEvePointSet* pointSet;
  if(nRiemannTracks!=0) {
    for(Int_t i=0; i<nRiemannTracks; i++){
      pointSet = fRiemannSetArray[i];
      gEve -> RemoveElement(pointSet, fEventManager);
    }
    fRiemannSetArray.clear();
  }

  if(fPadPlane!=NULL)
    fPadPlane -> Reset();
}

void
STEventDrawTask::Set2DPlotRange(Int_t uaIdx)
{
  if(uaIdx%100<0 || uaIdx%100>11 || uaIdx/100<0 || uaIdx/100>3) 
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
  if(fHistPad)
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
  if(fPadPlane) 
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
    if(fSet2dPlotRangeFlag) {
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

void 
STEventDrawTask::SetHitAttributes(Color_t color, Size_t size, Style_t style)
{
  fHitColor = color;
  fHitSize = size;
  fHitStyle = style;
}

void 
STEventDrawTask::SetHitClusterAttributes(Color_t color, Size_t size, Style_t style)
{
  fHitClusterColor = color;
  fHitClusterSize = size;
  fHitClusterStyle = style;
}

void 
STEventDrawTask::SetRiemannAttributes(Color_t color, Size_t size, Style_t style)
{
  fRiemannColor = color;
  fRiemannSize = size;
  fRiemannStyle = style;
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

/*
void
STEventDrawTask::SetDigiFile(TString name)
{
  TFile* digiFile = new TFile(name,"read");
  if(!digiFile) return;

  fDigiTree = (TTree*) digiFile -> Get("cbmsim");
  fDigiTree -> SetBranchAddress("STRawEvent", &fRawEventArray);

  fSetDigiFileFlag = kTRUE;
}
*/

void
STEventDrawTask::DrawPad(Int_t row, Int_t layer)
{
  if(!fRawEventArray) return;
  if(row==fCurrentRow && layer==fCurrentLayer) return;

  fCurrentRow = row;
  fCurrentLayer = layer;
  fEventManagerEditor -> SetRowLayer(row, layer);
  Int_t currentEvent = fEventManager -> GetCurrentEvent();
  if(currentEvent!=fCurrentEvent) 
  {
    /*
    fDigiTree -> GetEntry(currentEvent);
    fCurrentEvent = currentEvent;
    */
    fRawEvent = (STRawEvent*) fRawEventArray -> At(0);
  }
  STPad* pad = fRawEvent -> GetPad(row, layer);
  if(!pad) return;
  Double_t* adc = pad -> GetADC();

  fHistPad -> SetTitle(Form("row: %d, layer: %d",row, layer));
  for(Int_t tb=0; tb<fNTbs; tb++)
    fHistPad -> SetBinContent(tb+1, adc[tb]);


  STEvent* event = (STEvent*) fHitArray -> At(0);
  Int_t nHits = event -> GetNumHits();

  /*
  for(Int_t i=0; i<20; i++)
    fGraphHitTb[i] -> Set(0);

  for(Int_t iHit=0; iHit<nHits; iHit++)
  {
    STHit hit = event -> GetHitArray() -> at(iHit);
    if(hit.GetCharge()<fThreshold) continue;
    TVector3 position = hit.GetPosition();
    Double_t tb = position.Z()/fDriftVelocity/fTBTime;
    //...
  }
  */


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
  if(!select) return;

  if(select -> InheritsFrom(TH1::Class()))
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
