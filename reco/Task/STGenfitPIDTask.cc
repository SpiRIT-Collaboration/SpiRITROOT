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
void STGenfitPIDTask::SetFieldOffset(Double_t xOffset, Double_t yOffset, Double_t zOffset)
{
  fFieldXOffset = yOffset;
  fFieldYOffset = yOffset;
  fFieldZOffset = zOffset;

  LOG(INFO) << "== [STGenfitPIDTask] Field center : (" << xOffset << ", " << yOffset << ", " << zOffset << ") cm"<< FairLogger::endl;
}

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

  fRecoTrackArray = new TClonesArray("STRecoTrack");
  fRootManager -> Register("STRecoTrack", "SpiRIT", fRecoTrackArray, false);//fIsPersistence);
  fEmbedTrackArray = new TClonesArray("STRecoTrack");
  fRootManager -> Register("STEmbedRecoTrack", "SpiRIT", fEmbedTrackArray, fIsPersistence);


  fGenfitTest = new STGenfitTest2(fIsSamurai, fFieldXOffset, fFieldYOffset, fFieldZOffset);
  fGenfitTest -> SetTargetPlane(fTargetX*0.1, fTargetY*0.1, fTargetZ*0.1); // Target plane position unit mm -> cm
  fPIDTest = new STPIDTest();

  fVertexArray = new TClonesArray("STVertex");
  fRootManager -> Register("STVertex", "SpiRIT", fVertexArray, fIsPersistence);

  if (!fBDCName.IsNull()) {
    fFileBDC = new TFile(fBDCName);
    fTreeBDC = (TTree *) fFileBDC -> Get("TBDC");
    fTreeBDC -> SetBranchAddress("ProjX",&fXBDC);
    fTreeBDC -> SetBranchAddress("ProjY",&fYBDC);
    fTreeBDC -> SetBranchAddress("ProjZ",&fZBDC);
    //fTreeBDC -> SetBranchAddress("ProjdX",&fdXBDC);
    //fTreeBDC -> SetBranchAddress("ProjdY",&fdYBDC);
    //fTreeBDC -> SetBranchAddress("ProjdZ",&fdZBDC);

    fCovMatBDC = new TMatrixDSym(3);
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
  fEmbedTrackArray -> Clear("C");
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
    recoTrack -> SetRecoID(fRecoTrackArray->GetEntries()-1);
    recoTrack -> SetHelixID(iHelix);
    recoTrack -> SetHelixTrack(helixTrack);

    auto clusterArray = helixTrack -> GetClusterArray();
    vector<STHitCluster *> stableClusters;
    for (auto cluster : *clusterArray) {
      if (cluster -> IsStable())
        stableClusters.push_back(cluster);
    }

    auto cleanClusters = [](vector<STHitCluster *> *anArray, Double_t dMax) -> Bool_t {
      if (anArray -> size() <= 1)
	return kTRUE;

      auto numClusters = anArray -> size(); 

      for (auto iCluster = 0; iCluster < numClusters - 1; iCluster++) {
        auto firstCluster = anArray -> at(iCluster);
        auto secondCluster = anArray -> at(iCluster + 1);

        auto dCluster = (secondCluster -> GetPosition() - firstCluster -> GetPosition()).Mag();
        if (dCluster > dMax) {
          auto former = iCluster + 1;
          auto latter = numClusters - iCluster - 1;

//          cout << "former:" << former << " latter:" << latter << " dCluster:" << dCluster << " dMax:" << dMax << endl;

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
//      else if (pid == STPID::kProton)   pdg = 2212; 
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
      recoTrackCand -> SetMomentum(mom);
      recoTrackCand -> SetMomentumTargetPlane(momTargetPlane);
      recoTrackCand -> SetPosTargetPlane(posTargetPlane);

      fGenfitTest -> GetdEdxPointsByLayerRow(gfTrack, helixTrack, recoTrackCand -> GetdEdxPointArray());

      auto prob = 1; // fPIDTest -> GetProbability(pid, mom.Mag(), recoTrackCand -> GetdEdxWithCut(0,0.7));
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
      auto fitState = bestGenfitTrack -> getFittedState();
      TVector3 gfmomReco;
      TVector3 gfposReco(-999,-999,-999);
      TMatrixDSym covMat(6,6);
      fitState.getPosMomCov(gfposReco, gfmomReco, covMat);

      recoTrack -> SetGenfitCharge(gfmomReco.Z() < 0 ? -1 * fitStatus -> getCharge() : fitStatus -> getCharge());
    } catch (...) {
      recoTrack -> SetGenfitCharge(-2);
    }

    try {
      recoTrack -> SetTrackLength(bestGenfitTrack -> getTrackLen());
    } catch (...) {
      recoTrack -> SetTrackLength(-9999);
    }

    Int_t numRowClusters = 0, numRowClusters90 = 0;
    Int_t numLayerClusters = 0, numLayerClusters90 = 0;
    Int_t numEmbedClusters = 0;
    Double_t helixChi2R = 0, helixChi2X = 0, helixChi2Y = 0, helixChi2Z = 0;
    Double_t genfitChi2R = 0, genfitChi2X = 0, genfitChi2Y = 0, genfitChi2Z = 0;
    std::vector<Int_t> fByLayerClusters(112,0); //If cluster in layer 1 else 0
    std::vector<Int_t> fByRowClusters(108,0);   //cluster in row 1 else 0

    for (auto cluster : *helixTrack -> GetClusterArray()) {
      if (!cluster -> IsStable())
        continue;
      
      if( cluster -> IsEmbed())
	numEmbedClusters++;
      
      auto pos = cluster -> GetPosition();
      Int_t clusRow = (pos.X() + 8*54.)/8;
      Int_t clusLayer = pos.Z()/12;
      if(clusLayer > 111) clusLayer = 111;
      if (cluster -> IsRowCluster())
	{
	  numRowClusters++;
	  if(cluster->GetRow() <= 108) {
//	    fByRowClusters.at(cluster->GetRow()) = 1;
	    fByRowClusters.at(clusRow) += 1;
	    fByLayerClusters.at(clusLayer) += 1;
    } else
	    cout<<"ERROR IDX OF ROW WRONG"<<endl;
	  
	  if (pos.Z() < 1080)
	    numRowClusters90++;
	}
      else if (cluster -> IsLayerCluster())
	{
	  numLayerClusters++;
	  if(cluster->GetLayer() <= 112 ) {
//	    fByLayerClusters.at(cluster->GetLayer()) = 1;
	    fByRowClusters.at(clusRow) += 1;
	    fByLayerClusters.at(clusLayer) += 1;
    } else
	    cout<<"EROOR IDX OF LAYER WRONG "<<endl;
	  
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
    recoTrack -> SetNumEmbedClusters(numEmbedClusters);
    recoTrack -> SetNumRowClusters(numRowClusters);
    recoTrack -> SetNumLayerClusters(numLayerClusters);
    recoTrack -> SetNumRowClusters90(numRowClusters90);
    recoTrack -> SetNumLayerClusters90(numLayerClusters90);
    recoTrack -> SetRowVec(fByRowClusters);
    recoTrack -> SetLayerVec(fByLayerClusters);
    recoTrack -> SetHelixChi2R(helixChi2R);
    recoTrack -> SetHelixChi2X(helixChi2X);
    recoTrack -> SetHelixChi2Y(helixChi2Y);
    recoTrack -> SetHelixChi2Z(helixChi2Z);
    recoTrack -> SetChi2R(genfitChi2R);
    recoTrack -> SetChi2X(genfitChi2X);
    recoTrack -> SetChi2Y(genfitChi2Y);
    recoTrack -> SetChi2Z(genfitChi2Z);
  }

  LOG(INFO) << Space() << "STRecoTrack " << fRecoTrackArray -> GetEntriesFast() << FairLogger::endl;

  if (gfTrackArrayToVertex.size() < 2) {
    for (auto recoTrack : recoTrackArrayToVertex)
      recoTrack -> SetCharge(1);
    return;
  }

  if (!fBDCName.IsNull()) {
    fTreeBDC -> GetEntry(fEventHeader -> GetEventID());
    TVector3 posBDC(0.1*fXBDC,0.1*(fYBDC-227),0.1*(fZBDC+580.4));
    //(*fCovMatBDC)[0][0] = fdXBDC;
    //(*fCovMatBDC)[1][1] = fdYBDC;
    //(*fCovMatBDC)[2][2] = fdZBDC;
    (*fCovMatBDC)[0][0] = 0.01*5.9*5.9;
    (*fCovMatBDC)[1][1] = 0.01*4.5*4.5;
    (*fCovMatBDC)[2][2] = 0.01*2.8*2.8;

    fVertexFactory -> setBeamspot(posBDC, *fCovMatBDC);
  }

  vector<genfit::GFRaveVertex *> vertices;
  try {
    fVertexFactory -> findVertices(&vertices, gfTrackArrayToVertex, !fBDCName.IsNull());
  } catch (...) {
  }

  Int_t numVertices = vertices.size();
  Int_t ngtrk = 0;
  LOG(INFO) << Space() << "vector verticies " << vertices.size() << FairLogger::endl;

  for (UInt_t iVert = 0; iVert < numVertices; iVert++) {
    genfit::GFRaveVertex* vertex = static_cast<genfit::GFRaveVertex*>(vertices[iVert]);
    LOG(INFO) << Space() << "vtxx " << vertex->getPos().X() 
	      << " vtxy " << vertex->getPos().Y() 
	      << " vtxz " << vertex->getPos().Z() 
	      << FairLogger::endl;

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
	  if(0 == iVert)
	    if(((10*vertex->getPos()) - pocaVertex).Mag() < 20) ngtrk ++;

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

  // recording only embedded tracks
  Int_t numTrk = fRecoTrackArray -> GetEntriesFast();
  Int_t netrk = 0;
  for (Int_t i = 0; i < numTrk; i++){
    STRecoTrack *trk = (STRecoTrack *) fRecoTrackArray -> At(i);
    if(trk->GetNumEmbedClusters()>0){
      auto newtrk = (STRecoTrack *) fEmbedTrackArray -> ConstructedAt(netrk);
      
      // from STRecoTrackCand
      newtrk -> SetPID(trk->GetPID());
      newtrk -> SetPIDProbability(trk->GetPIDProbability());
      newtrk -> SetMomentum(trk->GetMomentum());
      newtrk -> SetMomentumTargetPlane(trk->GetMomentumTargetPlane());
      newtrk -> SetPosTargetPlane(trk->GetPosTargetPlane());
      newtrk -> SetIsEmbed(trk->IsEmbed());
      newtrk -> SetNumEmbedClusters(trk->GetNumEmbedClusters());

      auto inarray = trk -> GetdEdxPointArray();
      auto newarray = newtrk -> GetdEdxPointArray();
      for (auto dedx : *inarray) {
	newtrk -> AdddEdxPoint(dedx);
      }

      // from STRecoTrack
      newtrk -> SetCharge(trk->GetCharge());
      newtrk -> SetParentID(trk->GetParentID());
      newtrk -> SetVertexID(trk->GetVertexID());
      newtrk -> SetHelixID(trk->GetHelixID());
      newtrk -> SetPOCAVertex(trk->GetPOCAVertex());
      newtrk -> SetPosKyotoL(trk->GetPosKyotoL());
      newtrk -> SetPosKyotoR(trk->GetPosKyotoR());
      newtrk -> SetPosKatana(trk->GetPosKatana());
      newtrk -> SetPosNeuland(trk->GetPosNeuland());

      auto inclid = trk -> GetClusterIDArray();
      for (auto ii : *inclid) {
	newtrk -> AddClusterID(ii);
      }

      newtrk -> SetEffCurvature1(trk->GetEffCurvature1());
      newtrk -> SetEffCurvature2(trk->GetEffCurvature2());
      newtrk -> SetEffCurvature3(trk->GetEffCurvature3());
      newtrk -> SetNumRowClusters(trk->GetNumRowClusters());
      newtrk -> SetNumLayerClusters(trk->GetNumLayerClusters());
      newtrk -> SetNumRowClusters90(trk->GetNumRowClusters90());
      newtrk -> SetNumLayerClusters90(trk->GetNumLayerClusters90());
      newtrk -> SetHelixChi2R(trk->GetHelixChi2R());
      newtrk -> SetHelixChi2X(trk->GetHelixChi2X());
      newtrk -> SetHelixChi2Y(trk->GetHelixChi2Y());
      newtrk -> SetHelixChi2Z(trk->GetHelixChi2Z());
      newtrk -> SetTrackLength(trk->GetTrackLength());
      newtrk -> SetNDF(trk->GetNDF());
      newtrk -> SetChi2(trk->GetChi2());
      newtrk -> SetChi2R(trk->GetChi2R());
      newtrk -> SetChi2X(trk->GetChi2X());
      newtrk -> SetChi2Y(trk->GetChi2Y());
      newtrk -> SetChi2Z(trk->GetChi2Z());
      newtrk -> SetRecoID(trk->GetRecoID());

      netrk ++;
    }
  }

  fEventHeader -> SetNRecoTrk(fRecoTrackArray->GetEntries());
  fEventHeader -> SetNGoodRecoTrk(ngtrk);

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
