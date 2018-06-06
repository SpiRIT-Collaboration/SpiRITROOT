#include "STPSA_RiseEdge.hh"
ClassImp(STPSA_RiseEdge);

STPSA_RiseEdge::STPSA_RiseEdge()
{
  Func1_STD_Pulse = 0;
  ADCIndex_NoGGNoise = 25;
  fThreshold = 30;
  fMaxADC_Saturated = 3500;
  c1_PadADC_PeakCheck = new TCanvas("c1_PadADC_PeakCheck","c1_PadADC_PeakCheck",1400,1000);
  c1_PadADC_PeakCheck->Divide(3,3);
  h1_PadADC_Value = 0;
  for(int i=0;i<100;i++) { a1_PeakArrow[i] = 0; f1_STDPulse[i] = 0; }
  aSTDPulse = new STPulse();
  STDPulse_ArrayNum = 0;
}

STPSA_RiseEdge::~STPSA_RiseEdge()
{;}

void STPSA_RiseEdge::Analyze(STRawEvent* aRawEvent,TClonesArray* hitArray)
{
  int fNumPads = aRawEvent -> GetNumPads();
  cout<<"PadNum: "<<fNumPads<<endl;
  
  for(int iPad=0;iPad<fNumPads;iPad++)
  {
    STPad* aPad = aRawEvent->GetPad(iPad);
    int PadRowNum = aPad->GetRow();
    int PadLayerNum = aPad->GetLayer();
    cout<<"Pad Row, Layer: ( "<<PadRowNum<<" , "<<PadLayerNum<<" )"<<endl;
    FindPeak_NoGGNoise(aPad);
    
    char tag = getchar();
    if(tag=='q') { break; }
  }
}

void STPSA_RiseEdge::Show_1Pad(STRawEvent* aRawEvent,int LayerNum_tem,int RowNum_tem)
{
  int fNumPads = aRawEvent -> GetNumPads();
  cout<<"PadNum: "<<fNumPads<<endl;
  bool IsStart = 0;
  for(int iPad=0;iPad<fNumPads;iPad++)
  {
    STPad* aPad = aRawEvent->GetPad(iPad);
    int PadRowNum = aPad->GetRow();
    int PadLayerNum = aPad->GetLayer();
    if(IsStart==1 || (PadLayerNum==LayerNum_tem && PadRowNum==RowNum_tem))
    {
      cout<<"Pad Row, Layer: ( "<<PadRowNum<<" , "<<PadLayerNum<<" )"<<endl;
      FindPeak_NoGGNoise(aPad);
      DrawHitSTPulse(PadRowNum,PadLayerNum);
      IsStart=1;
      char tag = getchar();
      if(tag=='q') { break; }
      if(tag=='p') { iPad=iPad-2; getchar(); }
    }
  }
}

void STPSA_RiseEdge::Show_9Pads(STRawEvent* aRawEvent,int LayerNum_tem,int RowNum_tem)
{
  int fNumPads = aRawEvent -> GetNumPads();
  cout<<"PadNum: "<<fNumPads<<endl;
  bool IsStart = 0;
  
  int PadIndex = LayerNum_tem+108*RowNum_tem;
  if(PadIndex>12096) { cout<<"the Pad Index is larger than 12096, pay attention on the Layer and row Index!"<<endl; return; }
  
  for(int iPad=PadIndex;iPad<fNumPads;iPad++)
  {
    STPad* aPad = aRawEvent->GetPad(iPad);
    int PadLayerIndex = aPad->GetLayer();
    int PadRowIndex = aPad->GetRow();
    
    cout<<"Pad Row, Layer: ( "<<PadRowIndex<<" , "<<PadLayerIndex<<" )"<<endl;
    for(int iSTDPulseNum=0;iSTDPulseNum<STDPulse_ArrayNum;iSTDPulseNum++)
    {
      if(f1_STDPulse[iSTDPulseNum]!=0) { f1_STDPulse[iSTDPulseNum]->Delete(); f1_STDPulse[iSTDPulseNum] = 0; }
      STDPulse_ArrayNum = 0;
    }
    for(int i=0;i<3;i++)
    {
      for(int j=0;j<3;j++)
      {
        if(PadRowIndex-1+i>=0 && PadRowIndex-1+i<108 && PadLayerIndex-1+j>=0 && PadLayerIndex-1+j<112)
        {
          c1_PadADC_PeakCheck->cd(i*3+j+1)->Clear();
          int PadRowIndex_Around = PadRowIndex-1+i;
          int PadLayerIndex_Around = PadLayerIndex-1+j;
//          cout<<"AroundPad ID: "<<"  "<<PadRowIndex_Around <<"  "<<PadLayerIndex_Around<<endl;
          STPad* aPad_Around = aRawEvent->GetPad(112*PadRowIndex_Around+PadLayerIndex_Around);
          //check again
          int PadRowIndex_Around_tem = aPad_Around->GetRow();
          int PadLayerIndex_Around_tem = aPad_Around->GetLayer();
          if(PadRowIndex_Around_tem!=PadRowIndex_Around || PadLayerIndex_Around_tem!=PadLayerIndex_Around) 
          { cout<<"GetPad ID: "<<PadRowIndex_Around_tem<<"  "<< PadLayerIndex_Around_tem<<endl; return;}
          FindPeak_NoGGNoise(aPad_Around);
//          DrawHitSTPulse(PadRowIndex_Around,PadLayerIndex_Around);
        }
      }
    }
    IsStart=1;
    char tag = getchar();
    if(tag=='q') { break; }
    if(tag=='p') { iPad--; }
  }
}

