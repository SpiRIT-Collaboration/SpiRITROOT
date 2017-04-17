#include "STGenfitPIDTask.hh"
#include "STDatabasePDG.hh"
#include "STTrack.hh"

#include "FairRun.h"
#include "FairRuntimeDb.h"
#include "FairRootManager.h"

#include "Track.h"

#include <vector>
#include <iostream>
using namespace std;

ClassImp(STGenfitPIDTask)

STGenfitPIDTask::STGenfitPIDTask()
: STRecoTask("GENFIT Task", 1, false)
{
}

STGenfitPIDTask::STGenfitPIDTask(Bool_t persistence)
: STRecoTask("GENFIT Task", 1, persistence)
{
}

STGenfitPIDTask::~STGenfitPIDTask()
{
}

void STGenfitPIDTask::SetClusteringType(Int_t type) { fClusteringType = type; }
void STGenfitPIDTask::SetConstantField() { fIsSamurai = kFALSE; }

InitStatus
STGenfitPIDTask::Init()
{
  if (STRecoTask::Init() == kERROR)
    return kERROR;

  STDatabasePDG::Instance();

  fHelixTrackArray = (TClonesArray *) fRootManager -> GetObject("STHelixTrack");
  if (fHelixTrackArray == nullptr) {
    LOG(ERROR) << "Cannot find STHelixTrack array!" << FairLogger::endl;
    return kERROR;
  }

  fCandListArray = new TClonesArray("STRecoTrackCandList");
  fRootManager -> Register("STCandList", "SpiRIT", fCandListArray, fIsListPersistence);

  fRecoTrackArray = new TClonesArray("STRecoTrackArray");
  fRootManager -> Register("STRecoTrack", "SpiRIT", fRecoTrackArray, fIsPersistence);

  fGenfitTest = new STGenfitTest2(fIsSamurai);
  fPIDTest = new STPIDTest();

  fVertexArray = new TClonesArray("STVertex");
  fRootManager -> Register("STVertex", "SpiRIT", fVertexArray, fIsPersistence);

  fVertexFactory = new genfit::GFRaveVertexFactory(/* verbosity */ 2, /* useVacuumPropagator */ false);
  //fVertexFactory -> setMethod("kalman-smoothing:1");
  fGFRaveVertexMethod = "avf-smoothing:1-Tini:256-ratio:0.25-sigmacut:5";
  fVertexFactory -> setMethod(fGFRaveVertexMethod.Data());

  if (fRecoHeader != nullptr) {
    fRecoHeader -> SetPar("genfitpid_loadSamuraiMap", fIsSamurai);
    fRecoHeader -> Write("RecoHeader", TObject::kWriteDelete);
  }

  return kSUCCESS;
}

