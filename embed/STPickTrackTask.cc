//---------------------------------------------------------------------
// Description:
//      Pick Track task class.
//
// Author List:
//      Rensheng     Soochow Univ.
//
//----------------------------------------------------------------------

#include "STPickTrackTask.hh"

ClassImp(STPickTrackTask)

STPickTrackTask::STPickTrackTask()
{
  EmbedContainer = new STEmbedTrackContainer();
  RawEvent_Dummy = new STRawEvent();
  RawEvent_Dummy->Clear();
  
  PIDProbability = 0.5;
  TrackNum_Min = 1;
  TrackNum_Max = 80;
  
  RunNo = 0;
  EvtTag = "event";
  VertexTag = "OnTarget";
  aPulse = new STPulse();
  StandardPulseFunction = 0;
  MinClusterNum = 0;
  IsStoreEachEvt = 0;
  IsRawData_Recorded = 0;
  IsPickTrackCutUsed=0;
  aTrackCut = new STPickTrackCut();
  
  TrackID = -1;
}

STPickTrackTask::~STPickTrackTask()
{}

InitStatus STPickTrackTask::Init()
{
  fRootManager = FairRootManager::Instance();
  if(fRootManager==nullptr)
  {
    LOG(ERROR)<<"Cannot find RootManager!"<<FairLogger::endl;
    return kERROR;
  }
  fRawEventArray = (TClonesArray *) fRootManager -> GetObject("STRawEvent");
  if (fRawEventArray == nullptr)
  {
    LOG(ERROR) << "Cannot find STRawEvent array!" << FairLogger::endl;
    return kERROR;
  }
  fRecoTrackArray = (TClonesArray *) fRootManager -> GetObject("STRecoTrack");
  if (fRecoTrackArray == nullptr)
  {
    LOG(ERROR) << "Cannot find STRecoTrack array!" << FairLogger::endl;
    return kERROR;
  }
  fHitClusterArray = (TClonesArray *) fRootManager -> GetObject("STHitCluster");
  if (fHitClusterArray == nullptr)
  {
    LOG(ERROR) << "Cannot find STHitCluster array!" << FairLogger::endl;
    return kERROR;
  }
  fHitArray = (TClonesArray *) fRootManager -> GetObject("STHit");
  if (fHitArray == nullptr)
  {
    LOG(ERROR) << "Cannot find STHit array!" << FairLogger::endl;
    return kERROR;
  }
  fVertexArray = (TClonesArray *) fRootManager -> GetObject("STVertex");
  if (fVertexArray == nullptr)
  {
    LOG(ERROR) << "Cannot find STVertex array!" << FairLogger::endl;
    return kERROR;
  }

}

void STPickTrackTask::SetParContainers()
{
  
}

