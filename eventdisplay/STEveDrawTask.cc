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

  fEveManager = NULL;

  fEvent = NULL;

  fMCHitArray           = NULL;
  fDriftedElectronArray = NULL;
  fEventArray           = NULL;
  fRiemannTrackArray    = NULL;
  fLinearTrackArray     = NULL;
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

  for (Int_t i=0; i<fNumEveObject; i++)
  {
    fPointSet[i] = NULL;
    fThresholdMin[i] = 0;
    fThresholdMax[i] = 1.E10;
    fSetObject[i] = kFALSE;
  }

  fEveStyle[kMC] = kFullCircle;
  fEveSize[kMC]  = 1;
  fEveColor[kMC] = kGray+2;
  fRnrSelf[kMC]  = kFALSE;

  fEveStyle[kDigi] = 1;
  fEveSize[kDigi]  = 1.;
  fEveColor[kDigi] = kAzure-5;
  fRnrSelf[kDigi]  = kFALSE;

  fEveStyle[kHit]   = kFullCircle;
  fEveSize[kHit]    = 1.0;
  fEveColor[kHit]   = kBlue;
  fRnrSelf[kHit]    = kFALSE;
  fSetObject[kHit]  = kFALSE;

  fEveStyle[kHitBox]   = kFullCircle;
  fEveSize[kHitBox]    = 0.5;
  fEveColor[kHitBox]   = kBlue;
  fRnrSelf[kHitBox]    = kFALSE;
  fSetObject[kHitBox]  = kFALSE;

  fEveStyle[kCluster]     = kFullCircle;
  fEveSize[kCluster]      = 1.;
  fEveColor[kCluster]     = kBlack;
  fRnrSelf[kCluster]      = kFALSE;
  fRnrSelf[kClusterBox]   = kFALSE;
  fSetObject[kCluster]    = kFALSE;
  fSetObject[kClusterBox] = kFALSE;

  fEveStyle[kRiemannHit]  = kFullCircle;
  fEveSize[kRiemannHit]   = 1.0;
  fEveColor[kRiemannHit]  = 0;
  fRnrSelf[kRiemannHit]   = kFALSE;
  fSetObject[kRiemannHit] = kFALSE;

  fEveStyle[kLinear]  = 1;
  fEveSize[kLinear]   = 5;
  fEveColor[kLinear]  = kRed;
  fRnrSelf[kLinear]   = kFALSE;
  fSetObject[kLinear] = kFALSE;

  fEveStyle[kLinearHit]  = kFullCircle;
  fEveSize[kLinearHit]   = 0.5;
  fEveColor[kLinearHit]  = 0;
  fRnrSelf[kLinearHit]   = kFALSE;
  fSetObject[kLinearHit] = kFALSE;

  fBoxHitSet = NULL;
  fBoxClusterSet = NULL;

  fPulse = new STPulse();
  fLTFitter = new STLinearTrackFitter();

  for (Int_t i=0; i<fNumPulseFunction; i++)
  {
    fPulseFunction[i] = new TF1(Form("pulse_%d",i), this, &STEveDrawTask::Pulse, 0, 512, 2, "STEveDrawTask", "Pulse");
    fPulseFunction[i] -> SetLineColor(kRed);
    fPulseFunction[i] -> SetNpx(400);
  }

  fPulseSum = new TGraph();
  fPulseSum -> SetLineColor(kBlack);
  fPulseSum -> SetLineWidth(2);

  fRGBAPalette = new TEveRGBAPalette(0, 4096);
}

void STEveDrawTask::DrawADC(Int_t row, Int_t layer)
{
  DrawPad(fCurrentRow, fCurrentLayer);
  UpdatePadRange();
}

void STEveDrawTask::PushParameters()
{
  fEveManager -> SetNumRiemannSet(fRiemannSetArray.size());
  fEveManager -> SetNumLinearSet(fLinearTrackSetArray.size());
}

Double_t STEveDrawTask::Pulse(Double_t *x, Double_t *par)
{
  if ((x[0] == x[0]) == kFALSE) LOG(INFO) << "x is NAN! " << FairLogger::endl;
  if ((par[0] == par[0]) == kFALSE) LOG(INFO) << "par0 is NAN! " << FairLogger::endl;
  if ((par[1] == par[1]) == kFALSE) LOG(INFO) << "par1 is NAN! " << FairLogger::endl;
  return fPulse -> Pulse(x[0], par[0], par[1]);
}

