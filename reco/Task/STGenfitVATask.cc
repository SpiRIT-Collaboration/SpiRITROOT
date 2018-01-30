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

  if (!fBDCName.IsNull()) {
    fFileBDC = new TFile(fBDCName);
    fTreeBDC = (TTree *) fFileBDC -> Get("TBDC");
    fTreeBDC -> SetBranchAddress("ProjX",&fXBDC);
    fTreeBDC -> SetBranchAddress("ProjY",&fYBDC);
    fTreeBDC -> SetBranchAddress("ProjZ",&fZBDC);
    fTreeBDC -> SetBranchAddress("ProjdX",&fdXBDC);
    fTreeBDC -> SetBranchAddress("ProjdY",&fdYBDC);
    fTreeBDC -> SetBranchAddress("ProjdZ",&fdZBDC);

    fCovMatBDC = new TMatrixDSym(3);
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

  auto vertex = (STVertex *) fVertexArray -> At(chosenVID);

  auto numTracks = fRecoTrackArray -> GetEntriesFast();
  for (auto iTrack = 0; iTrack < numTracks; iTrack++) {
    auto track = (STRecoTrack *) fRecoTrackArray -> At(iTrack);

    if (track -> GetVertexID() != chosenVID)
      continue;

    auto vertexCluster = new STHitCluster();
    vertexCluster -> SetIsStable(kTRUE);
    vertexCluster -> SetHit(-4, vertex -> GetPos(), track -> GetdEdxWithCut(0,1));
    vertexCluster -> SetCovMatrix(vertex -> GetCov());

    Int_t trackID = fCandListArray -> GetEntriesFast();

    auto candList = (STRecoTrackCandList *) fCandListArray -> ConstructedAt(trackID);
    auto vaTrack = (STRecoTrack *) fVATrackArray -> ConstructedAt(trackID);

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

      genfit::Track *gfTrack = fGenfitTest -> FitTrackWithVertex(helixTrack, vertexCluster, pdg);
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

      if (fClusteringType == 2)
        fGenfitTest -> GetdEdxPointsByLayerRow(gfTrack, helixTrack, vaTrackCand -> GetdEdxPointArray());
      else
        fGenfitTest -> GetdEdxPointsByLength(gfTrack, helixTrack, vaTrackCand -> GetdEdxPointArray());

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
    Double_t charge = fGenfitTest -> DetermineCharge(vaTrack, vertex -> GetPos(), effCurvature1, effCurvature2, effCurvature3);
    vaTrack -> SetCharge(charge);
    vaTrack -> SetEffCurvature1(effCurvature1);
    vaTrack -> SetEffCurvature2(effCurvature2);
    vaTrack -> SetEffCurvature3(effCurvature3);
  }

  LOG(INFO) << Space() << "VATrack " << fRecoTrackArray -> GetEntriesFast() << FairLogger::endl;

  if (!fBDCName.IsNull()) {
    fTreeBDC -> GetEntry(fEventHeader -> GetEventID());
    TVector3 posBDC(fXBDC-1.462,fYBDC-235.57,fZBDC+580.4);
    (*fCovMatBDC)[0][0] = fdXBDC;
    (*fCovMatBDC)[1][1] = fdYBDC;
    (*fCovMatBDC)[2][2] = fdZBDC;

//    fVertexFactory -> setBeamspot(posBDC, *fCovMatBDC);
  }

/*
  vector<genfit::GFRaveVertex *> vertices;
  try {
    fVertexFactory -> findVertices(&vertices, gfTrackArrayToVertex, !fBDCName.IsNull());
  } catch (...) {
  }

  Int_t numVertices2 = vertices.size();

  for (UInt_t iVert = 0; iVert < numVertices2; iVert++) {
    genfit::GFRaveVertex* vertex2 = static_cast<genfit::GFRaveVertex*>(vertices[iVert]);

    Int_t numTracks2 = vertex2 -> getNTracks();
    for (Int_t iTrack = 0; iTrack < numTracks2; iTrack++) {
      genfit::GFRaveTrackParameters *par = vertex2 -> getParameters(iTrack);
      const genfit::Track *track = par -> getTrack();

      std::vector<genfit::Track *>::iterator iter = std::find(gfTrackArrayToVertex.begin(), gfTrackArrayToVertex.end(), track);

      if (iter != gfTrackArrayToVertex.end()) {
        Int_t index = std::distance(gfTrackArrayToVertex.begin(), iter);
        auto vaTrack = vaTrackArrayToVertex.at(index);
        genfit::Track *gfTrack = gfTrackArrayToVertex.at(index);

        if (vaTrack -> GetVertexID() < 0) {
          vaTrack -> SetVertexID(iVert);

          TVector3 momVertex;
          TVector3 pocaVertex;
          Double_t charge;
          fGenfitTest -> GetMomentumWithVertex(gfTrack, 10*vertex2->getPos(), momVertex, pocaVertex, charge);
          if (momVertex.Z() < 0)
            momVertex = -momVertex;
          vaTrack -> SetMomentum(momVertex);
          vaTrack -> SetPOCAVertex(pocaVertex);
          vaTrack -> SetCharge(charge);
        }
      }
    }

    new ((*fVertexArray)[iVert]) STVertex(*vertex2);
    delete vertex2;
  }

  LOG(INFO) << Space() << "VAVertex " << fVertexArray -> GetEntriesFast() << FairLogger::endl;
  */
}

void STGenfitVATask::SetBDCFile(TString fileName) { fBDCName = fileName; }
