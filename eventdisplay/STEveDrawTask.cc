/**
 * @brief Event display task
 * @author JungWoo Lee (Korea Univ.)
 */

#include "FairRun.h"
#include "STEveDrawTask.hh"
#include "FairRootManager.h"
#include "FairRuntimeDb.h"

#include <iostream>

using namespace std;

ClassImp(STEveDrawTask);

STEveDrawTask* STEveDrawTask::fInstance = 0;
STEveDrawTask* STEveDrawTask::Instance() { return fInstance; }

STEveDrawTask::STEveDrawTask()
{
  fInstance = this;

  for (Int_t i=0; i<fNumEveObject; i++)
  {
    fPointSet[i] = NULL;
    fThresholdMin[i] = 0;
    fThresholdMax[i] = 1.E10;
    fSetObject[i] = kFALSE;
  }

  fEveStyle[kMC] = kFullCircle;
  fEveSize [kMC] = 1;
  fEveColor[kMC] = kGray+2;
  fRnrSelf [kMC] = kFALSE;

  fEveStyle[kDigi] = 1;
  fEveSize [kDigi] = 1.;
  fEveColor[kDigi] = kAzure-5;
  fRnrSelf [kDigi] = kFALSE;

  fEveStyle[kHit] = kFullCircle;
  fEveSize [kHit] = 1.0;
  fEveColor[kHit] = kGray;
  fRnrSelf [kHit] = kFALSE;

  fEveStyle[kHitBox] = kFullCircle;
  fEveSize [kHitBox] = 0.5;
  fEveColor[kHitBox] = kBlue;
  fRnrSelf [kHitBox] = kFALSE;

  fEveStyle[kCluster] = kFullCircle;
  fEveSize [kCluster] = 1.5;
  fEveColor[kCluster] = kBlack;
  fRnrSelf [kCluster] = kFALSE;
  fRnrSelf [kClusterBox] = kFALSE;

  fEveStyle[kRiemannTrack] = 1;
  fEveSize [kRiemannTrack] = 5;
  fEveColor[kRiemannTrack] = -1;
  fRnrSelf [kRiemannTrack] = kFALSE;

  fEveStyle[kRiemannHit] = kFullCircle;
  fEveSize [kRiemannHit] = 1.0;
  fEveColor[kRiemannHit] = -1;
  fRnrSelf [kRiemannHit] = kFALSE;

  fEveStyle[kLinear] = 1;
  fEveSize [kLinear] = 5;
  fEveColor[kLinear] = -1;
  fRnrSelf [kLinear] = kFALSE;

  fEveStyle[kLinearHit] = kFullCircle;
  fEveSize [kLinearHit] = 0.5;
  fEveColor[kLinearHit] = -1;
  fRnrSelf [kLinearHit] = kFALSE;

  fEveStyle[kCurve] = 1;
  fEveSize [kCurve] = 5;
  fEveColor[kCurve] = -1;
  fRnrSelf [kCurve] = kFALSE;

  fEveStyle[kCurveHit] = kFullCircle;
  fEveSize [kCurveHit] = 0.5;
  fEveColor[kCurveHit] = -1;
  fRnrSelf [kCurveHit] = kFALSE;

  fEveStyle[kRecoTrack] = 1;
  fEveSize [kRecoTrack] = 5;
  fEveColor[kRecoTrack] = kRed;
  fRnrSelf [kRecoTrack] = kFALSE;

  fPulse = new STPulse();
  fLTFitter = new STLinearTrackFitter();
  fCTFitter = new STCurveTrackFitter();
  fGenfitTest = new STGenfitTest();

  fRGBAPalette = new TEveRGBAPalette(0, 4096);

  fPulseSum = new TGraph();
  fPulseSum -> SetLineColor(kBlack);
  fPulseSum -> SetLineWidth(2);
}

void STEveDrawTask::DrawADC(Int_t row, Int_t layer)
{
  DrawPad(row, layer);
}

void STEveDrawTask::PushParameters()
{
  fEveManager -> SetNumRiemannSet(fRiemannSetArray.size());
  fEveManager -> SetNumLinearSet(fLinearTrackSetArray.size());
  fEveManager -> SetNumCurveSet(fCurveTrackSetArray.size());
}

void 
STEveDrawTask::SetParContainers()
{
  fLogger -> Debug(MESSAGE_ORIGIN,"SetParContainers of STEveDrawTask");

  FairRun* run = FairRun::Instance();
  FairRuntimeDb* rtdb = run -> GetRuntimeDb();
  fPar = (STDigiPar*) rtdb -> getContainer("STDigiPar");
}

InitStatus 
STEveDrawTask::Init()
{
  fLogger -> Debug(MESSAGE_ORIGIN,"Init()");

  FairRootManager* ioMan = FairRootManager::Instance();
  fEveManager = STEveManager::Instance();

  fMCHitArray           = (TClonesArray*) ioMan -> GetObject("STMCPoint");
  fDriftedElectronArray = (TClonesArray*) ioMan -> GetObject("STDriftedElectron");
  fEventArray           = (TClonesArray*) ioMan -> GetObject("STEvent");
  fRiemannTrackArray    = (TClonesArray*) ioMan -> GetObject("STRiemannTrack");
  fLinearTrackArray     = (TClonesArray*) ioMan -> GetObject("STLinearTrack");
  fCurveTrackArray      = (TClonesArray*) ioMan -> GetObject("STCurveTrack");
  fRawEventArray        = (TClonesArray*) ioMan -> GetObject("STRawEvent");
  fRecoTrackArray       = (TClonesArray*) ioMan -> GetObject("STTrack");

  fHitArray             = (TClonesArray*) ioMan -> GetObject("STHit");
  fHitClusterArray      = (TClonesArray*) ioMan -> GetObject("STHitCluster");

  //gStyle -> SetPalette(0);
  //gStyle -> SetPalette(kInvertedDarkBodyRadiator);
  fCvsPadPlane = fEveManager -> GetCvsPadPlane();
  if (fCvsPadPlane != NULL)
    fCvsPadPlane -> AddExec("ex", "STEveDrawTask::ClickSelectedPadPlane()");
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

  TString trackingParName = fPar -> GetTrackingParFileName();
  STParReader *trackingPar = new STParReader(trackingParName);

  fNumHitsAtHead = trackingPar -> GetIntPar("NumHitsAtHead");

  fCvsPad = fEveManager -> GetCvsPad();
  SetHistPad();

  return kSUCCESS;
}

void 
STEveDrawTask::Exec(Option_t* option)
{
  fLogger -> Debug(MESSAGE_ORIGIN,"Exec()");

  Reset();

  if (fEventArray != NULL)
    fEvent = (STEvent*) fEventArray -> At(0);

  if (fMCHitArray != NULL && fSetObject[kMC]) 
    DrawMCPoints();

  if (fDriftedElectronArray != NULL && fSetObject[kDigi]) 
    DrawDriftedElectrons();

  if (fSetObject[kHit] || fSetObject[kHitBox])
    DrawHitPoints();

  if (fSetObject[kCluster] || fSetObject[kClusterBox])
    DrawHitClusterPoints();

  if (fRiemannTrackArray != NULL && (fSetObject[kRiemannHit] || fSetObject[kRiemannTrack]))
    DrawRiemannHits();

  if (fLinearTrackArray != NULL && (fSetObject[kLinear] || fSetObject[kLinearHit]))
    DrawLinearTracks();

  if (fCurveTrackArray != NULL && (fSetObject[kCurve] || fSetObject[kCurveHit]))
    DrawCurveTracks();

  if (fRecoTrackArray != NULL && (fSetObject[kRecoTrack]))
    DrawRecoTracks();

  gEve -> Redraw3D();

  UpdateCvsPadPlane();
  DrawPad(fCurrentRow, fCurrentLayer, kTRUE);
}