void STGenfitPIDTask::Exec(Option_t *opt)
{
  fCandListArray -> Clear("C");
  fRecoTrackArray -> Clear("C");
  fVertexArray -> Delete();

  if (fEventHeader -> IsBadEvent())
    return;

  fGenfitTest -> Init();

  vector<genfit::Track *> gfTrackArrayToVertex;
  vector<STRecoTrack *> recoTrackArrayToVertex;

  Int_t numTrackCand = fHelixTrackArray -> GetEntriesFast();
  for (Int_t iHelix = 0; iHelix < numTrackCand; iHelix++)
  {
    STHelixTrack *helixTrack = (STHelixTrack *) fHelixTrackArray -> At(iHelix);
    if (helixTrack -> IsHelix() == false)
      continue;

    Int_t trackID = fCandListArray -> GetEntriesFast();

    auto candList = (STRecoTrackCandList *) fCandListArray -> ConstructedAt(trackID);
    auto recoTrack = (STRecoTrack *) fRecoTrackArray -> ConstructedAt(trackID);

    auto clusterArray = helixTrack -> GetClusterArray();
    for (auto cluster : *clusterArray) {
      if (cluster -> IsStable()) {
        candList -> AddHitID(cluster -> GetClusterID());
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
      else if (pid == STPID::kDeuteron) pdg = 1000010020; 
      else if (pid == STPID::kTriton)   pdg = 1000010030; 
      else if (pid == STPID::k3He)      pdg = 1000020030; 
      else if (pid == STPID::k4He)      pdg = 1000020040; 
      else continue;

      auto recoTrackCand = (STRecoTrackCand *) fCandListArray -> At(iPID);

      genfit::Track *gfTrack = fGenfitTest -> FitTrack(helixTrack, pdg);
      if (gfTrack == nullptr) {
        recoTrackCand -> SetPIDProbability(0);
        continue;
      } else {
        recoTrackCand -> SetGenfitTrack(gfTrack);
      }

      TVector3 mom, momTargetPlane, posTargetPlane;
      fGenfitTest -> GetTrackParameters(gfTrack, mom, momTargetPlane, posTargetPlane);
      recoTrackCand -> SetMomentum(mom);
      recoTrackCand -> SetMomentumTargetPlane(momTargetPlane);
      recoTrackCand -> SetPosTargetPlane(posTargetPlane);

      if (fClusteringType == 2)
        fGenfitTest -> GetdEdxPointsByLayerRow(gfTrack, helixTrack, recoTrackCand -> GetdEdxPointArray());
      else
        fGenfitTest -> GetdEdxPointsByLength(gfTrack, helixTrack, recoTrackCand -> GetdEdxPointArray());

      auto prob = fPIDTest -> GetProbability(pid, mom.Mag(), recoTrackCand -> GetdEdxWithCut(0,0.7));
      recoTrackCand -> SetPIDProbability(prob);

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
    recoTrackArrayToVertex.push_back(recoTrack);

    helixTrack -> SetGenfitID(iHelix);
    helixTrack -> SetIsGenfitTrack();
    helixTrack -> SetGenfitMomentum(bestRecoTrackCand -> GetMomentum().Mag());

    TVector3 kyotoL, kyotoR, katana;
    fGenfitTest -> GetPosOnPlanes(bestRecoTrackCand -> GetGenfitTrack(), kyotoL, kyotoR, katana);
    bestRecoTrackCand -> Copy(recoTrack);
    recoTrack -> SetPosKyotoL(kyotoL);
    recoTrack -> SetPosKyotoR(kyotoR);
    recoTrack -> SetPosKatana(katana);
  }

  vector<genfit::GFRaveVertex *> vertices;
  try {
    fVertexFactory -> findVertices(&vertices, gfTrackArrayToVertex);
  } catch (...) {
  }

  Int_t numVertices = vertices.size();

  for (UInt_t iVert = 0; iVert < numVertices; iVert++) {
    genfit::GFRaveVertex* vertex = static_cast<genfit::GFRaveVertex*>(vertices[iVert]);

    Int_t numTracks = vertex -> getNTracks();
    for (Int_t iTrack = 0; iTrack < numTracks; iTrack++) {
      genfit::GFRaveTrackParameters *par = vertex -> getParameters(iTrack);
      const genfit::Track *track = par -> getTrack();

      std::vector<genfit::Track *>::iterator iter = std::find(gfTrackArrayToVertex.begin(), gfTrackArrayToVertex.end(), track);

      if (iter != gfTrackArrayToVertex.end()) {
        Int_t index = std::distance(gfTrackArrayToVertex.begin(), iter);
        auto recoTrack = recoTrackArrayToVertex.at(index);
        genfit::Track *gfTrack = gfTrackArrayToVertex.at(index);

        if (recoTrack -> GetVertexID() < 0) {
          recoTrack -> SetVertexID(iVert);

          TVector3 momVertex;
          Double_t charge;
          fGenfitTest -> GetMomentumWithVertex(gfTrack, 10*vertex->getPos() , momVertex, charge);
          recoTrack -> SetMomentum(momVertex);
          recoTrack -> SetCharge(charge);
        }
      }
    }

    new ((*fVertexArray)[iVert]) STVertex(*vertex);
    delete vertex;
  }

  LOG(INFO) << Space() << "STRecoTrack " << fRecoTrackArray -> GetEntriesFast() << FairLogger::endl;
  LOG(INFO) << Space() << "STVertex " << fVertexArray -> GetEntriesFast() << FairLogger::endl;
}
