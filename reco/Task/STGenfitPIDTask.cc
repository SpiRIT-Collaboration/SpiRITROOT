#include "STGenfitPIDTask.hh"
#include "STDatabasePDG.hh"
#include "STTrack.hh"

#include "FairRun.h"
#include "FairRuntimeDb.h"
#include "FairRootManager.h"

#include "Track.h"
#include "TParameter.h"

#include <vector>
#include <iostream>
using namespace std;

#include "KalmanFitterInfo.h"
#include "KalmanFittedStateOnPlane.h"
#include "MeasuredStateOnPlane.h"
#include "AbsMeasurement.h"

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
  fRootManager -> Register("STRecoTrack", "SpiRIT", fRecoTrackArray, fIsPersistence);

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
  if (fGFRaveVertexMethod.IsNull())
    fGFRaveVertexMethod = "avf-smoothing:1-Tini:256-ratio:0.25-sigmacut:5";
  LOG(INFO) << "Vertex Mehod " << fGFRaveVertexMethod << FairLogger::endl;
  fVertexFactory -> setMethod(fGFRaveVertexMethod.Data());

  if (fRecoHeader != nullptr) {
    fRecoHeader -> SetPar("genfitpid_loadSamuraiMap", fIsSamurai);
    fRecoHeader -> Write("RecoHeader", TObject::kWriteDelete);
  }

  if (!fSigFileName.IsNull()) {
    auto fileSigma = new TFile(TString(gSystem -> Getenv("VMCWORKDIR")) + "/input/" + fSigFileName);
    for (auto isLR : {0,1})
      for (auto ixyz : {0,1,2})
        for (auto idxN : {0,1,2})
          fHitClusterSigma[isLR][ixyz][idxN] = (TF1 *) fileSigma -> Get(Form("fit_%s_%s_n%d",(isLR?"lay":"row"),(ixyz==0?"x":(ixyz==1?"y":"z")),idxN+2));
  }

  if (!fNameCluster.IsNull())
  {
    fFileCluster = new TFile(fNameCluster,"recreate");
    fTreeCluster = new TTree("data","");
    TString header1 = TString("residual_");
    TString header2 = TString("standardized_residual_");
    for (auto isLR : {0,1}) {
      TString nameLR = isLR?"layer_cluster":"row_cluster";
      for (auto ixyz : {0,1,2}) {
        TString nameXYZ = ixyz==0?"x":(ixyz==1?"y":"z");
        double max = 4.;
        if ((isLR==0&&ixyz==0) || (isLR==1&&ixyz==2))
          max = .04;
        for (auto idxNH : {0,1,2}) {
          TString nameN = Form("n%d",idxNH);
          TString name1 = header1 + nameLR + "_" + nameXYZ + "_" + nameN;
          TString name2 = header2 + nameLR + "_" + nameXYZ + "_" + nameN;
          fHistRawResiduals[isLR][ixyz][idxNH] = new TH1D(name1,name1+";"+header1+nameXYZ,200,-max,max);
          fHistStdResiduals[isLR][ixyz][idxNH] = new TH1D(name2,name1+";"+header2+nameXYZ,200,-max,max);
        }
      }
    }
    fTreeCluster -> Branch("isLR",&fClusterIsLayerOrRow);
    fTreeCluster -> Branch("rx",&fClusterResidualX);
    fTreeCluster -> Branch("ry",&fClusterResidualY);
    fTreeCluster -> Branch("rz",&fClusterResidualZ);
    //fTreeCluster -> Branch("theta",&fTrackTheta);
    fTreeCluster -> Branch("chi",&fClusterChi);
    fTreeCluster -> Branch("dip",&fClusterDip);
    fTreeCluster -> Branch("n",&fClusterNumHits);
    fTreeCluster -> Branch("x",&fClusterX);
    fTreeCluster -> Branch("z",&fClusterZ);
    //fTreeCluster -> Branch("p",&fTrackP);
    //fTreeCluster -> Branch("dedx",&fTrackdEdx);
  }

  if (!fNameTrack.IsNull())
  {
    fFileTrack = new TFile(fNameTrack,"recreate");
    fTreeTrack = new TTree("data","");
    fTreeTrack -> Branch("pvalue",&fTrackPValue);
    fTreeTrack -> Branch("weight",&fTrackWeight);
    fTreeTrack -> Branch("p",&fTrackP);
    fTreeTrack -> Branch("theta",&fTrackTheta);
    fTreeTrack -> Branch("phi",&fTrackPhi);
    fTreeTrack -> Branch("dedx",&fTrackdEdx);
    fTreeTrack -> Branch("chi2",&fTrackChi2);
    fTreeTrack -> Branch("ndf",&fTrackNDF);
    fTreeTrack -> Branch("d",&fTrackDist);
    fTreeTrack -> Branch("z",&fTrackVertexZ);
    fTreeTrack -> Branch("n",&fTrackNumClusters);
  }

  if (!fDebugVertexFileName.IsNull()) {
    fDebugVertexFile.open(fDebugVertexFileName.Data());
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
    recoTrack -> SetRecoID(fRecoTrackArray->GetEntries()-1);
    recoTrack -> SetHelixID(iHelix);
    recoTrack -> SetHelixTrack(helixTrack);

    auto clusterArray = helixTrack -> GetClusterArray();
    vector<STHitCluster *> stableClusters;
    for (auto cluster : *clusterArray) {

      if (fUseConstCov) {
        cluster -> SetDx(fCovX);
        cluster -> SetDy(fCovY);
        cluster -> SetDz(fCovZ);
      }
      else {
        auto chi = cluster -> GetChi() * TMath::RadToDeg();
        auto dip = cluster -> GetLambda() * TMath::RadToDeg();
        auto numHits = cluster -> GetNumHits();

        int idxN;
        /**/ if (numHits<=2) idxN = 0;
        else if (numHits==3) idxN = 1;
        else /* numHits>=4*/ idxN = 2;

        Double_t sig;
        if (cluster -> IsLayerCluster()) {
          sig = fHitClusterSigma[1][0][idxN] -> Eval(chi); cluster->SetDx(sig*sig);
          sig = fHitClusterSigma[1][1][idxN] -> Eval(dip); cluster->SetDy(sig*sig);
          sig = fHitClusterSigma[1][2][idxN] -> Eval(chi); cluster->SetDz(sig*sig);
        }
        else {
          sig = fHitClusterSigma[0][0][idxN] -> Eval(chi); cluster->SetDx(sig*sig);
          sig = fHitClusterSigma[0][1][idxN] -> Eval(dip); cluster->SetDy(sig*sig);
          sig = fHitClusterSigma[0][2][idxN] -> Eval(chi); cluster->SetDz(sig*sig);
        }
      }

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
      else if (pid == STPID::kProton)   pdg = 2212;
//      else if (pid == STPID::kDeuteron) pdg = 1000010020;
//      else if (pid == STPID::kTriton)   pdg = 1000010030;
      //else if (pid == STPID::k3He)      pdg = 1000020030;
      //else if (pid == STPID::k4He)      pdg = 1000020040;
      else continue;

      if (fSelectPID<0) {}
      else if (fSelectPID!=pdg) continue;

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
    auto bestGenfitTrack = bestRecoTrackCand -> GetGenfitTrack(); // genfit::Track
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

    auto fitStatus = bestGenfitTrack -> getFitStatus(bestGenfitTrack -> getTrackRep(0)); // genfit::FitStatus
    recoTrack -> SetChi2(fitStatus -> getChi2());
    recoTrack -> SetNDF(fitStatus -> getNdf());
    recoTrack -> SetGenfitPValue(fitStatus -> getPVal());

    if (fPrintFittedPoints) {
      auto numPoints = bestGenfitTrack -> getNumPoints();
      auto numClusters = stableClusters.size();
      cout << 999999999 << endl;
      cout << helixTrack -> GetTrackID() << " " << numPoints << " " << numClusters << endl;


      if (numPoints == numClusters)
        for (auto iPoint=0; iPoint<numPoints; ++iPoint)
        {
          auto gfPoint = bestGenfitTrack -> getPoint(iPoint);
          auto fitterInfo = (genfit::KalmanFitterInfo*) gfPoint -> getFitterInfo();

          // genfit::KalmanFittedStateOnPlane *
          auto kalOnPlane = fitterInfo -> getBackwardUpdate();
          auto chi2KOP = kalOnPlane -> getChiSquareIncrement();
          auto ndfKOP = kalOnPlane -> getNdf();
          auto covKOP = kalOnPlane -> getCov();

          //genfit::MeasuredStateOnPlane
          auto fittedState = fitterInfo -> getFittedState();
          auto posFit = fittedState.getPos();
          auto covFit = fittedState.getCov();

          // genfit::AbsMeasurement
          auto rawMeasurement = gfPoint -> getRawMeasurement();
          auto posRaw = rawMeasurement -> getRawHitCoords(); // TVectorD
          auto covRaw = rawMeasurement -> getRawHitCov();      // TMatrixDSym

          // STHitCluster Extrapolate to plane
          auto cluster = stableClusters.at(iPoint);
          auto posExt = .1* cluster -> GetPOCA();

          cout << setw(15) << posRaw[0] << setw(15) << posRaw[1] << setw(15) << posRaw[2] << endl;
          cout << setw(15) << posFit.X() << setw(15) << posFit.Y() << setw(15) << posFit.Z() << endl;
          cout << setw(15) << posExt.X() << setw(15) << posExt.Y() << setw(15) << posExt.Z() << endl;

          cout << setw(15) << covRaw[0][0] << setw(15) << covRaw[1][1] << setw(15) << covRaw[2][2]
               << setw(15) << covRaw[0][1] << setw(15) << covRaw[0][2] << setw(15) << covRaw[2][1] << endl;
          cout << setw(15) << covFit[0][0] << setw(15) << covFit[1][1] << setw(15) << covFit[2][2]
               << setw(15) << covFit[0][1] << setw(15) << covFit[0][2] << setw(15) << covFit[2][1] << endl;
          cout << setw(15) << covKOP[0][0] << setw(15) << covKOP[1][1] << setw(15) << covKOP[2][2]
               << setw(15) << covKOP[0][1] << setw(15) << covKOP[0][2] << setw(15) << covKOP[2][1] << endl;

          cout << setw(15) << chi2KOP << setw(15) << ndfKOP << endl;
        }

      cout << fitStatus -> getChi2() << " " << fitStatus -> getNdf() << " " << fitStatus -> getPVal() << endl;
    }

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
  LOG(INFO) << Space() << "vector verticies " << vertices.size() << FairLogger::endl;

  for (UInt_t iVert = 0; iVert < numVertices; iVert++) {
    genfit::GFRaveVertex* vertex = static_cast<genfit::GFRaveVertex*>(vertices[iVert]);

    Int_t numTracks = vertex -> getNTracks();
    if (!fDebugVertexFileName.IsNull()) {
      auto posv = 10 * vertex -> getPos();
      auto eventID = fEventHeader -> GetEventID();
      fDebugVertexFile << eventID << " " << posv.x() << " " << posv.y() << " " << posv.z() << endl;
      fDebugVertexFile << numTracks << endl;
    }

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
          recoTrack -> SetVertexWeight(par -> getWeight());

          Double_t effCurvature1;
          Double_t effCurvature2;
          Double_t effCurvature3;
          Double_t charge = fGenfitTest -> DetermineCharge(recoTrack, vertex -> getPos(), effCurvature1, effCurvature2, effCurvature3);
          recoTrack -> SetCharge(charge);
          recoTrack -> SetEffCurvature1(effCurvature1);

          {
            auto mom = recoTrack -> GetMomentum();
            auto posv = 10 * vertex -> getPos();
            auto poca = recoTrack -> GetPOCAVertex();

            fTrackPValue = recoTrack -> GetGenfitPValue();
            fTrackWeight = recoTrack -> GetVertexWeight();
            fTrackP = mom.Mag();
            fTrackTheta = mom.Theta();
            fTrackPhi = mom.Phi();
            fTrackdEdx = recoTrack -> GetdEdxWithCut(0,0.7);
            fTrackChi2 = recoTrack -> GetChi2();
            fTrackNDF  = recoTrack -> GetNDF();
            fTrackDist = (poca - posv).Mag();
            fTrackVertexZ = posv.Z();
            fTrackNumClusters = recoTrack -> GetNumLayerClusters() + recoTrack -> GetNumRowClusters();

            auto postv = par -> getPos();

            if (!fNameTrack.IsNull())
              fTreeTrack -> Fill();

            if (!fDebugVertexFileName.IsNull() ||
              (!fNameCluster.IsNull()
              && fTrackNumClusters > 20
              && fTrackDist < 10
              && fTrackP > 2
              && fTrackP < 3000
              && fTrackdEdx > 1
              && fTrackdEdx < 2000
              && fTrackVertexZ < 0
              && fTrackVertexZ > -20)
              )
            {
              auto helixTrack = recoTrack -> GetHelixTrack();
              auto clusterArray = helixTrack -> GetClusterArray();

              auto numClusters = clusterArray -> size();
              Int_t idxStable = 0;
              for (auto iCluster=0; iCluster<numClusters; ++iCluster)
              {
                auto cluster = clusterArray -> at(iCluster);
                if (!cluster -> IsStable())
                  continue;

                auto pos = cluster -> GetPosition();

                auto gfPoint = gfTrack -> getPoint(idxStable);
                auto fitterInfo = (genfit::KalmanFitterInfo*) gfPoint -> getFitterInfo();
                auto fittedState = fitterInfo -> getFittedState(); //genfit::MeasuredStateOnPlane
                //auto posFittedM = fittedState.getPos();
                //auto posFitted = 10*TVector3(posFittedM[0], posFittedM[1], posFittedM[2]);
                auto posFitted = 10*fittedState.getPos();

                auto residual = pos - posFitted;

                if (!fNameCluster.IsNull()) {
                  fClusterIsLayerOrRow = cluster -> IsLayerCluster();
                  fClusterResidualX = residual.X();
                  fClusterResidualY = residual.Y();
                  fClusterResidualZ = residual.Z();
                  fClusterChi = cluster -> GetChi();
                  fClusterDip = cluster -> GetLambda();
                  fClusterNumHits = cluster -> GetNumHits();
                  fClusterX = pos.X();
                  fClusterZ = pos.Z();

                  double residuals[] = {fClusterResidualX, fClusterResidualY, fClusterResidualZ};

                  int idxN;
                  /**/ if (fClusterNumHits<=2) idxN = 0;
                  else if (fClusterNumHits==3) idxN = 1;
                  else   /*fClusterNumHits>=4*/idxN = 2;

                  Double_t sigma[3];
                  if (cluster -> IsLayerCluster()) {
                    sigma[0] = fHitClusterSigma[1][0][idxN] -> Eval(fClusterChi);
                    sigma[1] = fHitClusterSigma[1][1][idxN] -> Eval(fClusterDip);
                    sigma[2] = fHitClusterSigma[1][2][idxN] -> Eval(fClusterChi);
                  }
                  else {
                    sigma[0] = fHitClusterSigma[0][0][idxN] -> Eval(fClusterChi);
                    sigma[1] = fHitClusterSigma[0][1][idxN] -> Eval(fClusterDip);
                    sigma[2] = fHitClusterSigma[0][2][idxN] -> Eval(fClusterChi);
                  }

                  int isLR = fClusterIsLayerOrRow?1:0;
                  for (auto ixyz : {0,1,2}) {
                    fHistRawResiduals[isLR][ixyz][idxN] -> Fill(residuals[ixyz]);
                    fHistStdResiduals[isLR][ixyz][idxN] -> Fill(residuals[ixyz]/sigma[ixyz]);
                  }

                  fTreeCluster -> Fill();
                }

                idxStable++;
              }
              auto numClustersStable = idxStable;

              if (!fDebugVertexFileName.IsNull()) {
                fDebugVertexFile << iTrack << " " << numClustersStable << " " << poca.x() << " " << poca.y() << " " << poca.z() << " " << recoTrack -> GetVertexWeight() << endl;
                idxStable = 0;
                for (auto iCluster=0; iCluster<numClusters; ++iCluster)
                {
                  auto cluster = clusterArray -> at(iCluster);
                  if (!cluster -> IsStable())
                    continue;

                  auto gfPoint = gfTrack -> getPoint(idxStable);
                  auto fitterInfo = (genfit::KalmanFitterInfo*) gfPoint -> getFitterInfo();
                  auto fittedState = fitterInfo -> getFittedState(); //genfit::MeasuredStateOnPlane
                  //auto posFit = fittedState.getPos();
                  //auto posFitted = 10*TVector3(posFit[0], posFit[1], posFit[2]);
                  auto posFitted = 10*fittedState.getPos();

                  fDebugVertexFile << posFitted.x() << " " << posFitted.y() << " " << posFitted.z() << endl;
                  idxStable++;
                }
              }

              fCountFilledEvents++;
            }
          }
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

void STGenfitPIDTask::SetClusterSigmaFile(TString fileName) {
  fUseConstCov = false;
  fSigFileName = fileName;
}
void STGenfitPIDTask::SetUseConstCov(Double_t x, Double_t y, Double_t z) {
  fUseConstCov = true;
  fCovX = x;
  fCovY = y;
  fCovZ = z;
}
void STGenfitPIDTask::SetVertexMethod(TString method) { fGFRaveVertexMethod = method; }

void STGenfitPIDTask::SelectPID(Int_t pid) { fSelectPID = pid; }

void STGenfitPIDTask::SetTrackFileName(TString name) { fNameTrack = name; }
void STGenfitPIDTask::SetClusterFileName(TString name) { fNameCluster = name; }

void STGenfitPIDTask::FinishTask()
{
  if (!fNameCluster.IsNull()) {
    fFileCluster -> cd();
    fTreeCluster -> Write();
    (new TParameter<Int_t>("numFilledEvents",fCountFilledEvents)) -> Write();;
    for (auto isLR : {0,1})
      for (auto ixyz : {0,1,2})
        for (auto idxNH : {0,1,2})
          fHistRawResiduals[isLR][ixyz][idxNH] -> Write();
    for (auto isLR : {0,1})
      for (auto ixyz : {0,1,2})
        for (auto idxNH : {0,1,2})
          fHistStdResiduals[isLR][ixyz][idxNH] -> Write();
  }

  if (!fNameTrack.IsNull()) {
    fFileTrack -> cd();
    fTreeTrack -> Write();
  }
}