void 
STEveDrawTask::DrawMCPoints()
{
  fLogger -> Debug(MESSAGE_ORIGIN,"Draw MC Points");

  Int_t nPoints = fMCHitArray -> GetEntries();

  if (fPointSet[kMC] == NULL)
  {
    fPointSet[kMC] = new TEvePointSet("MC", nPoints);
    gEve -> AddElement(fPointSet[kMC]);
  }

  fPointSet[kMC] -> SetOwnIds(kTRUE);
  fPointSet[kMC] -> SetMarkerColor(fEveColor[kMC]);
  fPointSet[kMC] -> SetMarkerSize(fEveSize[kMC]);
  fPointSet[kMC] -> SetMarkerStyle(fEveStyle[kMC]);
  fPointSet[kMC] -> SetRnrSelf(fRnrSelf[kMC]);

  for (Int_t iPoint=0; iPoint<nPoints; iPoint++)
  {
    STMCPoint *point = (STMCPoint*) fMCHitArray -> At(iPoint);
              
    fPointSet[kMC] -> SetNextPoint(point -> GetX(), 
                                   point -> GetY(),
                                   point -> GetZ());
  }

  if (nPoints > 0)
    gEve -> ElementChanged(fPointSet[kMC]);
}

void 
STEveDrawTask::DrawDriftedElectrons()
{
  fLogger -> Debug(MESSAGE_ORIGIN,"Draw Drifted Electrons");

  Int_t nPoints = fDriftedElectronArray -> GetEntries();

  if (fPointSet[kDigi] == NULL)
  {
    fPointSet[kDigi] = new TEvePointSet("DriftedElectron", nPoints);
    gEve -> AddElement(fPointSet[kDigi]);
  }

  fPointSet[kDigi] -> SetOwnIds(kTRUE);
  fPointSet[kDigi] -> SetMarkerColor(fEveColor[kDigi]);
  fPointSet[kDigi] -> SetMarkerSize(fEveSize[kDigi]);
  fPointSet[kDigi] -> SetMarkerStyle(fEveStyle[kDigi]);
  fPointSet[kDigi] -> SetRnrSelf(fRnrSelf[kDigi]);

  for(Int_t iPoint=0; iPoint<nPoints; iPoint++)
  {
    STDriftedElectron *electron = (STDriftedElectron*) fDriftedElectronArray -> At(iPoint);

    Double_t x = electron -> GetX() + electron -> GetDiffusedX();
    Double_t y = electron -> GetHitY() + fDriftVelocity*(electron -> GetDiffusedTime() + electron -> GetDriftTime());
    Double_t z = electron -> GetZ() + electron -> GetDiffusedZ();

    fPointSet[kDigi] -> SetNextPoint(x/10., y/10., z/10.);
  }

  if (nPoints > 0)
    gEve -> ElementChanged(fPointSet[kDigi]);
}

void 
STEveDrawTask::DrawHitPoints()
{
  fLogger -> Debug(MESSAGE_ORIGIN,"DrawHitPoints()");

  Int_t nHits = 0;

  Bool_t useEvent = true;
  if (fEvent != NULL) {
    nHits = fEvent -> GetNumHits();
    useEvent = true;
  }
  else if (fHitArray != nullptr) {
    nHits = fHitArray -> GetEntries();
    useEvent = false;
  }
  else
    return;

  if (fSetObject[kHit]) 
  {
    if (fPointSet[kHit] == NULL)
    {
      fPointSet[kHit] = new TEvePointSet("Hit", nHits);
      gEve -> AddElement(fPointSet[kHit]);
    }

    fPointSet[kHit] -> SetOwnIds(kTRUE);
    fPointSet[kHit] -> SetMarkerColor(fEveColor[kHit]);
    fPointSet[kHit] -> SetMarkerSize(fEveSize[kHit]);
    fPointSet[kHit] -> SetMarkerStyle(fEveStyle[kHit]);
    fPointSet[kHit] -> SetMainTransparency(50);
    fPointSet[kHit] -> SetRnrSelf(fRnrSelf[kHit]);
  }

  if (fSetObject[kHitBox]) 
  {
    if (fBoxHitSet == NULL)
    {
      fBoxHitSet = new TEveBoxSet();
      gEve -> AddElement(fBoxHitSet);
    }

    fBoxHitSet -> Reset(TEveBoxSet::kBT_AABoxFixedDim, kFALSE, 32);
    fBoxHitSet -> SetOwnIds(kTRUE);
    fBoxHitSet -> SetAntiFlick(kTRUE);
    fBoxHitSet -> SetPalette(fRGBAPalette);
    fBoxHitSet -> SetDefDepth(1.2);
    fBoxHitSet -> SetDefHeight(0.5);
    fBoxHitSet -> SetDefWidth(0.8);
    fBoxHitSet -> RefitPlex();
    fBoxHitSet -> SetRnrSelf(fRnrSelf[kHitBox]);
  }

  for (Int_t iHit=0; iHit<nHits; iHit++)
  {
    STHit *hit;

    if (useEvent)
      hit = fEvent -> GetHit(iHit);
    else
      hit = (STHit *) fHitArray -> At(iHit);

    if (hit -> GetCharge() < fThresholdMin[kHit] || 
        hit -> GetCharge() > fThresholdMax[kHit])
      continue;

    TVector3 position = hit -> GetPosition();

    if (hit -> GetTb() < fWindowTbStart || hit -> GetTb() > fWindowTbEnd)
      continue;

    Double_t y = position.Y()/10.;
    y += fWindowYStart;

    if (fSetObject[kHit]) 
    {
      fPointSet[kHit] -> SetNextPoint(position.X()/10., y, position.Z()/10.);
    }

    if (fPadPlane != NULL)
      fPadPlane -> Fill(-position.X(), position.Z(), hit -> GetCharge());

    if (fSetObject[kHitBox]) 
    {
      fBoxHitSet -> AddBox(position.X()/10. - fBoxHitSet -> GetDefWidth()/2., 
                           y - fBoxHitSet -> GetDefHeight()/2., 
                           position.Z()/10. - fBoxHitSet -> GetDefDepth()/2.);

      fBoxHitSet -> DigitValue(hit -> GetCharge());
    }
  }

  if (nHits > 0)
  {
    if (fSetObject[kHit]) 
      gEve -> ElementChanged(fPointSet[kHit]);

    if (fSetObject[kHitBox]) 
      gEve -> ElementChanged(fBoxHitSet);
  }
}

void 
STEveDrawTask::DrawHitClusterPoints()
{
  fLogger -> Debug(MESSAGE_ORIGIN,"DrawHitClusterPoints()");

  Int_t nClusters = 0;

  Bool_t useEvent = true;
  if (fEvent != NULL) {
    nClusters = fEvent -> GetNumClusters();
    useEvent = true;
  }
  else if (fHitClusterArray != nullptr) {
    nClusters = fHitClusterArray -> GetEntries();
    useEvent = false;
  }
  else
    return;

  if (fSetObject[kClusterBox]) {
    if (fBoxClusterSet == NULL)
    {
      fBoxClusterSet = new TEveBoxSet("BoxCluster");
      gEve -> AddElement(fBoxClusterSet);
    }

    fBoxClusterSet -> UseSingleColor();
    fBoxClusterSet -> SetMainColor(kBlue);
    fBoxClusterSet -> SetMainTransparency(30);
    fBoxClusterSet -> Reset(TEveBoxSet::kBT_AABox, kFALSE, 64);
    fBoxClusterSet -> SetRnrSelf(fRnrSelf[kClusterBox]);
  }

  if (fSetObject[kCluster]) {
    if (fPointSet[kCluster] == NULL)
    {
      fPointSet[kCluster] = new TEvePointSet("HitCluster", nClusters);
      gEve -> AddElement(fPointSet[kCluster]);
    }

    fPointSet[kCluster] -> SetOwnIds(kTRUE);
    fPointSet[kCluster] -> SetMarkerColor(fEveColor[kCluster]);
    fPointSet[kCluster] -> SetMarkerSize(fEveSize[kCluster]);
    fPointSet[kCluster] -> SetMarkerStyle(fEveStyle[kCluster]);
    fPointSet[kCluster] -> SetRnrSelf(fRnrSelf[kCluster]);
  }

  fLogger -> Debug(MESSAGE_ORIGIN,Form("Number of clusters: %d",nClusters));
  for (Int_t iCluster=0; iCluster<nClusters; iCluster++)
  {
    STHitCluster *cluster;

    if (useEvent)
      cluster = fEvent -> GetCluster(iCluster);
    if (useEvent)
      cluster = (STHitCluster *) fHitClusterArray -> At(iCluster);

    if (cluster -> GetCharge() < fThresholdMin[kCluster] || 
        cluster -> GetCharge() > fThresholdMax[kCluster])
      continue;

    TVector3 position = cluster -> GetPosition();

    Double_t y = position.Y()/10.;
    if (y > -fWindowYStart || y < -fWindowYEnd)
      continue;
    y += fWindowYStart;

    TVector3 sigma = cluster -> GetPosSigma();

    if (fSetObject[kCluster]) 
    {
      fPointSet[kCluster] -> SetNextPoint(position.X()/10.,
                                          y,
                                          position.Z()/10.);
    }

    if (fSetObject[kClusterBox]) 
    {
      Double_t xS =  sigma.X()/10.;
      Double_t yS =  sigma.Y()/10.;
      Double_t zS =  sigma.Z()/10.;

      Double_t xP =  position.X()/10. - xS/2.;
      Double_t yP =                 y - yS/2.;
      Double_t zP =  position.Z()/10. - zS/2.;

      fBoxClusterSet -> AddBox(xP, yP, zP, xS, yS, zS);
      fBoxClusterSet -> DigitValue(cluster -> GetCharge());
    }
  }

  if (nClusters > 0)
  {
    if (fSetObject[kCluster]) 
      gEve -> ElementChanged(fPointSet[kCluster]);

    if (fSetObject[kClusterBox]) 
      gEve -> ElementChanged(fBoxClusterSet);
  }
}

