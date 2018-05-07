#include "STEmbedTrackContainer.hh"

ClassImp(STEmbedTrackContainer)

STEmbedTrackContainer::STEmbedTrackContainer()
{
  RawEvent = 0;
  RecoTrack = 0;
  Vertex = 0;
  f1_data = 0;
  t1_data = 0;
  EvtNum = 0;
  PosLimit = 2;//unit:mm;
  MomentumAngleLimit = Pi()/180.0; // unit: rad,default: 1 degree;
  MomentumAmpLimit = 0.1; //10% momentum resolution;
  
  fRawEventArray = new TClonesArray("STRawEvent",1);
  fRecoTrackArray = new TClonesArray("STRecoTrack",1);
  fVertexArray = new TClonesArray("STVertex",1);
  
  RunNo = 0;
}

STEmbedTrackContainer::~STEmbedTrackContainer()
{
  if(f1_data!=0) { f1_data->Close(); }
}

bool STEmbedTrackContainer::Init_StoreFile(string FileName)
{
  ifstream infile(FileName.c_str());
  //initialize the store file, and tree.
  if(!infile.good())
  {//build a new file;
    cout<<"--->"<<FileName<<" NOT exist, and will be created!"<<endl;
    EvtNum = 0;
    infile.close();
    f1_data = new TFile(FileName.c_str(),"update");
    t1_data = new TTree("cbmsim","Embedded event");
    t1_data->Branch("STRawEvent",fRawEventArray);
    t1_data->Branch("STRecoTrack",fRecoTrackArray);
    t1_data->Branch("STVertex",fVertexArray);
    t1_data->Branch("RunNo",&RunNo);
  }
  else if(infile.good())
  {//update the tree and file.
    cout<<"--->"<<FileName<<" exist, you can update new track!"<<endl;
    infile.close();
    f1_data = new TFile(FileName.c_str(),"update");
    t1_data = (TTree*) f1_data->Get("cbmsim");
    if(t1_data!=0)
    {
      t1_data->SetBranchAddress("STRawEvent",&fRawEventArray);
      t1_data->SetBranchAddress("STRecoTrack",&fRecoTrackArray);
      t1_data->SetBranchAddress("RunNo",&RunNo);
      t1_data->SetBranchAddress("STVertex",&fVertexArray);
      EvtNum = t1_data->GetEntries();
      cout<<FileName<<" exist "<<EvtNum<<" events already!"<<endl;
    }
    else
    {
      cout<<"--->"<<FileName<<" exist, but no tree: cbmsim, this tree will be created!"<<endl;
      EvtNum = 0;
      t1_data = new TTree("cbmsim","Embedded event");
      t1_data->Branch("STRawEvent",fRawEventArray);
      t1_data->Branch("STRecoTrack",fRecoTrackArray);
      t1_data->Branch("STVertex",fVertexArray);
      t1_data->Branch("RunNo",&RunNo);
    }
  }
  
  //initialize the index for the track
  for(int i=0;i<EvtNum;i++)
  {
    t1_data->GetEntry(i);
    RecoTrack = (STRecoTrack*) fRecoTrackArray -> At(0); // here I only embed one track into the event.
    
    MomentumTheta[i] = RecoTrack->GetMomentumTargetPlane().Theta();
    MomentumPhi[i] = RecoTrack->GetMomentumTargetPlane().Phi();
    MomentumAmp[i] = RecoTrack->GetMomentumTargetPlane().Mag();
    MomentumX[i] = RecoTrack->GetMomentumTargetPlane().X();
    MomentumY[i] = RecoTrack->GetMomentumTargetPlane().Y();
    MomentumZ[i] = RecoTrack->GetMomentumTargetPlane().Z();
    
    PosX[i] = RecoTrack->GetPosTargetPlane().X();
    PosY[i] = RecoTrack->GetPosTargetPlane().Y();
    PosZ[i] = RecoTrack->GetPosTargetPlane().Z();
    PID[i] = RecoTrack->GetPID();
  }

return 1;
}

bool STEmbedTrackContainer::Is_NeedStore(STRecoTrack* RecoTrack_tem)
{
  TVector3 fMomentumTargetPlane = RecoTrack_tem->GetMomentumTargetPlane();
  TVector3 fPosTargetPlane = RecoTrack_tem->GetPosTargetPlane();
  if(fMomentumTargetPlane.Mag()==0)
  {
    cout<<" You want to store a track with 0MeV. No way!"<<endl;
    return 0;
  }
  bool IsStore = 1;
  for(int i=0;i<EvtNum;i++)
  {
    if(PID[i]!=RecoTrack_tem->GetPID()) { continue; }
    //Position is different;
    double PosDelta_tem = Sqrt(Power(fPosTargetPlane.X()-PosX[i],2)+Power(fPosTargetPlane.Y()-PosY[i],2)+Power(fPosTargetPlane.Z()-PosZ[i],2));
    //Magnitude is different;
    double MomentumMagDelta_tem = fabs((MomentumAmp[i]-fMomentumTargetPlane.Mag())/(fMomentumTargetPlane.Mag()));
    //the angle of this two vector is different;
    double AngleDelta_tem = ACos((MomentumX[i]*fMomentumTargetPlane.X()+MomentumY[i]*fMomentumTargetPlane.Y()+MomentumZ[i]*fMomentumTargetPlane.Z())/(MomentumAmp[i]*fMomentumTargetPlane.Mag()));
    
    if(PosDelta_tem<PosLimit && MomentumMagDelta_tem<MomentumAmpLimit && AngleDelta_tem<MomentumAngleLimit)
    { IsStore = 0; break;}
  }

return IsStore;
}