void STPSA_RiseEdge::FindPeak_NoGGNoise(STPad* aPad)
{
  int PulseNum_Dummy = 0;
  int PulsePeakTb_Dummy[100] = {0};
  int PulseStartTb_Dummy[100] = {0};
  
//  int PulseTb[100] = {0};
  //judge the peak of the pulse shape: this function is just like a digital filtter.
  for(int iADC=0;iADC<ADCNUM;iADC++)
  {
    PadADCArray[iADC] = aPad->GetADC(iADC);
    if(iADC>ADCIndex_NoGGNoise && iADC<ADCNUM-5)
    {// judge the raise edge. only analyze the pulse, whose height [200,3500].
      if(PadADCArray[iADC-2]>fThreshold && PadADCArray[iADC-2]<fMaxADC_Saturated// judge the peak(iADC-5<iADC-4<iADC-3< (iADC-2=max) >iADC-1>iADC )
      && PadADCArray[iADC-2]>PadADCArray[iADC-1] && PadADCArray[iADC-1]>PadADCArray[iADC]
      && PadADCArray[iADC-2]>PadADCArray[iADC-3] && PadADCArray[iADC-3]>PadADCArray[iADC-4] && PadADCArray[iADC-4]>PadADCArray[iADC-5]
      && PadADCArray[iADC-5]>-10 && PadADCArray[iADC-4]>0 && PadADCArray[iADC-1]>0 && PadADCArray[iADC]>-10)
      {
        if(PadADCArray[iADC-2]-PadADCArray[iADC-5]>30)
        {
          PulsePeakTb_Dummy[PulseNum_Dummy] = iADC-2;
          PulseNum_Dummy++;
        }
        else if( PadADCArray[iADC-5]>PadADCArray[iADC-6] && PadADCArray[iADC-2]-PadADCArray[iADC-6]>30 && PadADCArray[iADC-5]>0 && PadADCArray[iADC-6]>-10 )
        {
          PulsePeakTb_Dummy[PulseNum_Dummy] = iADC-2;
          PulseNum_Dummy++;
        }
        else if( PadADCArray[iADC-5]>PadADCArray[iADC-6] && PadADCArray[iADC-6]>PadADCArray[iADC-7] && PadADCArray[iADC-2]-PadADCArray[iADC-7]>30 && PadADCArray[iADC-6]>0 && PadADCArray[iADC-7]>-10 )
        {
          PulsePeakTb_Dummy[PulseNum_Dummy] = iADC-2;
          PulseNum_Dummy++;
        }
      }
      //the method to judge the saturated pulse. (iADC-7<iADC-6<iADC-5<iADC-4< iADC-3<( max=iADC-2 ) >iADC-1>iADC), here I only consider the saturated time = 1Time bucket.
      else if(PadADCArray[iADC-2]>fMaxADC_Saturated 
           && PadADCArray[iADC-3]<fMaxADC_Saturated && PadADCArray[iADC-1]<fMaxADC_Saturated
           && PadADCArray[iADC-2]>PadADCArray[iADC-1] && PadADCArray[iADC-1]>PadADCArray[iADC]
           && PadADCArray[iADC-3]>PadADCArray[iADC-4] && PadADCArray[iADC-4]>PadADCArray[iADC-5] && PadADCArray[iADC-5]>PadADCArray[iADC-6] && PadADCArray[iADC-6]>PadADCArray[iADC-7] 
           && PadADCArray[iADC-7]>0 && PadADCArray[iADC]>0 && PadADCArray[iADC]< fMaxADC_Saturated
           && PadADCArray[iADC-3]-PadADCArray[iADC-7]>30)
      {
        PulsePeakTb_Dummy[PulseNum_Dummy] = iADC-3;
        PulseNum_Dummy++;
      }
      //the method to judge the saturated pulse. (iADC-6<iADC-5<iADC-4< (iADC-3=max=iADC-2) >iADC-1>iADC), here I only consider the saturated time = 2Time bucket.
      else if(PadADCArray[iADC-3]>fMaxADC_Saturated && PadADCArray[iADC-2]>fMaxADC_Saturated 
           && PadADCArray[iADC-4]<fMaxADC_Saturated && PadADCArray[iADC-1]<fMaxADC_Saturated
           && PadADCArray[iADC-2]>PadADCArray[iADC-1] && PadADCArray[iADC-1]>PadADCArray[iADC]
           && PadADCArray[iADC-4]>PadADCArray[iADC-5] && PadADCArray[iADC-5]>PadADCArray[iADC-6]
           && PadADCArray[iADC-7]>0 && PadADCArray[iADC]>0 && PadADCArray[iADC]< fMaxADC_Saturated
           && PadADCArray[iADC-4]-PadADCArray[iADC-7]>30)
      {
        PulsePeakTb_Dummy[PulseNum_Dummy] = iADC-3;
        PulseNum_Dummy++;
      }
      // the below is for judging the saturated time==3 Time bucket. (iADC-7<iADC-6<iADC-5< (iADC-4~iADC-3~iADC-2) >iADC-1>iADC)
      else if(PadADCArray[iADC-4]>fMaxADC_Saturated && PadADCArray[iADC-3]>fMaxADC_Saturated && PadADCArray[iADC-2]>fMaxADC_Saturated
           && PadADCArray[iADC-5]<fMaxADC_Saturated && PadADCArray[iADC-1]<fMaxADC_Saturated
           && PadADCArray[iADC-2]>PadADCArray[iADC-1] && PadADCArray[iADC-1]>PadADCArray[iADC]
           && PadADCArray[iADC-5]>PadADCArray[iADC-6] && PadADCArray[iADC-6]>PadADCArray[iADC-7]
           && PadADCArray[iADC-7]>0 && PadADCArray[iADC]>0
           && PadADCArray[iADC-5]-PadADCArray[iADC-7]>30)
      {
        PulsePeakTb_Dummy[PulseNum_Dummy] = iADC-4;
        PulseNum_Dummy++;
      }
      // the below is for judging the saturated time==4 Time bucket. (ADC-8<iADC-7<iADC-6<(iADC-5~iADC-4~iADC-3~iADC-2) >iADC-1>iADC)
      else if(PadADCArray[iADC-5]>fMaxADC_Saturated && PadADCArray[iADC-4]>fMaxADC_Saturated && PadADCArray[iADC-3]>fMaxADC_Saturated && PadADCArray[iADC-2]>fMaxADC_Saturated
           && PadADCArray[iADC-6]<fMaxADC_Saturated && PadADCArray[iADC-1]<fMaxADC_Saturated
           && PadADCArray[iADC-2]>PadADCArray[iADC-1] && PadADCArray[iADC-1]>PadADCArray[iADC]
           && PadADCArray[iADC-6]>PadADCArray[iADC-7] && PadADCArray[iADC-7]>PadADCArray[iADC-8]
           && PadADCArray[iADC-8]>0 && PadADCArray[iADC]>0
           && PadADCArray[iADC-6]-PadADCArray[iADC-8]>30)
      {
        PulsePeakTb_Dummy[PulseNum_Dummy] = iADC-5;
        PulseNum_Dummy++;
      }
      // the below is for judging the saturated time==4 Time bucket. (ADC-9<ADC-8<iADC-7<(iADC-6~iADC-5~iADC-4~iADC-3~iADC-2) >iADC-1>iADC)
      else if(PadADCArray[iADC-6]>fMaxADC_Saturated && PadADCArray[iADC-5]>fMaxADC_Saturated && PadADCArray[iADC-4]>fMaxADC_Saturated && PadADCArray[iADC-3]>fMaxADC_Saturated && PadADCArray[iADC-2]>fMaxADC_Saturated
           && PadADCArray[iADC-7]<fMaxADC_Saturated && PadADCArray[iADC-1]<fMaxADC_Saturated
           && PadADCArray[iADC-2]>PadADCArray[iADC-1] && PadADCArray[iADC-1]>PadADCArray[iADC]
           && PadADCArray[iADC-7]>PadADCArray[iADC-8] && PadADCArray[iADC-8]>PadADCArray[iADC-9] 
           && PadADCArray[iADC-9]>0 && PadADCArray[iADC]>0
           && PadADCArray[iADC-7]-PadADCArray[iADC-9]>30)
      {
        PulsePeakTb_Dummy[PulseNum_Dummy] = iADC-6;
        PulseNum_Dummy++;
      }
    }
  }
  cout<<"PeakNum: "<<PulseNum_Dummy<<endl;
  Draw(PulseNum_Dummy,PulsePeakTb_Dummy);
/*  //if the rise edge is built on the falling tail of the previous pulse, get the start time with centroid method, and the height
  for(int iPulse=0;iPulse<PulseNum_Dummy;iPulse++)
  {
    // here I suppose the rise time cannot be larger than 10 time bucket.
    int iADC_Peak = PulsePeakTb_Dummy[iPulse]; 
    int RiseTb_MaxNum = 10; // this maximum num is just a temporary 
    for(int iRise = 0;iRise<RiseTb_MaxNum-3;iRise++)
    {
      if(PadADCArray[iADC_Peak-4-iRise]>PadADCArray[iADC_Peak-3-iRise] || PadADCArray[iADC_Peak-4-iRise]<0 ) 
      {
        PulseStartTb_Dummy[iPulse] = iADC_Peak-3-iRise;
        break;
      }
      if(iRise==4) { PulseStartTb_Dummy[iPulse] = iADC_Peak-3-iRise; }
    }
  }
*/
  
}