void 
STEveDrawTask::DrawRiemannHits()
{
  fLogger -> Debug(MESSAGE_ORIGIN,"DrawRiemannHits()");

  STRiemannTrack* track = NULL;
  STHit* rHit = NULL;
  TEvePointSet* riemannPointSet = NULL;
  TEveLine* riemannTrackLine = NULL;

  if (fEvent == NULL && fHitClusterArray == nullptr) {
    fLogger -> Debug(MESSAGE_ORIGIN, "STEvent is needed for riemann track!");
    return;
  }

  if (fSetObject[kRiemannHit]) 
  {
    Int_t nTracks = fRiemannTrackArray -> GetEntries();
    Int_t nTracksInBuffer = fRiemannSetArray.size();

    if (nTracksInBuffer < nTracks)
    {
      Int_t diffNumTracks = nTracks - nTracksInBuffer;
      for (Int_t iTrack = 0; iTrack < diffNumTracks; iTrack++)
      {
        Int_t idxTrack = iTrack + nTracksInBuffer;
        riemannPointSet = new TEvePointSet(Form("RiemannTrackHits_%d", idxTrack), 1, TEvePointSelectorConsumer::kTVT_XYZ);
        if (fEveColor[kRiemannHit] == -1) riemannPointSet -> SetMarkerColor(GetColor(idxTrack));
        else riemannPointSet -> SetMarkerColor(fEveColor[kRiemannHit]);
        riemannPointSet -> SetMarkerSize(fEveSize[kRiemannHit]);
        riemannPointSet -> SetMarkerStyle(fEveStyle[kRiemannHit]);
        riemannPointSet -> SetRnrSelf(fRnrSelf[kRiemannHit]);
        fRiemannSetArray.push_back(riemannPointSet);
        gEve -> AddElement(riemannPointSet);
      }
    }

    for (Int_t iTrack = 0; iTrack < nTracks; iTrack++) 
    {
      track = (STRiemannTrack*) fRiemannTrackArray -> At(iTrack);
      Int_t nClusters = track -> GetNumHits();

      riemannPointSet = fRiemannSetArray.at(iTrack);

      for (Int_t iCluster = 0; iCluster < nClusters; iCluster++)
      {
        rHit = track -> GetHit(iCluster) -> GetHit();

        if (rHit -> GetCharge() < fThresholdMin[kRiemannHit] || 
            rHit -> GetCharge() > fThresholdMax[kRiemannHit])
          continue;

        Int_t id = rHit -> GetClusterID();
        STHitCluster oCluster;
        if (fEvent != NULL)
          oCluster = fEvent -> GetClusterArray() -> at(id);
        else
          oCluster = (STHitCluster *) fHitClusterArray -> At(id);

        TVector3 position = oCluster.GetPosition();

        Double_t y = position.Y()/10.;
        if (y > -fWindowYStart || y < -fWindowYEnd)
          continue;
        y += fWindowYStart;

        riemannPointSet -> SetNextPoint(position.X()/10.,
                                        y,
                                        position.Z()/10.);
      }
      riemannPointSet -> SetRnrSelf(fRnrSelf[kRiemannHit]);
    }
  }

  if (fSetObject[kRiemannTrack]) 
  {
    Int_t nTracks = fRiemannTrackArray -> GetEntries();
    Int_t nTracksInBuffer = fRiemannTrackSetArray.size();

    if (nTracksInBuffer < nTracks)
    {
      Int_t diffNumTracks = nTracks - nTracksInBuffer;
      for (Int_t iTrack = 0; iTrack < diffNumTracks; iTrack++)
      {
        Int_t idxTrack = iTrack + nTracksInBuffer;
        riemannTrackLine = new TEveLine(Form("RiemannTrackLine_%d", idxTrack));
        if (fEveColor[kRiemannTrack] == -1) riemannTrackLine -> SetMarkerColor(GetColor(idxTrack));
        else riemannTrackLine -> SetMarkerColor(fEveColor[kRiemannTrack]);
        riemannTrackLine -> SetMarkerSize(fEveSize[kRiemannTrack]);
        riemannTrackLine -> SetMarkerStyle(fEveStyle[kRiemannTrack]);
        riemannTrackLine -> SetRnrSelf(fRnrSelf[kRiemannTrack]);
        fRiemannTrackSetArray.push_back(riemannTrackLine);
        gEve -> AddElement(riemannTrackLine);
      }
    }

    for (Int_t iTrack = 0; iTrack < nTracks; iTrack++) 
    {
      track = (STRiemannTrack*) fRiemannTrackArray -> At(iTrack);
      Int_t nClusters = track -> GetNumHits();

      riemannTrackLine = fRiemannTrackSetArray.at(iTrack);

      Int_t id = 0;
      STHitCluster oCluster;
      if (fEvent != NULL)
        oCluster = fEvent -> GetClusterArray() -> at(id);
      else
        oCluster = (STHitCluster *) fHitClusterArray -> At(id);

      TVector3 position = oCluster.GetPosition();

      Double_t y = position.Y()/10.;
      if (y > -fWindowYStart || y < -fWindowYEnd)
        continue;
      y += fWindowYStart;

      riemannTrackLine -> SetNextPoint(position.X()/10.,
                                       y,
                                       position.Z()/10.);
      //while (1)
      //{
      //}

      riemannTrackLine -> SetRnrSelf(fRnrSelf[kRiemannTrack]);
    }
  }
}

