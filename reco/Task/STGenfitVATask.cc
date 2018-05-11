#include "STGenfitVATask.hh"
#include "STDatabasePDG.hh"
#include "STTrack.hh"

#include "FairRun.h"
#include "FairRuntimeDb.h"
#include "FairRootManager.h"

#include "Track.h"

#include <vector>
#include <iostream>
using namespace std;

ClassImp(STGenfitVATask)

STGenfitVATask::STGenfitVATask()
: STRecoTask("GENFIT VA Task", 1, false)
{
}

STGenfitVATask::STGenfitVATask(Bool_t persistence)
: STRecoTask("GENFIT VA Task", 1, persistence)
{
}

STGenfitVATask::~STGenfitVATask()
{
}

void STGenfitVATask::SetClusteringType(Int_t type) { fClusteringType = type; }
void STGenfitVATask::SetConstantField() { fIsSamurai = kFALSE; }

InitStatus
STGenfitVATask::Init()
{
  if (STRecoTask::Init() == kERROR)
    return kERROR;

  STDatabasePDG::Instance();

  fHelixTrackArray = (TClonesArray *) fRootManager -> GetObject("STHelixTrack");
  if (fHelixTrackArray == nullptr) {
    LOG(ERROR) << "Cannot find STHelixTrack array!" << FairLogger::endl;
    return kERROR;
  }

  fRecoTrackArray = (TClonesArray *) fRootManager -> GetObject("STRecoTrack");
  if (fRecoTrackArray == nullptr) {
    LOG(ERROR) << "Cannot find STRecoTrack array!" << FairLogger::endl;
    return kERROR;
  }

  fVertexArray = (TClonesArray *) fRootManager -> GetObject("STVertex");
  if (fVertexArray == nullptr) {
    LOG(ERROR) << "Cannot find STVertex array!" << FairLogger::endl;
    return kERROR;
  }

  fVATrackArray = new TClonesArray("STRecoTrack");
  fRootManager -> Register("VATracks", "SpiRIT", fVATrackArray, fIsPersistence);

  fCandListArray = new TClonesArray("STRecoTrackCandList");
  fRootManager -> Register("VACandList", "SpiRIT", fCandListArray, fIsListPersistence);

  fGenfitTest = new STGenfitTest2(fIsSamurai);
  fPIDTest = new STPIDTest();

  if (!fBeamFilename.IsNull()) {
    fBDCVertexArray = new TClonesArray("STVertex");
    fRootManager -> Register("BDCVertex", "SpiRIT", fBDCVertexArray, kTRUE);

    fBeamFile = new TFile(fBeamFilename);
    fBeamTree = (TTree *) fBeamFile -> Get("TBeam");
    fBeamTree -> SetBranchAddress("z", &fZ);
    fBeamTree -> SetBranchAddress("aoq", &fAoQ);
    fBeamTree -> SetBranchAddress("beta37", &fBeta37);
    fBDCTree = (TTree *) fBeamFile -> Get("TBDC");
    fBDCTree -> SetBranchAddress("bdc1x", &fBDC1x);
    fBDCTree -> SetBranchAddress("bdc1y", &fBDC1y);
    fBDCTree -> SetBranchAddress("bdc2x", &fBDC2x);
    fBDCTree -> SetBranchAddress("bdc2y", &fBDC2y);
    fBDCTree -> SetBranchAddress("bdcax", &fBDCax);
    fBDCTree -> SetBranchAddress("bdcby", &fBDCby);

    fBeamEnergy = new STBeamEnergy();
    fBeamEnergy -> setBeam(fRunNo);
    fBDCProjection = new STBDCProjection(TString(gSystem -> Getenv("VMCWORKDIR")) + "/parameters/ReducedBMap.txt");
    fBDCProjection -> setBeam(fRunNo);
  }


//  fVertexFactory = new genfit::GFRaveVertexFactory(/* verbosity */ 2, /* useVacuumPropagator */ false);
  //fVertexFactory -> setMethod("kalman-smoothing:1");
//  fGFRaveVertexMethod = "avf-smoothing:1-Tini:256-ratio:0.25-sigmacut:5";
//  fVertexFactory -> setMethod(fGFRaveVertexMethod.Data());

  if (fRecoHeader != nullptr) {
    fRecoHeader -> SetPar("genfitva_loadSamuraiMap", fIsSamurai);
    fRecoHeader -> Write("RecoHeader", TObject::kWriteDelete);
  }

  return kSUCCESS;
}