void STEmbedTrackContainer::AddTrack(STRawEvent* RawEvent_tem,STRecoTrack* RecoTrack_tem,STVertex* Vertex_tem)
{//here just copy the data in the STRawEvent and STRecoTrack to the container's tree;
  f1_data->cd();
  fRawEventArray->Clear();
  fRecoTrackArray->Clear();
  fVertexArray->Clear();
  if( RawEvent_tem!=0 ) { new ((*fRawEventArray)[0]) STRawEvent(RawEvent_tem); }
  if( RecoTrack_tem!=0 ) { new ((*fRecoTrackArray)[0]) STRecoTrack(RecoTrack_tem); }
  if( Vertex_tem!=0 ) { new ((*fVertexArray)[0]) STVertex(Vertex_tem); }
  
  t1_data->Fill();
  EvtNum = t1_data->GetEntries();
  //update the index.
  if( RecoTrack_tem!=0 )
  {
    MomentumTheta[EvtNum-1] = RecoTrack_tem->GetMomentumTargetPlane().Theta();
    MomentumPhi[EvtNum-1] = RecoTrack_tem->GetMomentumTargetPlane().Phi();
    MomentumAmp[EvtNum-1] = RecoTrack_tem->GetMomentumTargetPlane().Mag();
    MomentumX[EvtNum-1] = RecoTrack_tem->GetMomentumTargetPlane().X();
    MomentumY[EvtNum-1] = RecoTrack_tem->GetMomentumTargetPlane().Y();
    MomentumZ[EvtNum-1] = RecoTrack_tem->GetMomentumTargetPlane().Z();
  
    PosX[EvtNum-1] = RecoTrack_tem->GetPosTargetPlane().X();
    PosY[EvtNum-1] = RecoTrack_tem->GetPosTargetPlane().Y();
    PosZ[EvtNum-1] = RecoTrack_tem->GetPosTargetPlane().Z();
    PID[EvtNum-1] = RecoTrack_tem->GetPID();
  }
  cout<<"EvtNum: "<<EvtNum<<endl;
  cout<<"Store 1 track, PID:"<< PID[EvtNum-1] 
      <<" Pos:("<<PosX[EvtNum-1]<<","<<PosY[EvtNum-1]<<","<<PosZ[EvtNum-1]<<") ; "
      <<" Momentum:("<<MomentumX[EvtNum-1]<<","<<MomentumY[EvtNum-1]<<","<<MomentumZ[EvtNum-1]<<")"<<endl;
}

STRawEvent* const STEmbedTrackContainer::GetEvtTrack(int NeedPID,double* NeedPos, double* NeedMomentum,double* ResErr_Find)
//ResErr_Find[0]:pos, ResErr_Find[1]: Amplitude; ResErr_Find[2]: Angle
// Here first ensure the PosErr(1st level) is allowed, then get the closest momentum(2nd level),.
{
  bool IsFind = 0;
  int EvtID_tem = 0;
  double PreErr_tem[3] = {0};
  double Err_tem[3] = {0};
  EvtNum = t1_data->GetEntries();
  if(NeedMomentum[2]<=0) { cout<<" NO track with 0 MeV/c. Absolutely No!" <<endl; return 0;}
  
  for(int i=0;i<EvtNum;i++)
  {
    if(PID[i] != NeedPID) { continue; }
    Err_tem[0] = Sqrt(Power(NeedPos[0]-PosX[i],2)+Power(NeedPos[1]-PosY[i],2)+Power(NeedPos[2]-PosZ[i],2));
    Err_tem[1] = fabs((NeedMomentum[2]-MomentumAmp[i])/NeedMomentum[2]);
    double X_tem = NeedMomentum[2]*Sin(NeedMomentum[0])*Cos(NeedMomentum[1]);
    double Y_tem = NeedMomentum[2]*Sin(NeedMomentum[0])*Sin(NeedMomentum[1]);
    double Z_tem = NeedMomentum[2]*Cos(NeedMomentum[0]);
    Err_tem[2] = ACos((MomentumX[i]*X_tem+MomentumY[i]*Y_tem+MomentumZ[i]*Z_tem)/(MomentumAmp[i]*NeedMomentum[2]));
    cout<<Err_tem[0]<<"  "<<Err_tem[1]<<"  "<<Err_tem[2]<<endl;
    
    if(Err_tem[0]<ResErr_Find[0] && Err_tem[1]<ResErr_Find[1] && Err_tem[2]<ResErr_Find[2])
    {
      if(IsFind==1)
      {
        double PreErr_Amp = Power(PreErr_tem[1],2)+Power(PreErr_tem[2],2);
        double CurErr_Amp = Power(Err_tem[1],2)+Power(Err_tem[2],2);
        if(PreErr_Amp<CurErr_Amp)
        {// what you find is not better than the previous one, so EvtID will not be changed.
          ;//nothing to do~~
        }
        else
        {// update the track;
          EvtID_tem = i;
          IsFind = 1;
          for(int j=0;j<3;j++) { PreErr_tem[j] = Err_tem[j]; }
        }
      }
      else
      {
        EvtID_tem = i;
        IsFind = 1;
        for(int j=0;j<3;j++) { PreErr_tem[j] = Err_tem[j]; }
      }
    }
  }
  
  if(IsFind ==0)
  {
    cout<<"Cannot find a track, which meets your ResErr requirement! So return 0"<<endl; 
    return 0;
  }
  f1_data->cd();
  t1_data->GetEntry(EvtID_tem);
  RawEvent = (STRawEvent*) fRawEventArray -> At(0);
  
return RawEvent;
}