void
STEveDrawTask::DrawLinearTracks()
{
  fLogger -> Debug(MESSAGE_ORIGIN,"Draw Linear Tracks");

  STLinearTrack *track = (STLinearTrack*) fLinearTrackArray -> At(0);
  std::vector<STHit*> *hitPointerArray = track -> GetHitPointerArray();

  if ((fEvent == NULL) && (hitPointerArray -> size() == 0))
    return;

  STHit* hit = NULL;

  TEveLine* linearTrackLine = NULL;
  TEvePointSet* linearPointSet = NULL;

  Int_t nTracks = fLinearTrackArray -> GetEntries();
  Int_t nTracksInBuffer = fLinearTrackSetArray.size();

  if (nTracksInBuffer < nTracks)
  {
    Int_t diffNumTracks = nTracks - nTracksInBuffer;
    for (Int_t iTrack = 0; iTrack < diffNumTracks; iTrack++)
    {
      Int_t idxTrack = iTrack + nTracksInBuffer;
      linearTrackLine = new TEveLine(Form("LinearLine_%d", idxTrack), 2, TEvePointSelectorConsumer::kTVT_XYZ);
      if (fEveColor[kLinear] == -1) linearTrackLine -> SetLineColor(GetColor(idxTrack));
      else linearTrackLine -> SetLineColor(fEveColor[kLinear]);
      linearTrackLine -> SetLineStyle(fEveStyle[kLinear]);
      linearTrackLine -> SetLineWidth(fEveSize[kLinear]);
      fLinearTrackSetArray.push_back(linearTrackLine);
      gEve -> AddElement(linearTrackLine);

      if (fSetObject[kLinearHit] == kTRUE)
      {
        linearPointSet = new TEvePointSet(Form("LinearHit_%d", idxTrack), 1, TEvePointSelectorConsumer::kTVT_XYZ);
        if (fEveColor[kLinearHit] == -1) linearPointSet -> SetMarkerColor(GetColor(idxTrack));
        else linearPointSet -> SetMarkerColor(fEveColor[kLinearHit]);
        linearPointSet -> SetMarkerSize(fEveSize[kLinearHit]);
        linearPointSet -> SetMarkerStyle(fEveStyle[kLinearHit]);
        fLinearHitSetArray.push_back(linearPointSet);
        gEve -> AddElement(linearPointSet);
      }
    }
  }

  for (Int_t iTrack = 0; iTrack < nTracks; iTrack++)
  {
    track = (STLinearTrack*) fLinearTrackArray -> At(iTrack);
    Int_t nHits = track -> GetNumHits();

    Int_t hitIDFirst = track -> GetHitID(0);
    Int_t hitIDLast  = track -> GetHitID(track -> GetNumHits()-1);

    STHit *hitFirst;
    STHit *hitLast;

    if (fEvent != NULL)
    {
      hitFirst = fEvent -> GetHit(hitIDFirst);
      hitLast  = fEvent -> GetHit(hitIDLast);
    }
    else
    {
      hitFirst = track -> GetHit(hitIDFirst);
      hitLast  = track -> GetHit(hitIDLast);
    }

    TVector3 posFirst = fLTFitter -> GetClosestPointOnTrack(track, hitFirst);
    TVector3 posLast  = fLTFitter -> GetClosestPointOnTrack(track, hitLast);

    Double_t yFirst = posFirst.Y()/10.;
    yFirst += fWindowYStart;
    Double_t yLast = posLast.Y()/10.;
    yLast += fWindowYStart;

    linearTrackLine = fLinearTrackSetArray.at(iTrack);

    linearTrackLine -> SetNextPoint(posFirst.X()/10., yFirst, posFirst.Z()/10.);
    linearTrackLine -> SetNextPoint(posLast.X()/10., yLast, posLast.Z()/10.);

    linearTrackLine -> SetRnrSelf(fRnrSelf[kLinear]);

    if (fSetObject[kLinearHit] == kTRUE)
    {
      linearPointSet = fLinearHitSetArray.at(iTrack);

      for (Int_t iHit = 0; iHit < nHits; iHit++)
      {
        if (fEvent != NULL)
          hit = fEvent -> GetHit(track -> GetHitID(iHit));
        else
          hit = track -> GetHit(iHit);

        TVector3 position = hit -> GetPosition();

        linearPointSet -> SetNextPoint(position.X()/10.,
                                       position.Y()/10. + fWindowYStart,
                                       position.Z()/10.);
      }

      linearPointSet -> SetRnrSelf(fRnrSelf[kLinearHit]);
    }
  }
}

void
STEveDrawTask::DrawCurveTracks()
{
  fLogger -> Debug(MESSAGE_ORIGIN,"Draw Curve Tracks");


  STCurveTrack *track = (STCurveTrack*) fCurveTrackArray -> At(0);
  std::vector<STHit*> *hitPointerArray = track -> GetHitPointerArray();

  Bool_t useTrackData = true;
  if (hitPointerArray -> size() == 0)
    useTrackData = false;

  Bool_t useEvent = true;
  if (fEvent != NULL)
    useEvent = true;
  else if (fHitArray != nullptr)
    useEvent = false;
  else
    return;

  STHit* hit = NULL;

  TEveLine* curveTrackLine = NULL;
  TEvePointSet* curvePointSet = NULL;

  Int_t nTracks = fCurveTrackArray -> GetEntries();
  Int_t nTracksInBuffer = fCurveTrackSetArray.size();

  if (nTracksInBuffer < nTracks)
  {
    Int_t diffNumTracks = nTracks - nTracksInBuffer;
    for (Int_t iTrack = 0; iTrack < diffNumTracks; iTrack++)
    {
      Int_t idxTrack = iTrack + nTracksInBuffer;
      curveTrackLine = new TEveLine(Form("CurveLine_%d", idxTrack), 2, TEvePointSelectorConsumer::kTVT_XYZ);
      if (fEveColor[kCurve] == -1) curveTrackLine -> SetLineColor(GetColor(idxTrack));
      else curveTrackLine -> SetLineColor(fEveColor[kCurve]);
      curveTrackLine -> SetLineStyle(fEveStyle[kCurve]);
      curveTrackLine -> SetLineWidth(fEveSize[kCurve]);
      fCurveTrackSetArray.push_back(curveTrackLine);
      gEve -> AddElement(curveTrackLine);

      if (fSetObject[kCurveHit] == kTRUE)
      {
        curvePointSet = new TEvePointSet(Form("CurveHit_%d", idxTrack), 1, TEvePointSelectorConsumer::kTVT_XYZ);
        if (fEveColor[kCurveHit] == -1) curvePointSet -> SetMarkerColor(GetColor(idxTrack));
        else curvePointSet -> SetMarkerColor(fEveColor[kCurveHit]);
        curvePointSet -> SetMarkerSize(fEveSize[kCurveHit]);
        curvePointSet -> SetMarkerStyle(fEveStyle[kCurveHit]);
        fCurveHitSetArray.push_back(curvePointSet);
        gEve -> AddElement(curvePointSet);
      }
    }
  }

  for (Int_t iTrack = 0; iTrack < nTracks; iTrack++)
  {
    track = (STCurveTrack*) fCurveTrackArray -> At(iTrack);
    Int_t nHits = track -> GetNumHits();

    if (track -> IsFitted() == kTRUE)
    {
      Int_t firstHitIDPosition = 0;
      Int_t lastHitIDPosition = track -> GetNumHits() - 1;

      if (lastHitIDPosition > fNumHitsAtHead)
        firstHitIDPosition = lastHitIDPosition - fNumHitsAtHead;

      Int_t hitIDFirst = track -> GetHitID(firstHitIDPosition);
      Int_t hitIDLast  = track -> GetHitID(lastHitIDPosition);

      STHit *hitFirst;
      STHit *hitLast;

      if (useTrackData)
      {
        hitFirst = track -> GetHit(firstHitIDPosition);
        hitLast  = track -> GetHit(lastHitIDPosition);
      }
      else if (useEvent)
      {
        hitFirst = fEvent -> GetHit(hitIDFirst);
        hitLast  = fEvent -> GetHit(hitIDLast);
      }
      else
      {
        hitFirst = (STHit *) fHitArray -> At(hitIDFirst);
        hitLast  = (STHit *) fHitArray -> At(hitIDLast);
      }

      TVector3 posFirst = fLTFitter -> GetClosestPointOnTrack(track, hitFirst);
      TVector3 posLast  = fLTFitter -> GetClosestPointOnTrack(track, hitLast);

      Double_t yFirst = posFirst.Y()/10.;
      yFirst += fWindowYStart;
      Double_t yLast = posLast.Y()/10.;
      yLast += fWindowYStart;

      curveTrackLine = fCurveTrackSetArray.at(iTrack);
      curveTrackLine -> SetNextPoint(posFirst.X()/10., yFirst, posFirst.Z()/10.);
      curveTrackLine -> SetNextPoint(posLast.X()/10., yLast, posLast.Z()/10.);
      curveTrackLine -> SetRnrSelf(fRnrSelf[kCurve]);
    }

    if (fSetObject[kCurveHit] == kTRUE)
    {
      curvePointSet = fCurveHitSetArray.at(iTrack);

      for (Int_t iHit = 0; iHit < nHits; iHit++)
      {
        if (useTrackData)
          hit = track -> GetHit(iHit);
        else if (useEvent)
          hit = fEvent -> GetHit(track -> GetHitID(iHit));
        else
          hit = (STHit *) fHitArray -> At(iHit);

        TVector3 position = hit -> GetPosition();

        curvePointSet -> SetNextPoint(position.X()/10.,
                                       position.Y()/10. + fWindowYStart,
                                       position.Z()/10.);
      }

      curvePointSet -> SetRnrSelf(fRnrSelf[kCurveHit]);
    }
  }
}