STEveDrawTask::~STEveDrawTask()
{
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
  fRawEventArray        = (TClonesArray*) ioMan -> GetObject("STRawEvent");

  gStyle -> SetPalette(55);
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

  if (fRiemannTrackArray != NULL && fSetObject[kRiemannHit])
    DrawRiemannHits();

  if (fLinearTrackArray != NULL && 
      (fSetObject[kLinear] || fSetObject[kLinearHit]))
    DrawLinearTracks();

  gEve -> Redraw3D(kFALSE);
  UpdateCvsPadPlane();
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
}

void 
STEveDrawTask::DrawHitPoints()
{
  fLogger -> Debug(MESSAGE_ORIGIN,"DrawHitPoints()");

  if (fEvent == NULL) 
    return;

  Int_t nHits = fEvent -> GetNumHits();

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
    fPointSet[kHit] -> SetMainTransparency(80);
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
    STHit hit = fEvent -> GetHitArray() -> at(iHit);

    if (hit.GetCharge() < fThresholdMin[kHit] || 
        hit.GetCharge() > fThresholdMax[kHit])
      continue;

    TVector3 position = hit.GetPosition();

    if (hit.GetTb() < fWindowTbStart || hit.GetTb() > fWindowTbEnd)
      continue;

    Double_t y = position.Y()/10.;
    y += fWindowYStart;

    if (fSetObject[kHit]) 
      fPointSet[kHit] -> SetNextPoint(position.X()/10., y, position.Z()/10.);

    if (fPadPlane != NULL)
      fPadPlane -> Fill(-position.X(), position.Z(), hit.GetCharge());

    if (fSetObject[kHitBox]) 
    {
      fBoxHitSet -> AddBox(position.X()/10. - fBoxHitSet -> GetDefWidth()/2., 
                           y - fBoxHitSet -> GetDefHeight()/2., 
                           position.Z()/10. - fBoxHitSet -> GetDefDepth()/2.);

      fBoxHitSet -> DigitValue(hit.GetCharge());
    }
  }
}

void 
STEveDrawTask::DrawHitClusterPoints()
{
  fLogger -> Debug(MESSAGE_ORIGIN,"DrawHitClusterPoints()");

  if (fEvent == NULL) 
    return;

  Int_t nClusters = fEvent -> GetNumClusters();

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
    Double_t yP =                 y - yS/2.;
    Double_t zP =  position.Z()/10. - zS/2.;

    if (fSetObject[kClusterBox]) {
      fBoxClusterSet -> AddBox(xP, yP, zP, xS, yS, zS);
      fBoxClusterSet -> DigitValue(cluster.GetCharge());
    }
  }
}