void STPSA_RiseEdge::AnaPeak_noGGNoise()
{
  
}

// use this function to check the peak finding.
void STPSA_RiseEdge::Draw(int PulseNum_tem, int* PulsePeakADC_tem)
{
  if(h1_PadADC_Value==0) { h1_PadADC_Value = new TH1D("h1_PadADC_Value",";TimeBucket;ADC",270,0,270); }
//  c1_PadADC_PeakCheck->cd();
  h1_PadADC_Value->Reset();
  for(int i=0;i<270;i++)
  {
    h1_PadADC_Value->Fill(i,PadADCArray[i]);
  }
  h1_PadADC_Value->Draw("hist");
//  h1_PadADC_Value->GetYaxis()->SetRangeUser(0,4096);
  for(int i=0;i<PulseNum_tem;i++)
  {
    if(a1_PeakArrow[i]==0) { a1_PeakArrow[i] = new TArrow(PulsePeakADC_tem[i]+0.5,0,PulsePeakADC_tem[i]+0.5,PadADCArray[PulsePeakADC_tem[i]]);}
    else
    {
      a1_PeakArrow[i]->SetX1(PulsePeakADC_tem[i]+0.5);
      a1_PeakArrow[i]->SetY1(0);
      a1_PeakArrow[i]->SetX2(PulsePeakADC_tem[i]+0.5);
      a1_PeakArrow[i]->SetY2(PadADCArray[PulsePeakADC_tem[i]]);
    }
    a1_PeakArrow[i]->SetLineWidth(3);
    a1_PeakArrow[i]->SetLineColor(2);
    a1_PeakArrow[i]->Draw("same");
  }
  c1_PadADC_PeakCheck->Update();
}

void STPSA_RiseEdge::DrawHitSTPulse(int PadRowNum,int PadLayerNum)
{
  int HitNum = HitArray->GetEntries();
  int PadHitNum = 0;
  for(int iHit = 0;iHit<HitNum;iHit++)
  {
    STHit* aHit = (STHit*)HitArray->At(iHit);
    int HitRowNum = aHit->GetRow();
    int HitLayerNum = aHit->GetLayer();
    if(HitRowNum == PadRowNum && HitLayerNum == PadLayerNum)
    {
      double fTb = aHit->GetTb();
      double fCharge = aHit->GetCharge();
      cout<<"fTb: "<<fTb<<"  fCharge: "<<fCharge<<endl;
      f1_STDPulse[PadHitNum] = aSTDPulse->GetPulseFunction(aHit);
      f1_STDPulse[PadHitNum]->SetNpx(5000);
      f1_STDPulse[PadHitNum]->Draw("same");
      PadHitNum++;
     }
  }
  STDPulse_ArrayNum += PadHitNum;
  c1_PadADC_PeakCheck->Update();
}