void
STEveDrawTask::DrawRecoTracks()
{
  fLogger -> Debug(MESSAGE_ORIGIN,"Draw Reco Tracks");

  if (fEvent == NULL && fHitClusterArray == nullptr)
    return;

  STTrack* track = NULL;
  TEveLine* recoTrackLine = NULL;

  Int_t nTracks = fRecoTrackArray -> GetEntries();
  Int_t nTracksInBuffer = fRecoTrackSetArray.size();

  if (nTracksInBuffer < nTracks)
  {
    Int_t diffNumTracks = nTracks - nTracksInBuffer;
    for (Int_t iTrack = 0; iTrack < diffNumTracks; iTrack++)
    {
      Int_t idxTrack = iTrack + nTracksInBuffer;
      recoTrackLine = new TEveLine(Form("RecoTrack_%d", idxTrack));
      if (fEveColor[kRecoTrack] == -1) recoTrackLine -> SetLineColor(GetColor(idxTrack));
      else recoTrackLine -> SetLineColor(fEveColor[kRecoTrack]);
      recoTrackLine -> SetMarkerSize(fEveSize[kRecoTrack]);
      recoTrackLine -> SetMarkerStyle(fEveStyle[kRecoTrack]);
      recoTrackLine -> SetRnrSelf(fRnrSelf[kRecoTrack]);
      fRecoTrackSetArray.push_back(recoTrackLine);
      gEve -> AddElement(recoTrackLine);
    }
  }

  for (Int_t iTrack = 0; iTrack < nTracks; iTrack++) 
  //for (Int_t iTrack = 0; iTrack < 1; iTrack++) 
  {
    track = (STTrack*) fRecoTrackArray -> At(iTrack);

    if (track -> GetNDF() < 5)
      continue;

    recoTrackLine = fRecoTrackSetArray.at(iTrack);

    Bool_t isFitted = track -> IsFitted();
    if (!isFitted) 
      continue;

    if (fEvent != NULL)
      isFitted = fGenfitTest -> SetTrack(fEvent, track);
    else
      isFitted = fGenfitTest -> SetTrack(fHitClusterArray, track);
    if (!isFitted) 
      continue;

    for (Int_t i = 0; i < 100; i++) {
      TVector3 position;
      Bool_t isExtrapolated = fGenfitTest -> ExtrapolateTrack(i, position);
      if (!isExtrapolated)
        break;

      //position = 0.1 * position;

      Double_t y = position.Y();
      if (y > -fWindowYStart || y < -fWindowYEnd)
        continue;
      y += fWindowYStart;

      if (position.X() < -80 || position.X() > 80 || y > 5 || y < -55 || position.Z() < -10 || position.Z() > 150)
        break;

      recoTrackLine -> SetNextPoint(position.X(), y, position.Z());
    }

    recoTrackLine -> SetRnrSelf(fRnrSelf[kRecoTrack]);
  }
}

void 
STEveDrawTask::SetSelfRiemannSet(Int_t iRiemannSet, Bool_t offElse)
{
  Int_t nRiemannSets = fRiemannSetArray.size();
  Int_t nRiemannTrackLineSets = fRiemannTrackSetArray.size();

  if (iRiemannSet == -1) 
  {
    if (!offElse)
    {
      for (Int_t i=0; i<nRiemannSets; i++)
      {
        TEvePointSet* riemannPointSet = fRiemannSetArray.at(i);
        riemannPointSet -> SetRnrSelf(kTRUE);
      }
      for (Int_t i=0; i<nRiemannTrackLineSets; i++)
      {
        TEveLine* riemannTrackLine = fRiemannTrackSetArray.at(i);
        riemannTrackLine -> SetRnrSelf(kTRUE);
      }
    }
    else
    {
      for (Int_t i=0; i<nRiemannSets; i++)
      {
        TEvePointSet* riemannPointSet = fRiemannSetArray.at(i);
        riemannPointSet -> SetRnrSelf(kFALSE);
      }
      for (Int_t i=0; i<nRiemannTrackLineSets; i++)
      {
        TEveLine* riemannTrackLine = fRiemannTrackSetArray.at(i);
        riemannTrackLine -> SetRnrSelf(kFALSE);
      }
    }
  }

  else 
  {
    for (Int_t i=0; i<nRiemannSets; i++)
    {
      TEvePointSet* riemannPointSet = fRiemannSetArray.at(i);
      if (i==iRiemannSet) riemannPointSet -> SetRnrSelf(kTRUE);
      else if (offElse) riemannPointSet -> SetRnrSelf(kFALSE);
    }
    for (Int_t i=0; i<nRiemannTrackLineSets; i++)
    {
      TEveLine* riemannTrackLine = fRiemannTrackSetArray.at(i);
      if (i==iRiemannSet) riemannTrackLine -> SetRnrSelf(kTRUE);
      else if (offElse) riemannTrackLine -> SetRnrSelf(kFALSE);
    }
  }
}

void 
STEveDrawTask::SetSelfLinearSet(Int_t iLinearSet, Bool_t offElse)
{
  Int_t nLinearTrackSets = fLinearTrackSetArray.size();
  Int_t nLinearHitSets = fLinearHitSetArray.size();

  if (iLinearSet == -1) 
  {
    if (!offElse)
    {
      for (Int_t i=0; i<nLinearHitSets; i++)
      {
        TEvePointSet* pointSet = fLinearHitSetArray.at(i);
        pointSet -> SetRnrSelf(fRnrSelf[kLinearHit]);
      }
      for (Int_t i=0; i<nLinearTrackSets; i++)
      {
        TEveLine* linearTrackLine = fLinearTrackSetArray.at(i);
        linearTrackLine -> SetRnrSelf(kTRUE);
      }
    }
    else
    {
      for (Int_t i=0; i<nLinearHitSets; i++)
      {
        TEvePointSet* pointSet = fLinearHitSetArray.at(i);
        pointSet -> SetRnrSelf(kFALSE);
      }
      for (Int_t i=0; i<nLinearTrackSets; i++)
      {
        TEveLine* linearTrackLine = fLinearTrackSetArray.at(i);
        linearTrackLine -> SetRnrSelf(kFALSE);
      }
    }
  }

  else 
  {
    for (Int_t i=0; i<nLinearHitSets; i++)
    {
      TEvePointSet* pointSet = fLinearHitSetArray.at(i);

      if (i==iLinearSet) 
        pointSet -> SetRnrSelf(fRnrSelf[kLinearHit]);
      else if (offElse) 
        pointSet -> SetRnrSelf(kFALSE);
    }

    for (Int_t i=0; i<nLinearTrackSets; i++)
    {
      TEveLine* linearTrackLine = fLinearTrackSetArray.at(i);

      if (i==iLinearSet) 
        linearTrackLine -> SetRnrSelf(kTRUE);
      else if (offElse) 
        linearTrackLine -> SetRnrSelf(kFALSE);
    }
  }
}