void 
STEveDrawTask::DrawRiemannHits()
{
  fLogger -> Debug(MESSAGE_ORIGIN,"DrawRiemannHits()");

  STRiemannTrack* track = NULL;
  STHit* rHit = NULL;
  TEvePointSet* riemannPointSet = NULL;

  if (fEvent == NULL) 
    return;

  Int_t nTracks = fRiemannTrackArray -> GetEntries();
  Int_t nTracksInBuffer = fRiemannSetArray.size();

  if (nTracksInBuffer < nTracks)
  {
    Int_t diffNumTracks = nTracks - nTracksInBuffer;
    for (Int_t iTrack = 0; iTrack < diffNumTracks; iTrack++)
    {
      Int_t idxTrack = iTrack + nTracksInBuffer;
      riemannPointSet = new TEvePointSet(Form("RiemannTrackHits_%d", idxTrack), 1, TEvePointSelectorConsumer::kTVT_XYZ);
      riemannPointSet -> SetMarkerColor(GetColor(idxTrack));
      riemannPointSet -> SetMarkerSize(fEveSize[kRiemannHit]);
      riemannPointSet -> SetMarkerStyle(fEveStyle[kRiemannHit]);
      riemannPointSet -> SetRnrSelf(fRnrSelf[kRiemannHit]);
      fRiemannSetArray.push_back(riemannPointSet);
      gEve -> AddElement(riemannPointSet);
    }
  }

  for (Int_t iTrack=0; iTrack<nTracks; iTrack++) 
  {
    track = (STRiemannTrack*) fRiemannTrackArray -> At(iTrack);
    Int_t nClusters = track -> GetNumHits();

    riemannPointSet = fRiemannSetArray.at(iTrack);

    for (Int_t iCluster=0; iCluster<nClusters; iCluster++)
    {
      rHit = track -> GetHit(iCluster) -> GetHit();

      if (rHit -> GetCharge() < fThresholdMin[kRiemannHit] || 
          rHit -> GetCharge() > fThresholdMax[kRiemannHit])
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
    riemannPointSet -> SetRnrSelf(fRnrSelf[kRiemannHit]);
  }
}

void
STEveDrawTask::DrawLinearTracks()
{
  fLogger -> Debug(MESSAGE_ORIGIN,"Draw Linear Tracks");

  if (fEvent == NULL) 
    return;

  STLinearTrack* track = NULL;
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
      linearTrackLine -> SetLineStyle(fEveStyle[kLinear]);
      linearTrackLine -> SetLineColor(GetColor(idxTrack));
      linearTrackLine -> SetLineWidth(fEveSize[kLinear]);
      fLinearTrackSetArray.push_back(linearTrackLine);
      gEve -> AddElement(linearTrackLine);

      if (fSetObject[kLinearHit] == kTRUE)
      {
        linearPointSet = new TEvePointSet(Form("LinearHit_%d", idxTrack), 1, TEvePointSelectorConsumer::kTVT_XYZ);
        linearPointSet -> SetMarkerColor(GetColor(iTrack));
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

    STHit *hitFirst = fEvent -> GetHit(hitIDFirst);
    STHit *hitLast  = fEvent -> GetHit(hitIDLast);

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
        hit = fEvent -> GetHit(track -> GetHitID(iHit));
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
STEveDrawTask::SetSelfRiemannSet(Int_t iRiemannSet, Bool_t offElse)
{
  Int_t nRiemannSets = fRiemannSetArray.size();

  if (iRiemannSet == -1) 
  {
    if (!offElse)
    {
      for (Int_t i=0; i<nRiemannSets; i++)
      {
        TEvePointSet* riemannPointSet = fRiemannSetArray.at(i);
        riemannPointSet -> SetRnrSelf(kTRUE);
      }
    }
    else
    {
      for (Int_t i=0; i<nRiemannSets; i++)
      {
        TEvePointSet* riemannPointSet = fRiemannSetArray.at(i);
        riemannPointSet -> SetRnrSelf(kFALSE);
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
      //gEve -> RemoveElement(fPointSet[i], fEveManager);
      //fPointSet[i] = NULL;
    }
  }

  fLogger -> Debug(MESSAGE_ORIGIN,"Reset box hit set");

  if (fBoxHitSet) {
    fBoxHitSet -> Reset();
    //gEve -> RemoveElement(fBoxHitSet, fEveManager);
  }

  fLogger -> Debug(MESSAGE_ORIGIN,"Reset box cluster set");

  if (fBoxClusterSet) {
    fBoxClusterSet -> Reset();
    //gEve -> RemoveElement(fBoxClusterSet, fEveManager);
  }

  fLogger -> Debug(MESSAGE_ORIGIN,"Reset riemann set");

  Int_t nRiemannTracks = fRiemannSetArray.size();
  for (Int_t i=0; i<nRiemannTracks; i++) 
  {
    TEvePointSet* riemannPointSet = fRiemannSetArray.at(i);
    //gEve -> RemoveElement(riemannPointSet, fEveManager);
    riemannPointSet -> SetRnrSelf(kFALSE);
    riemannPointSet -> Reset();
  }
  //fRiemannSetArray.clear();

  fLogger -> Debug(MESSAGE_ORIGIN,"Reset linear line set");

  Int_t nLinearTracks = fLinearTrackSetArray.size();
  for (Int_t i=0; i<nLinearTracks; i++) 
  {
    TEveLine* linearTrackLine = fLinearTrackSetArray.at(i);
    //gEve -> RemoveElement(linearTrackLine, fEveManager);
    linearTrackLine -> SetRnrSelf(kFALSE);
    linearTrackLine -> Reset();
  }

  fLogger -> Debug(MESSAGE_ORIGIN,"Reset linear point set");

  Int_t nLinearPoints = fLinearHitSetArray.size();
  for (Int_t i=0; i<nLinearPoints; i++) 
  {
    TEvePointSet* pointSet = fLinearHitSetArray.at(i);
    //gEve -> RemoveElement(point, fEveManager);
    pointSet -> SetRnrSelf(kFALSE);
    pointSet -> Reset();
  }

  //fLinearTrackSetArray.clear();
  //fLinearHitSetArray.clear();

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
STEveDrawTask::DrawPad(Int_t row, Int_t layer)
{
  if (!fRawEventArray) return;
  if (fCvsPad == NULL)
    return;

  Long64_t currentEvent = fEveManager -> GetCurrentEventEntry();

  if (fCurrentEvent == currentEvent && 
                row == fCurrentRow  && 
              layer == fCurrentLayer)
    return;

  fCurrentRow = row;
  fCurrentLayer = layer;
  fEveManager -> SetRowLayer(row, layer);
  if (currentEvent != fCurrentEvent) 
  {
    fCurrentEvent = currentEvent;
    fRawEvent = (STRawEvent*) fRawEventArray -> At(0);
    fRawEvent -> SetHits(fEvent);
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

  Int_t numHits = pad -> GetNumHits();
  LOG(INFO) << "Number of hits: " << numHits << FairLogger::endl;
  if (numHits > fNumPulseFunction) numHits = fNumPulseFunction;
  for (Int_t iHit = 0; iHit < numHits; iHit++)
  {
    STHit* hit = pad -> GetHit(iHit);

    LOG(DEBUG) << "Hit " << iHit   << " (" 
               << hit -> GetHitID()  << ") " 
               << hit -> GetTb()     << " " 
               << hit -> GetCharge() << " "
               << hit -> GetChi2()   << " " 
               << hit -> GetNDF()    << FairLogger::endl;

    fPulseFunction[iHit] -> SetParameter(0, hit -> GetCharge());
    fPulseFunction[iHit] -> SetParameter(1, hit -> GetTb());
    fPulseFunction[iHit] -> Draw("same");
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
      yGraph += fPulseFunction[iHit] -> Eval(xGraph);
    fPulseSum -> SetPoint(countPoints++, xGraph, yGraph);
  }
  fPulseSum -> Draw("samel");

  fCvsPad -> Modified();
  fCvsPad -> Update();
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

       if (name == "mc")         return RenderMC(option);
  else if (name == "digi")       return RenderDigi(option);
  else if (name == "hit")        return RenderHit(option);
  else if (name == "hitbox")     return RenderHitBox(option);
  else if (name == "cluster")    return RenderCluster(option);
  else if (name == "clusterbox") return RenderClusterBox(option);
  else if (name == "riemannhit") return RenderRiemannHit(option);
  else if (name == "linear")     return RenderLinear(option);
  else if (name == "linearhit")  return RenderLinearHit(option);

  return -1;
}

Int_t 
STEveDrawTask::IsSet(TString name, Int_t option)
{
  name.ToLower();

       if (name == "mc")         return BoolToInt(fSetObject[kMC]);
  else if (name == "digi")       return BoolToInt(fSetObject[kDigi]);
  else if (name == "hit")        return BoolToInt(fSetObject[kHit]);
  else if (name == "hitbox")     return BoolToInt(fSetObject[kHitBox]);
  else if (name == "cluster")    return BoolToInt(fSetObject[kCluster]);
  else if (name == "clusterbox") return BoolToInt(fSetObject[kClusterBox]);
  else if (name == "riemannhit") return BoolToInt(fSetObject[kRiemannHit]);
  else if (name == "linear")     return BoolToInt(fSetObject[kLinear]);
  else if (name == "linearhit")  return BoolToInt(fSetObject[kLinearHit]);

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

Int_t STEveDrawTask::BoolToInt(Bool_t val) 
{ 
  if (val == kTRUE) 
    return 1;
  else 
    return 0;
}
