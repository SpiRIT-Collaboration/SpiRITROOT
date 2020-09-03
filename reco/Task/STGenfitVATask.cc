#include "STGenfitVATask.hh"
#include "STDatabasePDG.hh"
#include "STTrack.hh"
#include "STBeamInfo.hh"

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
  Vertex_Shifter = 0;
  FileName_BDCCorrection_Theta_TargetPos = "";
  IsOption_BDCCorrection = 0;
}

STGenfitVATask::STGenfitVATask(Bool_t persistence)
: STRecoTask("GENFIT VA Task", 1, persistence)
{
  Vertex_Shifter = 0;
  FileName_BDCCorrection_Theta_TargetPos = "";
  IsOption_BDCCorrection = 0;
}

STGenfitVATask::~STGenfitVATask()
{
}

void STGenfitVATask::SetPreferMCBeam()                   { fUseMCBeam = kTRUE; }
void STGenfitVATask::SetClusteringType(Int_t type) { fClusteringType = type; }
void STGenfitVATask::SetConstantField() { fIsSamurai = kFALSE; }
void STGenfitVATask::SetFieldOffset(Double_t xOffset, Double_t yOffset, Double_t zOffset)
{
  fFieldXOffset = xOffset;
  fFieldYOffset = yOffset;
  fFieldZOffset = zOffset;

  LOG(INFO) << "== [STGenfitVATask] Field center : (" << xOffset << ", " << yOffset << ", " << zOffset << ") cm"<< FairLogger::endl;
}

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

  fMCEventHeader = (FairMCEventHeader *) fRootManager -> GetObject("MCEventHeader");
  if (fMCEventHeader == nullptr) {
    LOG(ERROR) << "Cannot find fMCEventHeader array!" << FairLogger::endl;
    return kERROR;
  }

  if(fUseMCBeam)
  //  if(1) // Isobe
  {
    fMCEventHeader = (FairMCEventHeader *) fRootManager -> GetObject("MCEventHeader.");
    if(!fMCEventHeader) 
    {
      LOG(INFO) << "Cannot find MCEventHeader despite SetPerferMCBeam enabled" << FairLogger::endl;
      LOG(INFO) << "Will try to obtain vertex via alternative means" << FairLogger::endl;
      fUseMCBeam = false;
    }else
    {
      // disable alternative vertex sources if MCBeamInfo exist
      LOG(DEBUG) << "Vertex information is being loaded from MCEventHeader" << FairLogger::endl;
      if(fSTMCEventHeader = dynamic_cast<STFairMCEventHeader*>(fMCEventHeader))
        LOG(INFO) << "MCEventHeader can be casted to STFairMCEventHeader. Additional beam information can be loaded" << FairLogger::endl;
      else
        LOG(INFO) << "MCEventHeader cannot be casted to STFairMCEventHeader. Only basic information can be loaded" << FairLogger::endl;
      fBeamFilename = "";
      fFixedVertexX = fFixedVertexY = fFixedVertexZ = -9999;
    }
    
  }

  fVATrackArray = new TClonesArray("STRecoTrack");
  fRootManager -> Register("VATracks", "SpiRIT", fVATrackArray, false);
  fPATrackArray = new TClonesArray("STRecoTrack");
  fEmbedVATrackArray = new TClonesArray("STRecoTrack");
  fRootManager -> Register("EmbedVATracks", "SpiRIT", fEmbedVATrackArray, fIsPersistence);

  fCandListArray = new TClonesArray("STRecoTrackCandList");
  fRootManager -> Register("VACandList", "SpiRIT", fCandListArray, fIsListPersistence);

  fVAVertexArray = new TClonesArray("STVertex");
  fRootManager -> Register("VAVertex", "SpiRIT", fVAVertexArray, fIsPersistence);

  fGenfitTest = new STGenfitTest2(fIsSamurai, fFieldXOffset, fFieldYOffset, fFieldZOffset);
  fPIDTest = new STPIDTest();

  fBeamInfo = new STBeamInfo();
  fRootManager -> Register("STBeamInfo", "SpiRIT", fBeamInfo, fIsPersistence);

  if (fUseRave)
    fVertexFactory = ((STGenfitPIDTask *) FairRunAna::Instance() -> GetTask("GENFIT Task")) -> GetVertexFactoryInstance();

  if (!fBeamFilename.IsNull() || (fFixedVertexX != -9999 && fFixedVertexY != -9999 && fFixedVertexZ != -9999) || fUseMCBeam) {
    fBDCVertexArray = new TClonesArray("STVertex");
    fRootManager -> Register("BDCVertex", "SpiRIT", fBDCVertexArray, kTRUE);
  }

  if (!fBeamFilename.IsNull())
  {
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

  if (fRecoHeader != nullptr) {
    fRecoHeader -> SetPar("genfitva_loadSamuraiMap", fIsSamurai);
    fRecoHeader -> Write("RecoHeader", TObject::kWriteDelete);
  }
  
  if(IsOption_BDCCorrection==1)
  {
    Vertex_Shifter = new ST_VertexShift();
    Vertex_Shifter -> Load_BDC_Correction(FileName_BDCCorrection_Theta_TargetPos);
  }
  
  return kSUCCESS;
}