void STPickTrackTask::Exec(Option_t *opt)
{
//here I only collect the single cocktail track;
//latter, we will also collect the other tracks from the event, so just change the judgement of this function
  int RecoTrackNum = fRecoTrackArray->GetEntries();
  // the below is for recording the pion
  if(IsPickTrackCutUsed==1)
  {
    int VertexNum = fVertexArray->GetEntries();
    if(VertexNum!=1) { return; }
    STVertex* aVertex = (STVertex*) fVertexArray -> At(0);
    if(aTrackCut->IsVertexInTarget(aVertex)==0) { return; }
    
    EmbedContainer->SetRunNo(RunNo);
    STRawEvent *aRawEvent = (STRawEvent *) fRawEventArray -> At(0);
    
    for(int iTrack = 0;iTrack<RecoTrackNum;iTrack++)
    {
      STRecoTrack* aRecoTrack = (STRecoTrack*) fRecoTrackArray -> At(iTrack);
      TrackID = iTrack;
      vector<Int_t>* fHitClusterIDArray = aRecoTrack->GetClusterIDArray();
      int ClusterNum = (*fHitClusterIDArray).size();
      if(ClusterNum<=MinClusterNum) { continue; }
      if(aTrackCut->IsTrackInVertex(aRecoTrack,aVertex)==0) { continue; }
      if(aTrackCut->IsInPionMinusCut(aRecoTrack) || aTrackCut->IsInPionPlusCut(aRecoTrack)) // here I only record the pion.
      {
        if(IsRawData_Recorded==1) { PickHits_onPad(aRawEvent,aRecoTrack); }
        else { PickHits_onStandardPulse(aRawEvent,aRecoTrack); }
        EmbedContainer->AddTrack(RawEvent_Dummy,aRecoTrack,aVertex);
      }
    }
    return;
  }
  
  // the below is for recording every event
  if(IsStoreEachEvt==1)
  {
    EmbedContainer->SetRunNo(RunNo);
    STRawEvent *aRawEvent = (STRawEvent *) fRawEventArray -> At(0);
    STRecoTrack *aRecoTrack = 0;
    STVertex *aVertex = 0;
    if(RecoTrackNum==1)
    {
      aRecoTrack = (STRecoTrack *) fRecoTrackArray -> At(0);
      TrackID = 0;
      //this method will pickup the corresponding hits, and store these in the RawEvent_Dummy.
      if(IsRawData_Recorded==1) { PickHits_onPad(aRawEvent,aRecoTrack); }
      else { PickHits_onStandardPulse(aRawEvent,aRecoTrack); }
      EmbedContainer->AddTrack(RawEvent_Dummy,aRecoTrack,aVertex);
    }
    else
    {
      EmbedContainer->AddTrack(aRawEvent,aRecoTrack,aVertex);
    }
    cout<<"--->You are recording every track and the related raw event data."<<endl;
    return;
  }
  
  if( EvtTag=="cocktail")
  {
    if(RecoTrackNum!=1) { return; }
    STRawEvent *aRawEvent = (STRawEvent *) fRawEventArray -> At(0);
    STRecoTrack *aRecoTrack = (STRecoTrack *) fRecoTrackArray -> At(0);
    TrackID = 0;
    STVertex* aVertex = 0;
    if((*(aRecoTrack->GetClusterIDArray())).size()<MinClusterNum) { return; }
    if(aRecoTrack->GetPIDProbability()<PIDProbability) { return; }
    if(EmbedContainer->Is_NeedStore(aRecoTrack) )
    {
      EmbedContainer->SetRunNo(RunNo);
      //this method will pickup the corresponding hits, and store these in the RawEvent_Dummy.
      if(IsRawData_Recorded==1) { PickHits_onPad(aRawEvent,aRecoTrack); }
      else { PickHits_onStandardPulse(aRawEvent,aRecoTrack); }
      EmbedContainer->AddTrack(RawEvent_Dummy,aRecoTrack,aVertex);
    }
    return;
  }
  //Because the distinguish of PID is not fix in the SpiRITROOT, so the below is not working.
  if(EvtTag=="event")
  {
    //choose the track from one event, which has certain multiplicity;
    if(RecoTrackNum<TrackNum_Min || RecoTrackNum>TrackNum_Max){ return; }
    if(VertexTag=="Total")
    {
      for(int iTrack=0;iTrack<RecoTrackNum;iTrack++)
      {
        STRawEvent *aRawEvent = (STRawEvent *) fRawEventArray -> At(0);
        STRecoTrack *aRecoTrack = (STRecoTrack *) fRecoTrackArray -> At(iTrack);
        TrackID = iTrack;
        // for the number of cluster
        if((*(aRecoTrack->GetClusterIDArray())).size()<MinClusterNum) { continue; }
        // for the pid probability
        if(aRecoTrack->GetPIDProbability()<PIDProbability) { continue; }
        for(int j=0;j<ParticleNum;j++)
        {
          if(aRecoTrack->GetPID()!=ParticleID[j]) { continue; }
          if(EmbedContainer->Is_NeedStore(aRecoTrack))//aRecoTrack->GetMomentumTargetPlane().Mag()==0)
          {
            EmbedContainer->SetRunNo(RunNo);
            //this method will pickup the corresponding hits, and store these in the RawEvent_Dummy.
            if(IsRawData_Recorded==1) { PickHits_onPad(aRawEvent,aRecoTrack); }
            else { PickHits_onStandardPulse(aRawEvent,aRecoTrack); }
            
            STVertex* aVertex = 0;
            int VertexID_tem = aRecoTrack->GetVertexID();
            if(VertexID_tem!=-1) { aVertex = (STVertex*) fVertexArray->At(VertexID_tem); } 
            EmbedContainer->AddTrack(RawEvent_Dummy,aRecoTrack,aVertex);
          }
        }
      }
    }
    
    if(VertexTag=="OnTarget")
    {
      int VertexNum = fVertexArray->GetEntries();
      cout<<"--------> Vertex Num: "<<VertexNum<<"<----------"<<endl;
      if(VertexNum<=0) { return; }
      for(int iTrack=0;iTrack<RecoTrackNum;iTrack++)
      {
        STRawEvent *aRawEvent = (STRawEvent *) fRawEventArray -> At(0);
        STRecoTrack *aRecoTrack = (STRecoTrack *) fRecoTrackArray -> At(iTrack);
        TrackID = iTrack;
        // for the number of cluster
        if((*(aRecoTrack->GetClusterIDArray())).size()<MinClusterNum) { continue; }
        // for the pid probability
        if(aRecoTrack->GetPIDProbability()<PIDProbability) { continue; }
        // for the vertex.
        int VertexID_tem = aRecoTrack->GetVertexID();
        if(VertexID_tem==-1) { continue; }
        STVertex* aVertex = (STVertex*) fVertexArray->At(VertexID_tem);
        if(aTrackCut->IsVertexInTarget(aVertex)==0) { continue; }
        if(aTrackCut->IsTrackInVertex(aRecoTrack,aVertex)==0) { continue; }
        for(int j=0;j<ParticleNum;j++)
        {
          if(aRecoTrack->GetPID()!=ParticleID[j]) { continue; }
          if(EmbedContainer->Is_NeedStore(aRecoTrack))
          {
            EmbedContainer->SetRunNo(RunNo);
            //this method will pickup the corresponding hits, and store these in the RawEvent_Dummy.
            if(IsRawData_Recorded==1) { PickHits_onPad(aRawEvent,aRecoTrack); }
            else { PickHits_onStandardPulse(aRawEvent,aRecoTrack); }
            
            EmbedContainer->AddTrack(RawEvent_Dummy,aRecoTrack,aVertex);
          }
        }//particle series
      }//track
    }
  }
  
}

