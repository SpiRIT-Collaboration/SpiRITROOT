/**
 * @brief Event display task
 * @author JungWoo Lee (Korea Univ.)
 */

#include "FairRootManager.h"

#include "STEveReco.hh"

#include "TEveManager.h"
#include "TPaletteAxis.h"
#include "TStyle.h"

#include <iostream>

using namespace std;

ClassImp(STEveReco);

STEveReco::STEveReco()
: fIs2DPlotRange(kFALSE),
  fHitArray(0),
  fHitClusterArray(0),
  fRiemannArray(0),
  fKalmanArray(0),
  fEventManager(0),
  fThreshold(0),
  fHitSet(0),
  fHitClusterSet(0),
  fCvsPadPlane(0),
  fPadPlane(0),
  fMinZ(0),
  fMaxZ(1344),
  fMinX(432),
  fMaxX(-432)
{
}

STEveReco::~STEveReco()
{
}

InitStatus 
STEveReco::Init()
{
  FairRootManager* ioMan = FairRootManager::Instance();
  fEventManager = STEveEventManager::Instance();

  fHitArray = (TClonesArray*) ioMan->GetObject("STEventH");
  if(fHitArray) LOG(INFO)<<"Hit Found."<<FairLogger::endl;

  fHitClusterArray = (TClonesArray*) ioMan->GetObject("STEventHC");
  if(fHitClusterArray) LOG(INFO)<<"Hit Cluster Found."<<FairLogger::endl;

  fRiemannArray = (TClonesArray*) ioMan->GetObject("STRiemannHit");
  if(fRiemannArray) LOG(INFO)<<"Riemann Hit Found."<<FairLogger::endl;

  fKalmanArray = (TClonesArray*) ioMan->GetObject("STKalmanTrack");
  if(fKalmanArray) LOG(INFO)<<"Kalman Track Found."<<FairLogger::endl;

  gStyle -> SetPalette(55);
  fCvsPadPlane = fEventManager->GetCvsPadPlane();
  DrawPadPlane();
}

void 
STEveReco::Exec(Option_t* option)
{
  Reset();

  if(fHitArray)
  {
    STEvent* event = (STEvent*) fHitArray->At(0);
    Int_t nHits = event->GetNumHits();

    fHitSet = new TEvePointSet("Hit",nHits, TEvePointSelectorConsumer::kTVT_XYZ);
    fHitSet->SetOwnIds(kTRUE);
    fHitSet->SetMarkerColor(kRed);
    fHitSet->SetMarkerSize(0.5);
    fHitSet->SetMarkerStyle(kFullDotMedium);

    for(Int_t iHit; iHit<nHits; iHit++)
    {
      STHit hit = event->GetHitArray()->at(iHit);
      if(hit.GetCharge()<fThreshold) continue;
      TVector3 position = hit.GetPosition();
      fHitSet->SetNextPoint(position.X()/10.,position.Y()/10.,position.Z()/10.);
      fHitSet->SetPointId(new TNamed(Form("Hit %d",iHit),""));

      fPadPlane->Fill(position.Z(), position.X(), hit.GetCharge());
    }
    gEve -> AddElement(fHitSet);
  }

  if(fHitClusterArray)
  {
    STEvent* event = (STEvent*) fHitClusterArray->At(0);
    Int_t nClusters = event->GetNumClusters();

    fHitClusterSet = new TEvePointSet("HitCluster",nClusters, TEvePointSelectorConsumer::kTVT_XYZ);
    fHitClusterSet->SetOwnIds(kTRUE);
    fHitClusterSet->SetMarkerColor(kBlue);
    fHitClusterSet->SetMarkerSize(1);
    fHitClusterSet->SetMarkerStyle(kOpenCircle);

    for(Int_t iCluster; iCluster<nClusters; iCluster++)
    {
      STHitCluster cluster = event->GetClusterArray()->at(iCluster);
      if(cluster.GetCharge()<fThreshold) continue;
      TVector3 position = cluster.GetPosition();
      fHitClusterSet -> SetNextPoint(position.X()/10.,position.Y()/10.,position.Z()/10.);
      fHitClusterSet -> SetPointId(new TNamed(Form("HitCluster %d",iCluster),""));
    }
    gEve -> AddElement(fHitClusterSet);
  }

  gEve -> Redraw3D(kFALSE);
  UpdateCvsPadPlane();
}

void
STEveReco::Reset()
{
  if(fHitSet!=0) {
    fHitSet->Reset();
    gEve->RemoveElement(fHitSet, fEventManager);
  }

  if(fHitClusterSet!=0) {
    fHitClusterSet->Reset();
    gEve->RemoveElement(fHitClusterSet, fEventManager);
  }

  if(fPadPlane!=NULL)
    fPadPlane->Reset();
}

void
STEveReco::Set2DPlotRange(Int_t uaIdx)
{
  if(uaIdx%100<0 || uaIdx%100>11 || uaIdx/100<0 || uaIdx/100>3) 
  {
    fLogger->Error(MESSAGE_ORIGIN, 
      "2DPlotRange should be ABB ( A = [0, 3], BB = [00, 11] )!");
    return;
  }

  fMinZ = (uaIdx/100)*12*7*4;
  fMaxZ = (uaIdx/100 + 1)*12*7*4;
  fMinX = (uaIdx%100)*8*9 - 432;
  fMaxX = (uaIdx%100 + 1)*8*9 - 432;

  fIs2DPlotRange = kTRUE;
}

void
STEveReco::DrawPadPlane()
{
  if(fPadPlane) 
  {
    fPadPlane->Reset();
    return;
  }

  fCvsPadPlane -> cd();
  fPadPlane = new TH2D("padplane", "", 112, 0, 1344, 108, -432, 432);
  fPadPlane -> GetXaxis() -> SetTickLength(0.01);
  fPadPlane -> GetXaxis() -> SetTitle("z (mm)");
  fPadPlane -> GetXaxis() -> CenterTitle();
  fPadPlane -> GetYaxis() -> SetTickLength(0.01);
  fPadPlane -> GetYaxis() -> SetTitle("x (mm)");
  fPadPlane -> GetYaxis() -> CenterTitle();
  fPadPlane -> SetMinimum(0);
  fPadPlane -> SetMaximum(4095);
  fPadPlane -> SetStats(0);
  fPadPlane -> Draw("colz");

  for (Int_t i = 0; i < 107; i++) {
    Int_t x[2] = {-432 + (i + 1)*8, -432 + (i + 1)*8};
    Int_t z[2] = {0, 1344};
    TGraph *line = new TGraph(2, z, x);
    line -> SetMarkerStyle(1);
    if ((i + 1)%9 == 0)
        line -> SetLineStyle(1);
    else
        line -> SetLineStyle(3);
    line -> Draw("same");
  }

  for (Int_t i = 0; i < 111; i++) {
    Int_t x[2] = {-432, 432};
    Int_t z[2] = {(i + 1)*12, (i + 1)*12};
    TGraph *line = new TGraph(2, z, x);
    line -> SetMarkerStyle(1);
    if ((i + 1)%7 == 0)
        line -> SetLineStyle(1);
    else
        line -> SetLineStyle(3);
    line -> Draw("same");
  }
}

void 
STEveReco::UpdateCvsPadPlane()
{
  fCvsPadPlane -> Modified();
  fCvsPadPlane -> Update();

  TPaletteAxis *paxis 
    = (TPaletteAxis *) fPadPlane->GetListOfFunctions()->FindObject("palette");

  if (paxis) {
    if(fIs2DPlotRange) {
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