void 
STEveDrawTask::SetSelfCurveSet(Int_t iCurveSet, Bool_t offElse)
{
  Int_t nCurveTrackSets = fCurveTrackSetArray.size();
  Int_t nCurveHitSets = fCurveHitSetArray.size();

  if (iCurveSet == -1) 
  {
    if (!offElse)
    {
      for (Int_t i=0; i<nCurveHitSets; i++)
      {
        TEvePointSet* pointSet = fCurveHitSetArray.at(i);
        pointSet -> SetRnrSelf(fRnrSelf[kCurveHit]);
      }
      for (Int_t i=0; i<nCurveTrackSets; i++)
      {
        TEveLine* curveTrackLine = fCurveTrackSetArray.at(i);
        curveTrackLine -> SetRnrSelf(kTRUE);
      }
    }
    else
    {
      for (Int_t i=0; i<nCurveHitSets; i++)
      {
        TEvePointSet* pointSet = fCurveHitSetArray.at(i);
        pointSet -> SetRnrSelf(kFALSE);
      }
      for (Int_t i=0; i<nCurveTrackSets; i++)
      {
        TEveLine* curveTrackLine = fCurveTrackSetArray.at(i);
        curveTrackLine -> SetRnrSelf(kFALSE);
      }
    }
  }

  else 
  {
    for (Int_t i=0; i<nCurveHitSets; i++)
    {
      TEvePointSet* pointSet = fCurveHitSetArray.at(i);

      if (i==iCurveSet) 
        pointSet -> SetRnrSelf(fRnrSelf[kCurveHit]);
      else if (offElse) 
        pointSet -> SetRnrSelf(kFALSE);
    }

    for (Int_t i=0; i<nCurveTrackSets; i++)
    {
      TEveLine* curveTrackLine = fCurveTrackSetArray.at(i);

      if (i==iCurveSet) 
        curveTrackLine -> SetRnrSelf(kTRUE);
      else if (offElse) 
        curveTrackLine -> SetRnrSelf(kFALSE);
    }
  }
}

void 
STEveDrawTask::SetSelfRecoTrackSet(Int_t iRecoTrackSet, Bool_t offElse)
{
  Int_t nRecoTrackSets = fRecoTrackSetArray.size();

  if (iRecoTrackSet == -1) 
  {
    if (!offElse)
    {
      for (Int_t i=0; i<nRecoTrackSets; i++)
      {
        TEveLine* recoTrackLine = fRecoTrackSetArray.at(i);
        recoTrackLine -> SetRnrSelf(kTRUE);
      }
    }
    else
    {
      for (Int_t i=0; i<nRecoTrackSets; i++)
      {
        TEveLine* recoTrackLine = fRecoTrackSetArray.at(i);
        recoTrackLine -> SetRnrSelf(kFALSE);
      }
    }
  }

  else 
  {
    for (Int_t i=0; i<nRecoTrackSets; i++)
    {
      TEveLine* recoTrackLine = fRecoTrackSetArray.at(i);
      if (i==iRecoTrackSet) recoTrackLine -> SetRnrSelf(kTRUE);
      else if (offElse) recoTrackLine -> SetRnrSelf(kFALSE);
    }
  }
}

void 
STEveDrawTask::SetRendering(TString name, Bool_t rnr, Double_t thresholdMin, Double_t thresholdMax)
{
  STEveObject eveObj = GetEveObject(name);

  if (rnr == kTRUE) fSetObject[eveObj] = kTRUE; 
  fRnrSelf[eveObj] = rnr;

  if (thresholdMin != -1) 
    fThresholdMin[eveObj] = thresholdMin;

  if (thresholdMin != -1) 
    fThresholdMax[eveObj] = thresholdMax;
}


void 
STEveDrawTask::SetAttributes(TString name, Style_t style, Size_t size, Color_t color)
{
  STEveObject eveObj = GetEveObject(name);

  if (style != -1) 
    fEveStyle[eveObj] = style;

  if (size != -1) 
    fEveSize[eveObj] = size;

  if (color != -1) 
    fEveColor[eveObj] = color;
}


void
STEveDrawTask::SetObject(TString name, Bool_t set)
{
  STEveObject eveObj = GetEveObject(name);

  fSetObject[eveObj] = set;
}

void
STEveDrawTask::Reset()
{
  fLogger -> Debug(MESSAGE_ORIGIN,"Reset point set");

  for(Int_t i=0; i<fNumEveObject; i++) 
  {
    if(fPointSet[i]) {
      fPointSet[i] -> Reset();
    }
  }

  fLogger -> Debug(MESSAGE_ORIGIN,"Reset box hit set");

  if (fBoxHitSet) {
    fBoxHitSet -> Reset();
  }

  fLogger -> Debug(MESSAGE_ORIGIN,"Reset box cluster set");

  if (fBoxClusterSet) {
    fBoxClusterSet -> Reset();
  }

  fLogger -> Debug(MESSAGE_ORIGIN,"Reset riemann set");

  Int_t nRiemannTracks = fRiemannSetArray.size();
  for (Int_t i=0; i<nRiemannTracks; i++) 
  {
    TEvePointSet* riemannPointSet = fRiemannSetArray.at(i);
    riemannPointSet -> SetRnrSelf(kFALSE);
    riemannPointSet -> Reset();
  }

  fLogger -> Debug(MESSAGE_ORIGIN,"Reset riemann track line set");

  Int_t nRiemannTrackLines = fRiemannTrackSetArray.size();
  for (Int_t i=0; i<nRiemannTrackLines; i++) 
  {
    TEveLine* riemannTrackLine = fRiemannTrackSetArray.at(i);
    riemannTrackLine -> SetRnrSelf(kFALSE);
    riemannTrackLine -> Reset();
  }

  fLogger -> Debug(MESSAGE_ORIGIN,"Reset linear line set");

  Int_t nLinearTracks = fLinearTrackSetArray.size();
  for (Int_t i=0; i<nLinearTracks; i++) 
  {
    TEveLine* linearTrackLine = fLinearTrackSetArray.at(i);
    linearTrackLine -> SetRnrSelf(kFALSE);
    linearTrackLine -> Reset();
  }

  fLogger -> Debug(MESSAGE_ORIGIN,"Reset linear point set");

  Int_t nLinearPoints = fLinearHitSetArray.size();
  for (Int_t i=0; i<nLinearPoints; i++) 
  {
    TEvePointSet* pointSet = fLinearHitSetArray.at(i);
    pointSet -> SetRnrSelf(kFALSE);
    pointSet -> Reset();
  }

  fLogger -> Debug(MESSAGE_ORIGIN,"Reset curve line set");

  Int_t nCurveTracks = fCurveTrackSetArray.size();
  for (Int_t i=0; i<nCurveTracks; i++) 
  {
    TEveLine* curveTrackLine = fCurveTrackSetArray.at(i);
    curveTrackLine -> SetRnrSelf(kFALSE);
    curveTrackLine -> Reset();
  }

  fLogger -> Debug(MESSAGE_ORIGIN,"Reset curve point set");

  Int_t nCurvePoints = fCurveHitSetArray.size();
  for (Int_t i=0; i<nCurvePoints; i++) 
  {
    TEvePointSet* pointSet = fCurveHitSetArray.at(i);
    pointSet -> SetRnrSelf(kFALSE);
    pointSet -> Reset();
  }

  fLogger -> Debug(MESSAGE_ORIGIN,"Reset reco track line set");

  Int_t nRecoTrackLines = fRecoTrackSetArray.size();
  for (Int_t i=0; i<nRecoTrackLines; i++) 
  {
    TEveLine* recoTrackLine = fRecoTrackSetArray.at(i);
    recoTrackLine -> SetRnrSelf(kFALSE);
    recoTrackLine -> Reset();
  }

  if (fPadPlane != NULL)
    fPadPlane -> Reset("");
}

void
STEveDrawTask::Set2DPlotRange(Int_t uaIdx)
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
STEveDrawTask::SetHistPad()
{
  if (fHistPad)
  {
    fHistPad -> Reset();
    return;
  }

  if (fCvsPad == NULL)
    return;

  fCvsPad -> cd();
  fHistPad = new TH1D("Pad","",fNTbs,0,fNTbs);
  fHistPad -> SetLineWidth(3);
  fHistPad -> SetLineColor(kGray);
  fHistPad -> SetFillColor(kGray);
  fHistPad -> SetFillStyle(3004);
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
  fHistPad -> SetMinimum(-50);
  fHistPad -> SetMaximum(4095);
  fHistPad -> SetStats(0);
  fHistPad -> Draw();
}

void
STEveDrawTask::DrawPadPlane()
{
  if (fCvsPadPlane == NULL)
    return;

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
}