void STGenfitVATask::Exec(Option_t *opt)
{
  fCandListArray -> Clear("C");
  fVATrackArray -> Clear("C");
  if (!fBeamFilename.IsNull())
    fBDCVertexArray -> Clear("C");

  if (fEventHeader -> IsBadEvent())
    return;

  fGenfitTest -> Init();

  vector<genfit::Track *> gfTrackArrayToVertex;
  vector<STRecoTrack *> vaTrackArrayToVertex;

  Int_t chosenVID = 0;

  auto numVertices = fVertexArray -> GetEntriesFast();
  if (numVertices < 1) {
    LOG(INFO) << Space() << "STGenfitVATask - No vertex found!" << FairLogger::endl;
    return;
  } else if (numVertices > 1) {
    for (Int_t iVertex = 1; iVertex < numVertices; iVertex++) {
      auto v1 = (STVertex *) fVertexArray -> At(chosenVID);
      auto v2 = (STVertex *) fVertexArray -> At(iVertex);

      chosenVID = (v1 -> GetNTracks() > v2 -> GetNTracks() ? chosenVID : iVertex);
    }
  }

  Bool_t targetVertex = kTRUE;
  auto vertex = (STVertex *) fVertexArray -> At(chosenVID);
  vertex -> SetIsCollisionVertex();
  if (fPeakZ != 9999) {
    if (vertex -> GetPos().Z() < fPeakZ - fSigmaMultiple*fSigma || vertex -> GetPos().Z() > fPeakZ + fSigmaMultiple*fSigma)
      targetVertex = kFALSE;
    else
      vertex -> SetIsTargetVertex();
  }

  auto vertexPos = vertex -> GetPos();
  Bool_t goodBDC = kTRUE;
  if (!fBeamFilename.IsNull()) {
    fBeamTree -> GetEntry(fEventHeader -> GetEventID() - 1);
    fBDCTree -> GetEntry(fEventHeader -> GetEventID() - 1);
    fBeamEnergy -> reset(fZ, fAoQ, fBeta37);
    Double_t E1 = fBeamEnergy -> getCorrectedEnergy();

    if (fZ > 0 && fZ < 75 && fAoQ > 1. && fAoQ < 3 && fBDC1x > -999 && fBDC1y > -999 && fBDC2x > -999 && fBDC2y > -999) {
//      Double_t ProjectedAtZ = -580.4 + vertex -> GetPos().Z();  // mid target = -592.644, start pad plane =-580.4, end of pad plane = 763.6
      Double_t ProjectedAtZ = -580.4 + (fPeakZ != -9999 ? fPeakZ : vertex -> GetPos().Z());  // mid target = -592.644, start pad plane =-580.4, end of pad plane = 763.6
//      double ProjectedAtZ=-592.644;//////mid target = -592.644, start pad plane =-580.4, end of pad plane = 763.6
      fBDCProjection -> ProjectParticle(fBDC2x, fBDC2y, -2160., fBDCax, fBDCby, fZ, E1, ProjectedAtZ, fBeamEnergy -> getMass());//-580.4,-583.904

      vertex -> SetIsGoodBDC();
    } else
      goodBDC = kFALSE;

    if (!targetVertex && !goodBDC)
      LOG(INFO) << Space() << "STGenfitVATask " << "Off-target event probable with bad BDC!" << FairLogger::endl;
    else if (!targetVertex)
      LOG(INFO) << Space() << "STGenfitVATask " << "Off-target event probable!" << FairLogger::endl;
    else if (!goodBDC)
      LOG(INFO) << Space() << "STGenfitVATask " << "Bad BDC!" << FairLogger::endl;
    else {
      vertexPos = TVector3(fBDCProjection -> getX() + fOffsetX, fBDCProjection -> getY() + fOffsetY, (fPeakZ != -9999 ? fPeakZ : vertex -> GetPos().Z()));

      auto bdcVertex = (STVertex *) fBDCVertexArray -> ConstructedAt(0);
      bdcVertex -> SetIsGoodBDC();
      bdcVertex -> SetIsTargetVertex();
      bdcVertex -> SetIsCollisionVertex();
      bdcVertex -> SetPos(vertexPos);

      LOG(INFO) << Space() << "STGenfitVATask " << "Target event probable with good BDC!" << FairLogger::endl;
    }
  }

  auto numTracks = fRecoTrackArray -> GetEntriesFast();
  for (auto iTrack = 0; iTrack < numTracks; iTrack++) {
    auto track = (STRecoTrack *) fRecoTrackArray -> At(iTrack);

    if (track -> GetVertexID() != chosenVID)
      continue;

    auto cov = vertex -> GetCov();
    cov(0,0) = abs(cov(0,0));
    cov(1,1) = abs(cov(1,1));
    cov(2,2) = abs(cov(2,2));
    cov(0,1) = abs(cov(0,1));
    cov(1,2) = abs(cov(1,2));
    cov(2,0) = abs(cov(2,0));
    cov(1,0) = cov(0,1);
    cov(2,1) = cov(1,2);
    cov(0,2) = cov(2,0);

    auto vertexCluster = new STHitCluster();
    vertexCluster -> SetIsStable(kTRUE);
    vertexCluster -> SetHit(-4, vertexPos, 30);
    vertexCluster -> SetCovMatrix(cov);
    vertexCluster -> SetDFromCovForGenfit(1,1,1,true);

    Int_t trackID = fCandListArray -> GetEntriesFast();

    auto candList = (STRecoTrackCandList *) fCandListArray -> ConstructedAt(trackID);
    auto vaTrack = (STRecoTrack *) fVATrackArray -> ConstructedAt(trackID);
    vaTrack -> SetParentID(iTrack);

    auto helixTrack = (STHelixTrack *) fHelixTrackArray -> At(track -> GetHelixID());
    vaTrack -> SetHelixTrack(helixTrack);

    auto clusterArray = helixTrack -> GetClusterArray();
    clusterArray -> insert(clusterArray -> begin(), vertexCluster);
    for (auto cluster : *clusterArray) {
      if (cluster -> IsStable()) {
        candList -> AddHitID(cluster -> GetClusterID());
        vaTrack -> AddClusterID(cluster -> GetClusterID());
      }
    }

    STPID::PID bestPID = STPID::kNon;
    Double_t bestProb = 0;

    for (Int_t iPID = 0; iPID < NUMSTPID; ++iPID)
    {
      STPID::PID pid = static_cast<STPID::PID>(iPID);

      Int_t pdg;
           if (pid == STPID::kPion)     pdg = 211; 
      else if (pid == STPID::kProton)   pdg = 2212; 
//      else if (pid == STPID::kDeuteron) pdg = 1000010020;
//      else if (pid == STPID::kTriton)   pdg = 1000010030;
      //else if (pid == STPID::k3He)      pdg = 1000020030;
      //else if (pid == STPID::k4He)      pdg = 1000020040;
      else continue;

      auto vaTrackCand = candList -> GetRecoTrackCand(iPID);

      //genfit::Track *gfTrack = fGenfitTest -> FitTrackWithVertex(helixTrack, vertexCluster, pdg);
      genfit::Track *gfTrack = fGenfitTest -> FitTrack(helixTrack, pdg);
      if (gfTrack == nullptr) {
        vaTrackCand -> SetPIDProbability(0);
        continue;
      } else {
        vaTrackCand -> SetGenfitTrack(gfTrack);
      }

      TVector3 mom, momTargetPlane, posTargetPlane;
      fGenfitTest -> GetTrackParameters(gfTrack, mom, momTargetPlane, posTargetPlane);
      if (mom.Z() < 0)
        mom = -mom;
      if (momTargetPlane.Z() < 0)
        momTargetPlane = -momTargetPlane;
      vaTrackCand -> SetMomentum(mom);
      vaTrackCand -> SetMomentumTargetPlane(momTargetPlane);
      vaTrackCand -> SetPosTargetPlane(posTargetPlane);

      fGenfitTest -> GetdEdxPointsByLayerRow(gfTrack, helixTrack, vaTrackCand -> GetdEdxPointArray(), true);

      auto prob = fPIDTest -> GetProbability(pid, mom.Mag(), vaTrackCand -> GetdEdxWithCut(0,0.7));
      vaTrackCand -> SetPIDProbability(prob);

      if (bestProb < prob) {
        bestProb = prob;
        bestPID = pid;
      }
    }

    if (bestPID == STPID::kNon) {
      candList -> SetBestPID(bestPID);
      continue;
    }

    candList -> SetBestPID(bestPID);
    auto bestRecoTrackCand = candList -> GetRecoTrackCand(candList -> GetBestPID());
    gfTrackArrayToVertex.push_back(bestRecoTrackCand -> GetGenfitTrack());
    vaTrackArrayToVertex.push_back(vaTrack);

    helixTrack -> SetGenfitID(iTrack);
    helixTrack -> SetIsGenfitTrack();
    helixTrack -> SetGenfitMomentum(bestRecoTrackCand -> GetMomentum().Mag());

    TVector3 kyotoL, kyotoR, katana, neuland;
    fGenfitTest -> GetPosOnPlanes(bestRecoTrackCand -> GetGenfitTrack(), kyotoL, kyotoR, katana, neuland);
    bestRecoTrackCand -> Copy(vaTrack);
    vaTrack -> SetPosKyotoL(kyotoL);
    vaTrack -> SetPosKyotoR(kyotoR);
    vaTrack -> SetPosKatana(katana);
    vaTrack -> SetPosNeuland(neuland);

    vaTrack -> SetVertexID(chosenVID);

    genfit::Track *gfTrack = bestRecoTrackCand -> GetGenfitTrack();
    TVector3 momVertex;
    TVector3 pocaVertex;
    fGenfitTest -> GetMomentumWithVertex(gfTrack, vertex -> GetPos(), momVertex, pocaVertex);
    if (momVertex.Z() < 0)
      momVertex = -momVertex;
    vaTrack -> SetMomentum(momVertex);
    vaTrack -> SetPOCAVertex(pocaVertex);

    Double_t effCurvature1;
    Double_t effCurvature2;
    Double_t effCurvature3;
    Double_t charge = fGenfitTest -> DetermineCharge(vaTrack, vertex -> GetPos(), effCurvature1, effCurvature2, effCurvature3, true);
    vaTrack -> SetCharge(charge);
    vaTrack -> SetEffCurvature1(effCurvature1);
    vaTrack -> SetEffCurvature2(effCurvature2);
    vaTrack -> SetEffCurvature3(effCurvature3);
  }

  LOG(INFO) << Space() << "VATrack " << fRecoTrackArray -> GetEntriesFast() << FairLogger::endl;
}

void STGenfitVATask::SetBeamFile(TString fileName) { fBeamFilename = fileName; }
void STGenfitVATask::SetInformationForBDC(Int_t runNo, Double_t offsetX, Double_t offsetY)
{
  fRunNo = runNo;
  fOffsetX = offsetX;
  fOffsetY = offsetY;
}

void STGenfitVATask::SetZtoProject(Double_t peakZ, Double_t sigma, Double_t sigmaMultiple)
{
  fPeakZ = peakZ;
  fSigma = sigma;
  fSigmaMultiple = sigmaMultiple;
}