void STPickTrackTask::PickHits_onPad(STRawEvent* aRawEvent,STRecoTrack* aTrack)
{
// This function will record all the signal(pulse) on the pad, also the noise.
  RawEvent_Dummy->Clear();
//  fHitArray_Track->Clear();
  vector<Int_t>* fHitClusterIDArray = aTrack->GetClusterIDArray();
  int ClusterNum = (*fHitClusterIDArray).size();
//  cout<<"ClusterNum: "<<ClusterNum<<endl;
//  int HitNumTotal = 0;
  for(int iCluster = 0;iCluster<ClusterNum;iCluster++)
  {
    STHitCluster* aCluster = (STHitCluster*) fHitClusterArray->At((*fHitClusterIDArray)[iCluster]);
    vector<Int_t> *HitIDArray = aCluster->GetHitIDs();
    int HitNum = (*HitIDArray).size();
//    int HitNum = aCluster->GetHitPtrs()->size();
//    cout<<"HitNum: "<<HitNum<<endl;
//    HitNumTotal += HitNum;
    
    for(int iHit = 0;iHit<HitNum;iHit++)
    {
//      STHit* aHit = (*aCluster->GetHitPtrs())[iHit];
      int HitID = (*HitIDArray)[iHit];
      STHit* aHit = (STHit*) fHitArray->At(HitID);
      int RowNum = aHit->GetRow();
      int LayerNum = aHit->GetLayer();
      double fTb = aHit->GetTb();
      double fCharge = aHit->GetCharge();
//      cout<<RowNum<<"  "<<LayerNum<<"  "<<fTb<<"  "<<fCharge<<endl;
      if(TrackID == aHit->GetTrackID() || aHit->IsClustered()!=0) 
      { RawEvent_Dummy->SetPad(aRawEvent->GetPad(RowNum, LayerNum)); }
/*
      else { cout<<"--->"<<TrackID <<"  "<< aHit->GetTrackID()<<endl; }
*/
    }// for the hit in one cluster
  }// for the cluster in one track
  RawEvent_Dummy->SetIsGood(aRawEvent->IsGood());
  RawEvent_Dummy->SetEventID(aRawEvent->GetEventID());
//  int PadNum = RawEvent_Dummy->GetNumPads();
//  cout<<"HitNum Total: "<<HitNumTotal<<" PadNum: "<<PadNum<<endl;
}

