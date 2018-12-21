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

  fHitClusterArray = (TClonesArray *) fRootManager -> GetObject("STHitCluster");
  if (fHitClusterArray == nullptr) {
    LOG(ERROR) << "Cannot find STHitCluster array!" << FairLogger::endl;
    return kERROR;
  }

  fCandListArray = new TClonesArray("STRecoTrackCandList");
  fRootManager -> Register("STCandList", "SpiRIT", fCandListArray, fIsListPersistence);

  fRecoTrackArray = new TClonesArray("STRecoTrack");
  fRootManager -> Register("STRecoTrack", "SpiRIT", fRecoTrackArray, fIsPersistence);

  fGenfitTest = new STGenfitTest2(fIsSamurai);
  fGenfitTest -> SetTargetPlane(fTargetX*0.1, fTargetY*0.1, fTargetZ*0.1); // Target plane position unit mm -> cm
  fPIDTest = new STPIDTest();

  fVertexArray = new TClonesArray("STVertex");
  fRootManager -> Register("STVertex", "SpiRIT", fVertexArray, fIsPersistence);

  if (!fBDCName.IsNull()) {
    fBDCFile = new TFile(fBDCName);
    fBDCTree = (TTree *) fBDCFile -> Get("TBDC");

    fBDCTree -> SetBranchAddress("ProjP", &fPBDC);
    fBDCTree -> SetBranchAddress("bdc1x", &fBDC1x);
    fBDCTree -> SetBranchAddress("bdc1y", &fBDC1y);
    fBDCTree -> SetBranchAddress("bdc2x", &fBDC2x);
    fBDCTree -> SetBranchAddress("bdc2y", &fBDC2y);
    fBDCTree -> SetBranchAddress("bdcax", &fBDCax);
    fBDCTree -> SetBranchAddress("bdcby", &fBDCby);

    //fBeamFile = new TFile(fBeamFilename);
    fBeamTree = (TTree *) fBDCFile -> Get("TBeam");
    fBeamTree -> SetBranchAddress("z", &fZ);
    fBeamTree -> SetBranchAddress("aoq", &fAoQ);
    fBeamTree -> SetBranchAddress("beta37", &fBeta37);
    fBeamTree -> SetBranchAddress("intA",&fABeam);

    fCovMatBDC = new TMatrixDSym(3);

    fBeamEnergy = new STBeamEnergy();
    fBeamEnergy -> setBeam(fRunNo);
    fBDCProjection = new STBDCProjection(TString(gSystem -> Getenv("VMCWORKDIR")) + "/parameters/ReducedBMap.txt");
    fBDCProjection -> setBeam(fRunNo);
  }

  fVertexFactory = new genfit::GFRaveVertexFactory(/* verbosity */ 2, /* useVacuumPropagator */ false);
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
    recoTrack -> SetHelixID(iHelix);
    recoTrack -> SetHelixTrack(helixTrack);

    auto clusterArray = helixTrack -> GetClusterArray();
    vector<STHitCluster *> stableClusters;
    for (auto cluster : *clusterArray) {
      if (cluster -> IsStable())
        stableClusters.push_back(cluster);
    }

    auto cleanClusters = [](vector<STHitCluster *> *anArray, Double_t dMax) -> Bool_t {
      if (anArray -> size() == 1)
        return kTRUE;

      auto numClusters = anArray -> size(); 

      for (auto iCluster = 0; iCluster < numClusters - 1; iCluster++) {
        auto firstCluster = anArray -> at(iCluster);
        auto secondCluster = anArray -> at(iCluster + 1);

        auto dCluster = (secondCluster -> GetPosition() - firstCluster -> GetPosition()).Mag();
        if (dCluster > dMax) {
          auto former = iCluster + 1;
          auto latter = numClusters - iCluster - 1;

          if (former > latter) {
            while (latter) {
              anArray -> back() -> SetIsStable(kFALSE);
              anArray -> pop_back();
              latter--;
            }

            return kFALSE;
          } else if (former <= latter) {
            while (former) {
              anArray -> front() -> SetIsStable(kFALSE);
              anArray -> erase(anArray -> begin());
              former--;
            }

            return kFALSE;
          }
        }
      }

      return kTRUE;
    };

    while (!cleanClusters(&stableClusters, fMaxDCluster)) {}

    for (auto cluster : stableClusters) {
      candList -> AddHitID(cluster -> GetClusterID());
      recoTrack -> AddClusterID(cluster -> GetClusterID());
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

      auto recoTrackCand = candList -> GetRecoTrackCand(iPID);

      genfit::Track *gfTrack = fGenfitTest -> FitTrack(helixTrack, pdg);
      if (gfTrack == nullptr) {
        recoTrackCand -> SetPIDProbability(0);
        continue;
      } else {
        recoTrackCand -> SetGenfitTrack(gfTrack);
      }

      TVector3 mom, momTargetPlane, posTargetPlane;
      fGenfitTest -> GetTrackParameters(gfTrack, mom, momTargetPlane, posTargetPlane);
      if (mom.Z() < 0)
        mom = -mom;
      if (momTargetPlane.Z() < 0)
        momTargetPlane = -momTargetPlane;
      recoTrackCand -> SetMomentum(mom);
      recoTrackCand -> SetMomentumTargetPlane(momTargetPlane);
      recoTrackCand -> SetPosTargetPlane(posTargetPlane);

      fGenfitTest -> GetdEdxPointsByLayerRow(gfTrack, helixTrack, recoTrackCand -> GetdEdxPointArray());

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
    auto bestGenfitTrack = bestRecoTrackCand -> GetGenfitTrack();
    gfTrackArrayToVertex.push_back(bestGenfitTrack);
    recoTrackArrayToVertex.push_back(recoTrack);

    helixTrack -> SetGenfitID(trackID);
    helixTrack -> SetIsGenfitTrack();
    helixTrack -> SetGenfitMomentum(bestRecoTrackCand -> GetMomentum().Mag());

    TVector3 kyotoL, kyotoR, katana, neuland;
    fGenfitTest -> GetPosOnPlanes(bestRecoTrackCand -> GetGenfitTrack(), kyotoL, kyotoR, katana, neuland);
    bestRecoTrackCand -> Copy(recoTrack);
    recoTrack -> SetPosKyotoL(kyotoL);
    recoTrack -> SetPosKyotoR(kyotoR);
    recoTrack -> SetPosKatana(katana);
    recoTrack -> SetPosNeuland(neuland);

    auto fitStatus = bestGenfitTrack -> getFitStatus(bestGenfitTrack -> getTrackRep(0));
    recoTrack -> SetChi2(fitStatus -> getChi2());
    recoTrack -> SetNDF(fitStatus -> getNdf());
    try {
      recoTrack -> SetTrackLength(bestGenfitTrack -> getTrackLen());
    } catch (...) {
      recoTrack -> SetTrackLength(-9999);
    }

    Int_t numRowClusters = 0, numRowClusters90 = 0;
    Int_t numLayerClusters = 0, numLayerClusters90 = 0;
    Double_t helixChi2R = 0, helixChi2X = 0, helixChi2Y = 0, helixChi2Z = 0;
    Double_t genfitChi2R = 0, genfitChi2X = 0, genfitChi2Y = 0, genfitChi2Z = 0;
    for (auto cluster : *helixTrack -> GetClusterArray()) {
      if (!cluster -> IsStable())
        continue;

      auto pos = cluster -> GetPosition();
      if (cluster -> IsRowCluster()) {
        numRowClusters++;
        if (pos.Z() < 1080)
          numRowClusters90++;
      } else if (cluster -> IsLayerCluster()) {
        numLayerClusters++;
        if (pos.Z() < 1080)
          numLayerClusters90++;
      }

      TVector3 dVec, point;
      Double_t dValue, alpha;
      helixTrack -> ExtrapolateToPointAlpha(pos, point, alpha);
      helixChi2R += (point - pos).Mag2();
      helixChi2X += (point.X() - pos.X())*(point.X() - pos.X());
      helixChi2Y += (point.Y() - pos.Y())*(point.Y() - pos.Y());
      helixChi2Z += (point.Z() - pos.Z())*(point.Z() - pos.Z());
      /*
      helixTrack -> ExtrapolateToX(pos.X(), point, alpha, dVec, dValue);
      helixChi2X += (point.X() - pos.X())*(point.X() - posX());
      helixTrack -> ExtrapolateToY(pos.Y(), point, alpha, dVec, dValue);
      helixChi2Y += (point.Y() - pos.Y())*(point.Y() - posY());
      helixTrack -> ExtrapolateToZ(pos.Z(), point, alpha, dVec, dValue);
      helixChi2Z += (point.Z() - pos.Z())*(point.Z() - posZ());
      */

      if (!fGenfitTest -> ExtrapolateTo(bestGenfitTrack, pos, point))
        continue;

      genfitChi2R += (point - pos).Mag2();
      genfitChi2X += (point.X() - pos.X())*(point.X() - pos.X());
      genfitChi2Y += (point.Y() - pos.Y())*(point.Y() - pos.Y());
      genfitChi2Z += (point.Z() - pos.Z())*(point.Z() - pos.Z());
      /*
      fGenfitTest -> GetPosOnPlane(bestRecoTrackCand -> GetGenfitTrack(), pos, 1, point);
      genfitChi2X += (point.X() - pos.X())*(point.X() - pos.X());
      fGenfitTest -> GetPosOnPlane(bestRecoTrackCand -> GetGenfitTrack(), pos, 2, point);
      genfitChi2Y += (point.Y() - pos.Y())*(point.Y() - pos.Y());
      fGenfitTest -> GetPosOnPlane(bestRecoTrackCand -> GetGenfitTrack(), pos, 3, point);
      genfitChi2Z += (point.Z() - pos.Z())*(point.Z() - pos.Z());
      */
    }
    recoTrack -> SetNumRowClusters(numRowClusters);
    recoTrack -> SetNumLayerClusters(numLayerClusters);
    recoTrack -> SetNumRowClusters90(numRowClusters90);
    recoTrack -> SetNumLayerClusters90(numLayerClusters90);
    recoTrack -> SetHelixChi2R(helixChi2R);
    recoTrack -> SetHelixChi2X(helixChi2X);
    recoTrack -> SetHelixChi2Y(helixChi2Y);
    recoTrack -> SetHelixChi2Z(helixChi2Z);
    recoTrack -> SetChi2R(genfitChi2R);
    recoTrack -> SetChi2X(genfitChi2X);
    recoTrack -> SetChi2Y(genfitChi2Y);
    recoTrack -> SetChi2Z(genfitChi2Z);
  }

  Int_t numRecoTracks = fRecoTrackArray -> GetEntriesFast();
  LOG(INFO) << Space() << "STRecoTrack " << numRecoTracks << FairLogger::endl;

  if (gfTrackArrayToVertex.size() < 2) {
    for (auto recoTrack : recoTrackArrayToVertex)
      recoTrack -> SetCharge(1);
    return;
  }

  if (!fBDCName.IsNull()) {
    fBDCTree -> GetEntry(fEventHeader -> GetEventID() -1);
    fBeamTree -> GetEntry(fEventHeader -> GetEventID() -1);

    Int_t idxHelix = fHelixTrackArray -> GetEntries();
    STHelixTrack *bdcHelix = new ((*fHelixTrackArray)[idxHelix]) STHelixTrack(idxHelix);
    bdcHelix -> SetTrackID(idxHelix);
    bdcHelix -> SetTrackID(fABeam);

    auto bdcTrack = (STRecoTrack *) fRecoTrackArray -> ConstructedAt(numRecoTracks);
    bdcTrack -> SetRecoID(-1);
    bdcTrack -> SetHelixID(idxHelix);
    bdcTrack -> SetHelixTrack(bdcHelix);

    Double_t bdcYOffset = -224.1573;

    {
      fBeamEnergy -> reset(fZ, fAoQ, fBeta37);
      Double_t E1 = fBeamEnergy -> getCorrectedEnergy();
      // mid target = -592.644, start pad plane =-580.4, end of pad plane = 763.6

      Int_t idxCluster = fHitClusterArray -> GetEntries();
      for (auto bdcz=-500.; bdcz<0; bdcz+=20.)
      {
        fBDCProjection -> ProjectParticle(fBDC2x, fBDC2y, -2160., fBDCax, fBDCby, fZ, E1, bdcz - 580.4, fBeamEnergy -> getMass());//-580.4,-583.904
        auto bdcpos = TVector3(fBDCProjection -> getX() + fOffsetX, fBDCProjection -> getY() + bdcYOffset, bdcz);

        auto cluster = new ((*fHitClusterArray)[idxCluster]) STHitCluster();
        cluster -> SetClusterID(idxCluster);
        cluster -> SetIsStable(true);
        cluster -> SetCharge(-50);
        cluster -> SetPosition(bdcpos);
        bdcHelix -> AddHitCluster(cluster);
        bdcTrack -> AddClusterID(cluster -> GetClusterID());

        cout << setw(15) << bdcpos.X() << setw(15) << bdcpos.Y() << setw(15) << bdcpos.Z() << endl;

        idxCluster++;
      }
    }

    genfit::Track *gfBDCTrack = fGenfitTest -> FitBDC(bdcHelix, 1000501320);
    if (gfBDCTrack == nullptr) {
      cout << "*******************************************" << endl;
      cout << Space() << "BDC Track is nullptr" << endl;
      cout << "*******************************************" << endl;
      bdcTrack -> SetPIDProbability(0);
    } else {
      cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << endl;
      cout << Space() << "BDC Track is GOOD!!!" << endl;
      cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << endl;
      //gfBDCTrack -> Print();
      bdcTrack -> SetGenfitTrack(gfBDCTrack);
      bdcTrack -> SetPIDProbability(100);

      gfTrackArrayToVertex.push_back(gfBDCTrack);
      recoTrackArrayToVertex.push_back(bdcTrack);

      TVector3 mom, momTargetPlane, posTargetPlane;
      fGenfitTest -> GetTrackParameters(gfBDCTrack, mom, momTargetPlane, posTargetPlane);
      if (mom.Z() < 0)
        mom = -mom;
      if (momTargetPlane.Z() < 0)
        momTargetPlane = -momTargetPlane;
      bdcTrack -> SetMomentum(mom);
      bdcTrack -> SetMomentumTargetPlane(momTargetPlane);
      bdcTrack -> SetPosTargetPlane(posTargetPlane);

      bdcHelix -> SetGenfitMomentum(mom.Mag());
    }
  }

  vector<genfit::GFRaveVertex *> vertices;
  try {
    fVertexFactory -> findVertices(&vertices, gfTrackArrayToVertex, !fBDCName.IsNull());
  } catch (...) {
  }

  Int_t numVertices = vertices.size();
  LOG(INFO) << Space() << "vector verticies " << vertices.size() << FairLogger::endl;

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
          TVector3 pocaVertex;
          fGenfitTest -> GetMomentumWithVertex(gfTrack, 10*vertex->getPos(), momVertex, pocaVertex);
          if (momVertex.Z() < 0)
            momVertex = -momVertex;
          recoTrack -> SetMomentum(momVertex);
          recoTrack -> SetPOCAVertex(pocaVertex);

          Double_t effCurvature1;
          Double_t effCurvature2;
          Double_t effCurvature3;
          Double_t charge = fGenfitTest -> DetermineCharge(recoTrack, vertex -> getPos(), effCurvature1, effCurvature2, effCurvature3);
          recoTrack -> SetCharge(charge);
          recoTrack -> SetEffCurvature1(effCurvature1);
          recoTrack -> SetEffCurvature2(effCurvature2);
          recoTrack -> SetEffCurvature3(effCurvature3);
        }
      }
    }

    new ((*fVertexArray)[iVert]) STVertex(*vertex);
    delete vertex;
  }

  LOG(INFO) << Space() << "STVertex " << fVertexArray -> GetEntriesFast() << FairLogger::endl;
}

void STGenfitPIDTask::SetBDCFile(TString fileName) { fBDCName = fileName; }

void STGenfitPIDTask::SetTargetPlane(Double_t x, Double_t y, Double_t z)
{
  fTargetX = x;
  fTargetY = y;
  fTargetZ = z;
}

genfit::GFRaveVertexFactory *STGenfitPIDTask::GetVertexFactoryInstance()
{
  return fVertexFactory;
}