void STGenfitVATask::Exec(Option_t *opt)
{
  fCandListArray -> Clear("C");
  fVATrackArray -> Clear("C");
  fPATrackArray -> Clear("C");
  fEmbedVATrackArray -> Clear("C");
  fVAVertexArray -> Clear("C");
  fBeamInfo -> Clear();
  if (!fBeamFilename.IsNull() || (fFixedVertexX != -9999 && fFixedVertexY != -9999 && fFixedVertexZ != -9999))
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

  auto vertex = (STVertex *) fVertexArray -> At(chosenVID);
  vertex -> SetIsCollisionVertex();
  if (fPeakZ != -9999 && (vertex -> GetPos().Z() > fPeakZ - fSigmaMultiple*fSigma && vertex -> GetPos().Z() < fPeakZ + fSigmaMultiple*fSigma))
    vertex -> SetIsTargetVertex();
  else if (TMath::Abs(vertex -> GetPos().Z() + 10) < 20) // -30 < vz < 10 
    vertex -> SetIsTargetVertex();

  if (!vertex -> IsTargetVertex()) {
    LOG(INFO) << Space() << "STGenfitVATask vertex z position is out of (-30, 10) range! Not adding vertex in!" << FairLogger::endl;

    return; // if the event is not vertex event, not add vertex in
  }

  auto vertexPos = vertex -> GetPos(); //this position is TPC Vertex
  //  std::cout << "vpos0: " << vertexPos.X() << std::endl;

  Bool_t goodBDC = kTRUE;
  if (!fBeamFilename.IsNull()) {
    fBeamTree -> GetEntry(fEventHeader -> GetEventID() - 1);
    fBDCTree -> GetEntry(fEventHeader -> GetEventID() - 1);
    fBeamEnergy -> reset(fZ, fAoQ, fBeta37);

    fBeamInfo -> fBeamAoQ = fAoQ;
    fBeamInfo -> fBeamZ = fZ;

    Double_t E1 = fBeamEnergy -> getCorrectedEnergy();

    if (fZ > 0 && fZ < 75 && fAoQ > 1. && fAoQ < 3 && fBDC1x > -999 && fBDC1y > -999 && fBDC2x > -999 && fBDC2y > -999) {
//      Double_t ProjectedAtZ = -580.4 + vertex -> GetPos().Z();  // mid target = -592.644, start pad plane =-580.4, end of pad plane = 763.6
      Double_t ProjectedAtZ = -580.4 + (fPeakZ != -9999 ? fPeakZ : vertex -> GetPos().Z());  // mid target = -592.644, start pad plane =-580.4, end of pad plane = 763.6
//      double ProjectedAtZ=-592.644;//////mid target = -592.644, start pad plane =-580.4, end of pad plane = 763.6
      fBDCProjection -> ProjectParticle(fBDC2x, fBDC2y, -2160., fBDCax, fBDCby, fZ, E1, ProjectedAtZ, fBeamEnergy -> getMass());//-580.4,-583.904

      vertex -> SetIsGoodBDC();
    } else
      goodBDC = kFALSE;

    if (!goodBDC)
      LOG(INFO) << Space() << "STGenfitVATask " << "Bad BDC!" << FairLogger::endl;
    else {
      fBeamInfo -> fXTargetPlane = fBDCProjection -> getX();
      fBeamInfo -> fYTargetPlane = fBDCProjection -> getY();
      fBeamInfo -> fRotationAngleATargetPlane = fBDCProjection -> getA();
      fBeamInfo -> fRotationAngleBTargetPlane = fBDCProjection -> getB();
      fBeamInfo -> fBeamEnergyTargetPlane = fBDCProjection -> getMeVu();
      fBeamInfo -> fBeamVelocityTargetPlane = fBDCProjection -> getBeta();

      vertexPos = TVector3(fBDCProjection -> getX() + fOffsetX, fBDCProjection -> getY() + fOffsetY, (fPeakZ != -9999 ? fPeakZ : vertex -> GetPos().Z()) + fOffsetZ);

      auto bdcVertex = (STVertex *) fBDCVertexArray -> ConstructedAt(0);
      bdcVertex -> SetIsGoodBDC();
      bdcVertex -> SetPos(vertexPos);

      if (vertex -> IsTargetVertex()) {
        LOG(INFO) << Space() << "STGenfitVATask " << "Target event probable with good BDC!" << FairLogger::endl;
        bdcVertex -> SetIsTargetVertex();
      } else {
        LOG(INFO) << Space() << "STGenfitVATask " << "Off-target event probable with good BDC!" << FairLogger::endl;
        bdcVertex -> SetIsCollisionVertex();
      }
    }
  } else if (fFixedVertexX != -9999 && fFixedVertexY != -9999 && fFixedVertexZ != -9999) {
    vertexPos = TVector3(fFixedVertexX, fFixedVertexY, fFixedVertexZ);
    fBeamInfo -> fXTargetPlane = fFixedVertexX;
    fBeamInfo -> fYTargetPlane = fFixedVertexY;

    auto bdcVertex = (STVertex *) fBDCVertexArray -> ConstructedAt(0);
    bdcVertex -> SetIsGoodBDC();
    bdcVertex -> SetPos(vertexPos);

    if (vertex -> IsTargetVertex()) {
      LOG(INFO) << Space() << "STGenfitVATask " << "Target event probable with Fixed BDC!" << FairLogger::endl;
      bdcVertex -> SetIsTargetVertex();
    } else {
      LOG(INFO) << Space() << "STGenfitVATask " << "Off-target event probable with Fixed BDC!" << FairLogger::endl;
      bdcVertex -> SetIsCollisionVertex();
    }

  } else if(fUseMCBeam)
  {
    fBeamInfo -> fXTargetPlane = fMCEventHeader -> GetX()*10; // cm to mm
    fBeamInfo -> fYTargetPlane = fMCEventHeader -> GetY()*10;
    fBeamInfo -> fRotationAngleATargetPlane = fMCEventHeader -> GetRotX()*1000; // rad to mrad
    fBeamInfo -> fRotationAngleBTargetPlane = fMCEventHeader -> GetRotY()*1000;
    if(fSTMCEventHeader)
    {
      fBeamInfo -> fBeamEnergyTargetPlane = fSTMCEventHeader -> GetEnergyPerNucleons()*1000; // GeV to MeV
      fBeamInfo -> fBeamZ = fSTMCEventHeader -> GetBeamZ();
      fBeamInfo -> fBeamAoQ = fSTMCEventHeader -> GetBeamAoZ();
    }
    vertexPos = TVector3(fMCEventHeader -> GetX(), fMCEventHeader -> GetY(), fMCEventHeader -> GetZ());
    vertexPos *= 10.; // cm to mm

    auto bdcVertex = (STVertex *) fBDCVertexArray -> ConstructedAt(0);
    bdcVertex -> SetIsGoodBDC();
    bdcVertex -> SetPos(vertexPos);
    LOG(INFO) << Space() << "STGenfitVATask " << "BDC vertex is loaded from MCEventHeader " << vertexPos.X() << " " << vertexPos.Y() << " " << vertexPos.Z() << FairLogger::endl;
  }

    //    std::cout << "vpos: " << vertexPos.X() << " " << VertexPosShift.X() << std::endl;
  LOG(INFO) << Space() << "STGenfitVATask vtxpos " << vertexPos.X()  << " " << vertexPos.Y() << " " << vertexPos.Z() << FairLogger::endl;
  LOG(INFO) << Space() << "STGenfitVATask pripos " << fMCEventHeader -> GetX()  << " " << fMCEventHeader -> GetY() << " " << fMCEventHeader -> GetZ() << FairLogger::endl;
  TVector3 pripos = TVector3(fMCEventHeader -> GetX(), fMCEventHeader -> GetY(), fMCEventHeader -> GetZ());

  auto numTracks = fRecoTrackArray -> GetEntriesFast();
  for (auto iTrack = 0; iTrack < numTracks; iTrack++) {
    auto track = (STRecoTrack *) fRecoTrackArray -> At(iTrack);

    if (track -> GetVertexID() != chosenVID)
      continue;
    
    double BDCShiftX = 0; double BDCShiftY = 0; //unit: mm
    
    //the below if for getting the shift value according to the RecoMomentum.
    if(IsOption_BDCCorrection==1)
    {
      TVector3 RecoMom = track->GetMomentumTargetPlane();
      double Phi = RecoMom.Phi()*TMath::RadToDeg();
      double Theta = RecoMom.Theta()*TMath::RadToDeg();
      if(Phi<0) { Phi = Phi+360; }
      Vertex_Shifter->GetShiftValue_Smooth(Theta,Phi,&BDCShiftX,&BDCShiftY);
    }
    
    TVector3 VertexPosShift(BDCShiftX,BDCShiftY,0);
//    cout<<" Phi: "<<Phi<<" Theta: "<<Theta<<" ShiftX: "<<ShiftX<<" ShiftY: "<<ShiftY<<endl;
    
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
    vertexCluster -> SetClusterID(-2);
    vertexCluster -> SetHit(-4, vertexPos+VertexPosShift, 30); // here add this shift to the BDC original position 
    vertexCluster -> SetCovMatrix(cov);
    vertexCluster -> SetDFromCovForGenfit(1,1,1,true);

    // isobe priCluster: MCvertex instead of reco. vertex
    auto priCluster = new STHitCluster();
    priCluster -> SetIsStable(kTRUE);
    priCluster -> SetClusterID(-3);
    priCluster -> SetHit(-4, pripos, 30);
    //priCluster -> SetHit(-4, vertexPos+VertexPosShift, 30); // here add this shift to the BDC original position 
    priCluster -> SetCovMatrix(cov);
    priCluster -> SetDFromCovForGenfit(1,1,1,true);


    Int_t trackID = fCandListArray -> GetEntriesFast();

    auto candList = (STRecoTrackCandList *) fCandListArray -> ConstructedAt(trackID);
    auto vaTrack = (STRecoTrack *) fVATrackArray -> ConstructedAt(trackID);
    vaTrack -> SetParentID(iTrack);
    auto paTrack = (STRecoTrack *) fPATrackArray -> ConstructedAt(trackID);
    paTrack -> SetParentID(iTrack);

    auto helixTrack = (STHelixTrack *) fHelixTrackArray -> At(track -> GetHelixID());
    vaTrack -> SetHelixTrack(helixTrack);
    paTrack -> SetHelixTrack(helixTrack);

    auto clusterArray = helixTrack -> GetClusterArray();
    clusterArray -> insert(clusterArray -> begin(), priCluster);
    clusterArray -> insert(clusterArray -> begin(), vertexCluster);
    for (auto cluster : *clusterArray) {
      if (cluster -> IsStable()) {
        candList -> AddHitID(cluster -> GetClusterID());
        vaTrack -> AddClusterID(cluster -> GetClusterID());
        paTrack -> AddClusterID(cluster -> GetClusterID());
      }
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

      auto vaTrackCand = candList -> GetRecoTrackCand(iPID);

      //genfit::Track *gfTrack = fGenfitTest -> FitTrackWithVertex(helixTrack, vertexCluster, pdg);
      genfit::Track *gfTrack = fGenfitTest -> FitTrack(helixTrack, pdg);
      genfit::Track *pgfTrack = fGenfitTest -> FitTrack(helixTrack, pdg, true);
      if (gfTrack == nullptr) {
        vaTrackCand -> SetPIDProbability(0);
        continue;
      } else {
        vaTrackCand -> SetGenfitTrack(gfTrack);
      }

      TVector3 mom, momTargetPlane, posTargetPlane;
      fGenfitTest -> GetTrackParameters(gfTrack, mom, momTargetPlane, posTargetPlane);
      TVector3 pmom, pmomTargetPlane, pposTargetPlane;
      fGenfitTest -> GetTrackParameters(pgfTrack, pmom, pmomTargetPlane, pposTargetPlane);

      if (mom.Z() < 0)
        mom = -mom;
      if (pmom.Z() < 0)
        pmom = -pmom;
      if (momTargetPlane.Z() < 0)
        momTargetPlane = -momTargetPlane;
      vaTrackCand -> SetMomentum(mom);
      vaTrackCand -> SetMomentumTargetPlane(momTargetPlane);
      vaTrackCand -> SetPosTargetPlane(posTargetPlane);

      vaTrackCand -> SetPriMomentum(pmom);
      vaTrackCand -> SetPriPosTargetPlane(pposTargetPlane);
      vaTrackCand -> SetPosVtxCluster(vertexCluster->GetPosition());


      fGenfitTest -> GetdEdxPointsByLayerRow(gfTrack, helixTrack, vaTrackCand -> GetdEdxPointArray(), true);

      auto prob = 1; // fPIDTest -> GetProbability(pid, mom.Mag(), vaTrackCand -> GetdEdxWithCut(0,0.7));
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
    auto bestGenfitTrack = bestRecoTrackCand -> GetGenfitTrack();
    gfTrackArrayToVertex.push_back(bestGenfitTrack);
    vaTrackArrayToVertex.push_back(vaTrack);

//    helixTrack -> SetGenfitID(iTrack);
//    helixTrack -> SetIsGenfitTrack();
//    helixTrack -> SetGenfitMomentum(bestRecoTrackCand -> GetMomentum().Mag());

    TVector3 kyotoL, kyotoR, katana, neuland;
    fGenfitTest -> GetPosOnPlanes(bestRecoTrackCand -> GetGenfitTrack(), kyotoL, kyotoR, katana, neuland);
    bestRecoTrackCand -> Copy(vaTrack);
    vaTrack -> SetHelixID(helixTrack -> GetTrackID());
    vaTrack -> SetRecoID(helixTrack -> GetGenfitID());
    vaTrack -> SetPosKyotoL(kyotoL);
    vaTrack -> SetPosKyotoR(kyotoR);
    vaTrack -> SetPosKatana(katana);
    vaTrack -> SetPosNeuland(neuland);

    auto fitStatus = bestGenfitTrack -> getFitStatus(bestGenfitTrack -> getTrackRep(0));
    vaTrack -> SetChi2(fitStatus -> getChi2());
    vaTrack -> SetNDF(fitStatus -> getNdf());

    try {
      auto fitState = bestGenfitTrack -> getFittedState();
      TVector3 gfmomReco;
      TVector3 gfposReco(-999,-999,-999);
      TMatrixDSym covMat(6,6);
      fitState.getPosMomCov(gfposReco, gfmomReco, covMat);

      vaTrack -> SetGenfitCharge(gfmomReco.Z() < 0 ? -1 * fitStatus -> getCharge() : fitStatus -> getCharge());
    } catch (...) {
      vaTrack -> SetGenfitCharge(-2);
    }

    try {
      vaTrack -> SetTrackLength(bestGenfitTrack -> getTrackLen());
    } catch (...) {
      vaTrack -> SetTrackLength(-9999);
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

      if (!fGenfitTest -> ExtrapolateTo(bestGenfitTrack, pos, point))
        continue;

      genfitChi2R += (point - pos).Mag2();
      genfitChi2X += (point.X() - pos.X())*(point.X() - pos.X());
      genfitChi2Y += (point.Y() - pos.Y())*(point.Y() - pos.Y());
      genfitChi2Z += (point.Z() - pos.Z())*(point.Z() - pos.Z());
    }
    vaTrack -> SetNumEmbedClusters(numEmbedClusters);
    vaTrack -> SetNumRowClusters(numRowClusters);
    vaTrack -> SetNumLayerClusters(numLayerClusters);
    vaTrack -> SetNumRowClusters90(numRowClusters90);
    vaTrack -> SetNumLayerClusters90(numLayerClusters90);
    vaTrack -> SetRowVec(fByRowClusters);
    vaTrack -> SetLayerVec(fByLayerClusters);
    vaTrack -> SetHelixChi2R(helixChi2R);
    vaTrack -> SetHelixChi2X(helixChi2X);
    vaTrack -> SetHelixChi2Y(helixChi2Y);
    vaTrack -> SetHelixChi2Z(helixChi2Z);
    vaTrack -> SetChi2R(genfitChi2R);
    vaTrack -> SetChi2X(genfitChi2X);
    vaTrack -> SetChi2Y(genfitChi2Y);
    vaTrack -> SetChi2Z(genfitChi2Z);

    if (!fUseRave) {
      genfit::Track *gfTrack = bestRecoTrackCand -> GetGenfitTrack();
      TVector3 momVertex;
      TVector3 pocaVertex;
      fGenfitTest -> GetMomentumWithVertex(gfTrack, vertex -> GetPos(), momVertex, pocaVertex);
      if (momVertex.Z() < 0)
        momVertex = -momVertex;
      vaTrack -> SetVertexID(chosenVID);
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
  }

  LOG(INFO) << Space() << "VATrack " << fVATrackArray -> GetEntriesFast() << FairLogger::endl;

  if (fUseRave) {
    if (gfTrackArrayToVertex.size() < 2) {
      for (auto vaTrack : vaTrackArrayToVertex)
        vaTrack -> SetCharge(1);
      return;
    }

    vector<genfit::GFRaveVertex *> vertices;
    try {
      fVertexFactory -> findVertices(&vertices, gfTrackArrayToVertex);
    } catch (...) {
    }

    Int_t vaNumVertices = vertices.size();
    LOG(INFO) << Space() << "vector verticies " << vertices.size() << FairLogger::endl;

    for (UInt_t iVert = 0; iVert < vaNumVertices; iVert++) {
      genfit::GFRaveVertex* vaVertex = static_cast<genfit::GFRaveVertex*>(vertices[iVert]);

      auto vaNumTracks = vaVertex -> getNTracks();
      for (Int_t iTrack = 0; iTrack < vaNumTracks; iTrack++) {
        genfit::GFRaveTrackParameters *par = vaVertex -> getParameters(iTrack);
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
            fGenfitTest -> GetMomentumWithVertex(gfTrack, 10*vaVertex->getPos(), momVertex, pocaVertex);
            if (momVertex.Z() < 0)
              momVertex = -momVertex;
            vaTrack -> SetMomentum(momVertex);
            vaTrack -> SetPOCAVertex(pocaVertex);

            Double_t effCurvature1;
            Double_t effCurvature2;
            Double_t effCurvature3;
            Double_t charge = fGenfitTest -> DetermineCharge(vaTrack, vaVertex -> getPos(), effCurvature1, effCurvature2, effCurvature3);
            vaTrack -> SetCharge(charge);
            vaTrack -> SetEffCurvature1(effCurvature1);
            vaTrack -> SetEffCurvature2(effCurvature2);
            vaTrack -> SetEffCurvature3(effCurvature3);
          }
        }
      }

      new ((*fVAVertexArray)[iVert]) STVertex(*vaVertex);
      delete vaVertex;
    }

    // Isobe

    Int_t numTrk = fVATrackArray -> GetEntriesFast();
    Int_t netrk = 0;
    for (Int_t i = 0; i < numTrk; i++){
      STRecoTrack *trk = (STRecoTrack *) fVATrackArray -> At(i);
      if(trk->GetNumEmbedClusters()>0){
	auto newtrk = (STRecoTrack *) fEmbedVATrackArray -> ConstructedAt(netrk);
	//      std::cout << __FILE__ << "\t" << __LINE__ << std::endl;
        // from STRecoTrackCand
        newtrk -> SetPID(trk->GetPID());
        newtrk -> SetPIDProbability(trk->GetPIDProbability());
        newtrk -> SetMomentum(trk->GetMomentum());
        newtrk -> SetMomentumTargetPlane(trk->GetMomentumTargetPlane());
        newtrk -> SetPriMomentum(trk->GetPriMomentum());
        newtrk -> SetPosTargetPlane(trk->GetPosTargetPlane());
        newtrk -> SetPriPosTargetPlane(trk->GetPriPosTargetPlane());
        newtrk -> SetPosVtxCluster(trk->GetPosVtxCluster());
        //      std::cout << "VAPos2: " << trk->GetPosVtxCluster().X() << "\t"      << trk->GetPosVtxCluster().Y() << "\t"      << trk->GetPosVtxCluster().Z() << std::endl;
        newtrk -> SetIsEmbed(trk->IsEmbed());
        newtrk -> SetNumEmbedClusters(trk->GetNumEmbedClusters());

        //      std::cout << __FILE__ << "\t" << __LINE__ << std::endl;

	auto inarray = trk -> GetdEdxPointArray();
        auto newarray = newtrk -> GetdEdxPointArray();
        for (auto dedx : *inarray) {
          newtrk -> AdddEdxPoint(dedx);
        }

        //      std::cout << __FILE__ << "\t" << __LINE__ << std::endl;

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

        //      std::cout << __FILE__ << "\t" << __LINE__ << std::endl;

        auto inclid = trk -> GetClusterIDArray();
        for (auto ii : *inclid) {
          newtrk -> AddClusterID(ii);
        }

        //      std::cout << __FILE__ << "\t" << __LINE__ << std::endl;

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

        //      std::cout << __FILE__ << "\t" << __LINE__ << std::endl;

        netrk ++;
      }
    }

    fEventHeader -> SetNVATrk(fVATrackArray->GetEntries());

    // end: Isobe

    LOG(INFO) << Space() << "VAVertex " << fVAVertexArray -> GetEntriesFast() << FairLogger::endl;
  }
}