void 
STEveDrawTask::UpdateCvsPadPlane()
{
  if (fCvsPadPlane == NULL)
    return;

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
STEveDrawTask::GetColor(Int_t index)
{
  Color_t color;

  Color_t colors[] = {kOrange, kTeal, kViolet, kSpring, kPink, kAzure};

  Int_t remainder = index%6;
  color = colors[remainder];

  Int_t quotient  = index/6;
  Int_t offColor  = (quotient%20);

  if (offColor%2 == 0)
    offColor = offColor/2 + 1;
  else 
    offColor = -(offColor+1)/2;

  color += offColor;

  return color;
}

void 
STEveDrawTask::DrawPadByPosition(Double_t x, Double_t z)
{
  Int_t row = (-x+fXPadPlane/2)/8;
  Int_t layer = z/12;

  DrawPad(row, layer);
}

void
STEveDrawTask::DrawPad(Int_t row, Int_t layer, Bool_t forceUpdate)
{
  if (!fRawEventArray) return;
  if (fCvsPad == NULL)
    return;

  Long64_t currentEvent = fEveManager -> GetCurrentEventEntry();

  if ((fCurrentEvent == currentEvent && row == fCurrentRow  && layer == fCurrentLayer) && !forceUpdate)
    return;

  fCurrentRow = row;
  fCurrentLayer = layer;
  fEveManager -> SetRowLayer(row, layer);
  if (currentEvent != fCurrentEvent) 
  {
    fCurrentEvent = currentEvent;
    fRawEvent = (STRawEvent*) fRawEventArray -> At(0);
    fRawEvent -> ClearHits();
    if (fEvent != NULL)
      fRawEvent -> SetHits(fEvent);
    else if (fHitArray != nullptr)
      fRawEvent -> SetHits(fHitArray);
  }
  STPad* pad = fRawEvent -> GetPad(row, layer);
  if (pad == NULL) 
    return;
  Double_t* adc = pad -> GetADC();

  fHistPad -> SetTitle(Form("row: %d, layer: %d",row, layer));
  Double_t maxAdcCurrentPad = 0;
  Double_t minAdcCurrentPad = 10000;
  for (Int_t tb=0; tb<fNTbs; tb++) {
    Double_t val = adc[tb];
    if (val > maxAdcCurrentPad) maxAdcCurrentPad = val;
    if (val < minAdcCurrentPad) minAdcCurrentPad = val;
    fHistPad -> SetBinContent(tb+1, val);
  }

  fHistPad -> SetMaximum(maxAdcCurrentPad * 1.1);
  fHistPad -> SetMinimum(minAdcCurrentPad * 1.05);

  fCvsPad -> cd();
  fHistPad -> Draw();

  Int_t numHits = pad -> GetNumHits();
  Int_t numMoreHits = numHits - fPulseFunctionArray.size();

  if (numMoreHits > 0)
  {
    for (Int_t iPulse = 0; iPulse < numMoreHits; iPulse++) {
      TF1* f = fPulse -> GetPulseFunction();
      f -> SetNpx(400);
      f -> SetLineColor(kRed);
      fPulseFunctionArray.push_back(f);
    }
  }

  for (Int_t iHit = 0; iHit < numHits; iHit++)
  {
    STHit* hit = pad -> GetHit(iHit);
    TF1* f = fPulseFunctionArray.at(iHit);
    f -> SetParameter(0, hit -> GetCharge());
    f -> SetParameter(1, hit -> GetTb());
    f -> Draw("same");
  }

  fPulseSum -> Set(0);
  fPulseSum -> Clear();

  Double_t xGraph = 0;
  Double_t yGraph = 0;
  Int_t countPoints = 0;
  while(xGraph < 512)
  {
    xGraph += 0.1;
    yGraph  = 0;
    for (Int_t iHit = 0; iHit < numHits; iHit++)
    {
      TF1* f = fPulseFunctionArray.at(iHit);
      yGraph += f -> Eval(xGraph);
    }
    fPulseSum -> SetPoint(countPoints++, xGraph, yGraph);
  }
  fPulseSum -> Draw("samel");

  fCvsPad -> Modified();
  fCvsPad -> Update();

  UpdatePadRange();
}

void
STEveDrawTask::UpdatePadRange()
{
  fHistPad -> GetXaxis() -> SetRangeUser(fWindowTbStart, fWindowTbEnd);
  fCvsPad -> Modified();
  fCvsPad -> Update();
}

void
STEveDrawTask::ClickSelectedPadPlane()
{
  TObject* select = ((TCanvas*)gPad) -> GetClickSelected();

  if (select == NULL || select -> InheritsFrom(TH1::Class()) == kFALSE)
    return;

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

  STEveDrawTask::Instance() -> DrawPadByPosition(xOnClick,yOnClick);

  ((TCanvas*)gPad) -> SetClickSelected(0);
}

void 
STEveDrawTask::SetThresholdRange(TString name, Double_t min, Double_t max) 
{ 
  STEveObject eveObj = GetEveObject(name);

  fThresholdMin[eveObj] = min;
  fThresholdMax[eveObj] = max;
}

void 
STEveDrawTask::UpdateWindowTb(Int_t start, Int_t end) 
{ 
  fWindowTbStart = start;
  fWindowTbEnd   = end;

  fWindowYStart = fWindowTbStart * fTBTime * fDriftVelocity / 10.;
  fWindowYEnd   = fWindowTbEnd * fTBTime * fDriftVelocity / 10.;
}

Int_t 
STEveDrawTask::RnrEveObject(TString name, Int_t option)
{
  name.ToLower();

       if (name == "mc")           return RenderMC(option);
  else if (name == "digi")         return RenderDigi(option);
  else if (name == "hit")          return RenderHit(option);
  else if (name == "hitbox")       return RenderHitBox(option);
  else if (name == "cluster")      return RenderCluster(option);
  else if (name == "clusterbox")   return RenderClusterBox(option);
  else if (name == "riemanntrack") return RenderRiemannTrack(option);
  else if (name == "riemannhit")   return RenderRiemannHit(option);
  else if (name == "linear")       return RenderLinear(option);
  else if (name == "linearhit")    return RenderLinearHit(option);
  else if (name == "curve")        return RenderCurve(option);
  else if (name == "curvehit")     return RenderCurveHit(option);
  else if (name == "recotrack")    return RenderRecoTrack(option);

  return -1;
}

Int_t 
STEveDrawTask::IsSet(TString name, Int_t option)
{
  name.ToLower();

       if (name == "mc")           return BoolToInt(fSetObject[kMC]);
  else if (name == "digi")         return BoolToInt(fSetObject[kDigi]);
  else if (name == "hit")          return BoolToInt(fSetObject[kHit]);
  else if (name == "hitbox")       return BoolToInt(fSetObject[kHitBox]);
  else if (name == "cluster")      return BoolToInt(fSetObject[kCluster]);
  else if (name == "clusterbox")   return BoolToInt(fSetObject[kClusterBox]);
  else if (name == "riemanntrack") return BoolToInt(fSetObject[kRiemannTrack]);
  else if (name == "riemannhit")   return BoolToInt(fSetObject[kRiemannHit]);
  else if (name == "linear")       return BoolToInt(fSetObject[kLinear]);
  else if (name == "linearhit")    return BoolToInt(fSetObject[kLinearHit]);
  else if (name == "curve")        return BoolToInt(fSetObject[kCurve]);
  else if (name == "curvehit")     return BoolToInt(fSetObject[kCurveHit]);
  else if (name == "recotrack")    return BoolToInt(fSetObject[kRecoTrack]);

  return -1;
}

Int_t 
STEveDrawTask::RenderMC(Int_t option)
{
  if (option == 0)
    return BoolToInt(fRnrSelf[kMC]);

  if (fPointSet[kMC] == NULL)
    return -1;

  fRnrSelf[kMC] = !fRnrSelf[kMC];
  fPointSet[kMC] -> SetRnrSelf(fRnrSelf[kMC]);

  return BoolToInt(fRnrSelf[kMC]);
}

Int_t 
STEveDrawTask::RenderDigi(Int_t option)
{
  if (option == 0)
    return BoolToInt(fRnrSelf[kDigi]);

  if (fPointSet[kDigi] == NULL)
    return -1;

  fRnrSelf[kDigi] = !fRnrSelf[kDigi];
  fPointSet[kDigi] -> SetRnrSelf(fRnrSelf[kDigi]);

  return BoolToInt(fRnrSelf[kDigi]);
}

Int_t 
STEveDrawTask::RenderHit(Int_t option)
{
  if (option == 0) 
    return BoolToInt(fRnrSelf[kHit]);

  if (fPointSet[kHit] == NULL)
    return -1;

  fRnrSelf[kHit] = !fRnrSelf[kHit];
  fPointSet[kHit] -> SetRnrSelf(fRnrSelf[kHit]);

  return BoolToInt(fRnrSelf[kHit]);
}

Int_t 
STEveDrawTask::RenderHitBox(Int_t option)
{
  if (option == 0)
    return BoolToInt(fRnrSelf[kHitBox]);

  if (fBoxHitSet == NULL)
    return -1;

  fRnrSelf[kHitBox] = !fRnrSelf[kHitBox];
  fBoxHitSet -> SetRnrSelf(fRnrSelf[kHitBox]);

  return BoolToInt(fRnrSelf[kHitBox]);
}

Int_t 
STEveDrawTask::RenderCluster(Int_t option)
{
  if (option == 0)
    return BoolToInt(fRnrSelf[kCluster]);

  if (fPointSet[kCluster] == NULL)
    return -1;

  fRnrSelf[kCluster] = !fRnrSelf[kCluster];
  fPointSet[kCluster] -> SetRnrSelf(fRnrSelf[kCluster]);

  return BoolToInt(fRnrSelf[kCluster]);
}

Int_t 
STEveDrawTask::RenderClusterBox(Int_t option)
{
  if (option == 0)
    return BoolToInt(fRnrSelf[kClusterBox]);

  if (fBoxClusterSet == NULL)
    return -1;

  fRnrSelf[kClusterBox] = !fRnrSelf[kClusterBox];
  fBoxClusterSet -> SetRnrSelf(fRnrSelf[kClusterBox]);

  return BoolToInt(fRnrSelf[kClusterBox]);
}

Int_t 
STEveDrawTask::RenderRiemannTrack(Int_t option)
{
  if (option == 0)
    return BoolToInt(fRnrSelf[kRiemannTrack]);

  Int_t nRiemannTrackSets = fRiemannTrackSetArray.size();
  if (nRiemannTrackSets == 0)
    return -1;

  fRnrSelf[kRiemannTrack] = !fRnrSelf[kRiemannTrack];
  for (Int_t i=0; i<nRiemannTrackSets; i++)
  {
    TEveLine* riemannTrackLine = fRiemannTrackSetArray.at(i);
    if (riemannTrackLine -> Size() == 0)
      riemannTrackLine -> SetRnrSelf(kFALSE);
    else
      riemannTrackLine -> SetRnrSelf(fRnrSelf[kRiemannTrack]);
  }

  return BoolToInt(fRnrSelf[kRiemannTrack]);
}

Int_t 
STEveDrawTask::RenderRiemannHit(Int_t option)
{
  if (option == 0)
    return BoolToInt(fRnrSelf[kRiemannHit]);

  Int_t nRiemannSets = fRiemannSetArray.size();
  if (nRiemannSets == 0)
    return -1;

  fRnrSelf[kRiemannHit] = !fRnrSelf[kRiemannHit];
  for (Int_t i=0; i<nRiemannSets; i++)
  {
    TEvePointSet* riemannPointSet = fRiemannSetArray.at(i);
    if (riemannPointSet -> Size() == 0)
      riemannPointSet -> SetRnrSelf(kFALSE);
    else
      riemannPointSet -> SetRnrSelf(fRnrSelf[kRiemannHit]);
  }

  return BoolToInt(fRnrSelf[kRiemannHit]);
}

Int_t 
STEveDrawTask::RenderLinear(Int_t option)
{
  if (option == 0)
    return BoolToInt(fRnrSelf[kLinear]);

  Int_t nLinearTrackSets = fLinearTrackSetArray.size();
  if (nLinearTrackSets == 0)
    return -1;

  fRnrSelf[kLinear] = !fRnrSelf[kLinear];
  for (Int_t i=0; i<nLinearTrackSets; i++)
  {
    TEveLine* linearTrackLine = fLinearTrackSetArray.at(i);
    if (linearTrackLine -> Size() == 0)
      linearTrackLine -> SetRnrSelf(kFALSE);
    else
      linearTrackLine -> SetRnrSelf(fRnrSelf[kLinear]);
  }

  return BoolToInt(fRnrSelf[kLinear]);
}

Int_t 
STEveDrawTask::RenderLinearHit(Int_t option)
{
  if (option == 0)
    return BoolToInt(fRnrSelf[kLinearHit]);

  Int_t nLinearHitSets = fLinearHitSetArray.size();
  if (nLinearHitSets == 0)
    return -1;

  fRnrSelf[kLinearHit] = !fRnrSelf[kLinearHit];
  for (Int_t i=0; i<nLinearHitSets; i++)
  {
    TEvePointSet* pointSet = fLinearHitSetArray.at(i);
    if (pointSet -> Size() == 0)
      pointSet -> SetRnrSelf(kFALSE);
    else
      pointSet -> SetRnrSelf(fRnrSelf[kLinearHit]);
  }

  return BoolToInt(fRnrSelf[kLinearHit]);
}

Int_t 
STEveDrawTask::RenderCurve(Int_t option)
{
  if (option == 0)
    return BoolToInt(fRnrSelf[kCurve]);

  Int_t nCurveTrackSets = fCurveTrackSetArray.size();
  if (nCurveTrackSets == 0)
    return -1;

  fRnrSelf[kCurve] = !fRnrSelf[kCurve];
  for (Int_t i=0; i<nCurveTrackSets; i++)
  {
    TEveLine* curveTrackLine = fCurveTrackSetArray.at(i);
    if (curveTrackLine -> Size() == 0)
      curveTrackLine -> SetRnrSelf(kFALSE);
    else
      curveTrackLine -> SetRnrSelf(fRnrSelf[kCurve]);
  }

  return BoolToInt(fRnrSelf[kCurve]);
}

Int_t 
STEveDrawTask::RenderCurveHit(Int_t option)
{
  if (option == 0)
    return BoolToInt(fRnrSelf[kCurveHit]);

  Int_t nCurveHitSets = fCurveHitSetArray.size();
  if (nCurveHitSets == 0)
    return -1;

  fRnrSelf[kCurveHit] = !fRnrSelf[kCurveHit];
  for (Int_t i=0; i<nCurveHitSets; i++)
  {
    TEvePointSet* pointSet = fCurveHitSetArray.at(i);
    if (pointSet -> Size() == 0)
      pointSet -> SetRnrSelf(kFALSE);
    else
      pointSet -> SetRnrSelf(fRnrSelf[kCurveHit]);
  }

  return BoolToInt(fRnrSelf[kCurveHit]);
}

Int_t 
STEveDrawTask::RenderRecoTrack(Int_t option)
{
  if (option == 0)
    return BoolToInt(fRnrSelf[kRecoTrack]);

  Int_t nRecoTrackSets = fRecoTrackSetArray.size();
  if (nRecoTrackSets == 0)
    return -1;

  fRnrSelf[kRecoTrack] = !fRnrSelf[kRecoTrack];
  for (Int_t i=0; i<nRecoTrackSets; i++)
  {
    TEveLine* recoTrackLine = fRecoTrackSetArray.at(i);
    if (recoTrackLine -> Size() == 0)
      recoTrackLine -> SetRnrSelf(kFALSE);
    else
      recoTrackLine -> SetRnrSelf(fRnrSelf[kRecoTrack]);
  }

  return BoolToInt(fRnrSelf[kRecoTrack]);
}

Int_t STEveDrawTask::BoolToInt(Bool_t val) 
{ 
  if (val == kTRUE) 
    return 1;
  else 
    return 0;
}
