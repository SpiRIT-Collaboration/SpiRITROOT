#include "ST_TrackCut.hh"
ClassImp(ST_TrackCut);

ST_TrackCut::ST_TrackCut()
{
  // later, this value should also input from a configure file.
  TargetPosX[0] = -15; TargetPosX[1] = 15;
  TargetPosY[0] = -246.06; TargetPosY[1] = -206.06;
  TargetPosZ = -11.9084;
  TargetPosZ_Sigma = 1.69675;
  
  Track_DistanceToVertex_Cut = 5; // unit: mm
  
  PIDCut_Num = 0;
}

ST_TrackCut::~ST_TrackCut()
{;}

bool ST_TrackCut::IsVertexInTarget(STVertex* aVertex)
{
  double Vertex_PosX = aVertex->GetPos().X();
  double Vertex_PosY = aVertex->GetPos().Y();
  double Vertex_PosZ = aVertex->GetPos().Z();
  bool TagTem = 0;
  if(Vertex_PosZ>TargetPosZ-3*TargetPosZ_Sigma && Vertex_PosZ<TargetPosZ+3*TargetPosZ_Sigma
   && Vertex_PosX> TargetPosX[0] && Vertex_PosX< TargetPosX[1]
   && Vertex_PosY> TargetPosY[0]&& Vertex_PosY< TargetPosY[1] )
  { TagTem = 1; }
  
return TagTem;
}

bool ST_TrackCut::IsTrackInVertex(STRecoTrack* aTrack,STVertex* aVertex)
{
  double dist = (aTrack->GetPOCAVertex()-aVertex->GetPos()).Mag();
  bool TagTem = 0;
  if( dist<Track_DistanceToVertex_Cut ) { TagTem = 1; }
return TagTem;
}

void ST_TrackCut::ReadCutFile(string FileName_Tem)
{
  cout<<"--> Reading File: "<<FileName_Tem<<" ..."<<endl;
  ifstream infile(FileName_Tem.c_str());
  char buf[1000];
  int buflen = 1000;
  
  while(infile)
  {
    infile.clear(ios::goodbit);
    infile.getline(buf,buflen);
//    cout<<buf<<endl;
    if(buf[0]=='!' || buf[0]==' ' || strlen(buf)<=1) { continue; }
    if(buf[0]=='#')
    {
      istringstream buf_stream(&(buf[1]));
      string CutTag;
      buf_stream>>CutTag;
      PIDCut_Tag[PIDCut_Num] = CutTag;
      string NumTag; int PointNum_Tem = 0;
      infile>>NumTag>>PointNum_Tem;
      if(NumTag!="num") { cout<<"The format of the cut file is WRONG, pay attention!"<<endl; getchar(); return; }
      if(PointNum_Tem<2) { cout<<"Point Num: "<<PointNum_Tem<<" for the "<<CutTag<<". No way! "<<endl; getchar(); return; }
      double dEdX_Tem = 0; double Momentum_Tem = 0;
      PIDCut_Array[PIDCut_Num] = new TCutG(CutTag.c_str(),PointNum_Tem);
      for(int iPoint=0;iPoint<PointNum_Tem;iPoint++)
      {
        infile>>Momentum_Tem>>dEdX_Tem;
        PIDCut_Array[PIDCut_Num]->SetPoint(iPoint,Momentum_Tem,dEdX_Tem);
      }
      cout<<"Load PID cut: "<<CutTag<<"... ... ---Finised!"<<endl;
      PIDCut_Num++;
    }
  }
}

TCutG* ST_TrackCut::GetPIDCut(string CutTag)
{
  for(int i=0;i<PIDCut_Num;i++)
  {
    if(CutTag==PIDCut_Tag[i])
    {
      return PIDCut_Array[i];
    }
  }
  cout<<CutTag<<" cannot be found in the PID Cut file!!! "<<endl; getchar();
return 0;
}

void ST_TrackCut::DrawCut()
{
  TCanvas* c1_Cut = new TCanvas("c1_Cut","c1_Cut",1);
  c1_Cut->cd();
  for(int i=0;i<PIDCut_Num;i++)
  {
    if(i==0) 
    { 
      PIDCut_Array[i]->Draw("AL*");
      PIDCut_Array[i]->GetXaxis()->SetLimits(-500,3000);
      PIDCut_Array[i]->GetYaxis()->SetRangeUser(0,800);
    }
    else { PIDCut_Array[i]->Draw("sameL*"); }
  }
}

void ST_TrackCut::DrawCut(TCanvas* c1_tem)
{
  c1_tem->cd();
  for(int i=0;i<PIDCut_Num;i++)
  { PIDCut_Array[i]->Draw("sameL*"); }
}