void STGenfitVATask::SetBeamFile(TString fileName) { fBeamFilename = fileName; }
void STGenfitVATask::SetInformationForBDC(Int_t runNo, Double_t offsetX, Double_t offsetY, Double_t offsetZ)
{
  fRunNo = runNo;
  fOffsetX = offsetX;
  fOffsetY = offsetY;
  fOffsetZ = offsetZ;

  cout << "== [STGenfitVATask] BDC offset for run#" << runNo << " is set to (" << offsetX << ", " << offsetY << ", " << offsetZ << ") mm" << endl;
}

void STGenfitVATask::SetZtoProject(Double_t peakZ, Double_t sigma, Double_t sigmaMultiple)
{
  fPeakZ = peakZ;
  fSigma = sigma;
  fSigmaMultiple = sigmaMultiple;

  cout << "== [STGenfitVATask] BDC will be projected to z = " << peakZ << " mm, if TPC vertex is within " << fSigmaMultiple << " sigma range with sigma = " << sigma << " mm" << endl;
}

void STGenfitVATask::SetFixedVertex(Double_t x, Double_t y, Double_t z)
{
  fFixedVertexX = x;
  fFixedVertexY = y;
  fFixedVertexZ = z;

  cout << "== [STGenfitVATask] BDC point added to a fixed point (" << x << ", " << y << ", " << z << ") mm" << endl;
}

void STGenfitVATask::SetUseRave(Bool_t val)
{
  fUseRave = val;
}