void STPickTrackTask::PickHits_onStandardPulse(STRawEvent* aRawEvent,STRecoTrack* aTrack)
{
// This function will only record the standard pulse on the pad. So it suppose that we have get rid of all the non-related pulses.
  RawEvent_Dummy->Clear();
//the below is for get the track with all the hits that are related to the track.
  int HitNum = fHitArray->GetEntries();
  for(int iHit=0;iHit<HitNum;iHit++)
  {
    STHit* aHit = (STHit*) fHitArray->At(iHit);
    if(TrackID == aHit->GetTrackID())
    {
      int RowNum = aHit->GetRow();
      int LayerNum = aHit->GetLayer();
      double fTb = aHit->GetTb();
      double fCharge = aHit->GetCharge();
      StandardPulseFunction = aPulse->GetPulseFunction(aHit);
      StandardPulseFunction->SetParameters(fCharge,fTb-0.5);
      StandardPulseFunction->SetNpx(5000);
      STPad* aPad = aRawEvent->GetPad(RowNum, LayerNum);
      //her I use the StandardPulseFunction to produce the pulse signal.
      for(int iADC = 0;iADC<512;iADC++)
      {
        aPad->SetADC(iADC,StandardPulseFunction->Eval(iADC));
      }
      RawEvent_Dummy->SetPad(aPad);
      StandardPulseFunction->Delete();
    }
  }
/*  
  vector<Int_t>* fHitClusterIDArray = aTrack->GetClusterIDArray();
  int ClusterNum = (*fHitClusterIDArray).size();
//  cout<<"ClusterNum: "<<ClusterNum<<endl;
  int HitTotalNum = 0;
  int HitTotalNum_TrackIDUnmatched = 0;
//  int HitPtrsTotalNum = 0;
  for(int iCluster = 0;iCluster<ClusterNum;iCluster++)
  {
    int ClusterID = (*fHitClusterIDArray)[iCluster];
    STHitCluster* aCluster = (STHitCluster*) fHitClusterArray->At(ClusterID);
    vector<Int_t> *HitIDArray = aCluster->GetHitIDs();
    int HitNum = (*HitIDArray).size();
//    int HitNum_Ptrs = aCluster->GetHitPtrs()->size();
//    HitPtrsTotalNum = HitPtrsTotalNum + HitNum_Ptrs;
//    cout<<"HitNum: "<<HitNum<<" HitNum_Ptrs: "<<HitNum_Ptrs<<endl;
    HitTotalNum += HitNum;
    for(int iHit = 0;iHit<HitNum;iHit++)
    {
//      STHit* aHit = (*aCluster->GetHitPtrs())[iHit];
      int HitID = (*HitIDArray)[iHit];
      STHit* aHit = (STHit*) fHitArray->At(HitID);
      int RowNum = aHit->GetRow();
      int LayerNum = aHit->GetLayer();
      double fTb = aHit->GetTb();
      double fCharge = aHit->GetCharge();
//      cout<<RowNum<<"  "<<LayerNum<<"  "<<fTb<<"  "<<fCharge<<endl;
      StandardPulseFunction = aPulse->GetPulseFunction(aHit);
      StandardPulseFunction->SetParameters(fCharge,fTb-0.5);
      StandardPulseFunction->SetNpx(5000);
      STPad* aPad = aRawEvent->GetPad(RowNum, LayerNum);
      //her I use the StandardPulseFunction to produce the pulse signal.
      for(int iADC = 0;iADC<512;iADC++)
      {
        aPad->SetADC(iADC,StandardPulseFunction->Eval(iADC));
      }
      if(TrackID == aHit->GetTrackID()  || aHit->IsClustered()!=0)
      {
        RawEvent_Dummy->SetPad(aPad);
      }
//      else
//      {
//        cout<<"--->"<<TrackID <<"  "<< aHit->GetTrackID()<<endl;
//        HitTotalNum_TrackIDUnmatched++;
//      }
      StandardPulseFunction->Delete();
    }// for the hit in one cluster
  }// for the cluster in one track
*/
  RawEvent_Dummy->SetIsGood(aRawEvent->IsGood());
  RawEvent_Dummy->SetEventID(aRawEvent->GetEventID());
//  int PadNum = RawEvent_Dummy->GetNumPads();
//  cout<<"HitTotalNum_TrackIDUnmatched: "<<HitTotalNum_TrackIDUnmatched<<" HitPtrsTotalNum: "<<HitPtrsTotalNum<<" HitTotalNum: "<<HitTotalNum<<endl;
//  cout<<"PadNum: "<<PadNum<<endl;

}


