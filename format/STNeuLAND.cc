#include "STNeuLAND.hh"

ClassImp(STNeuLAND);

STNeuLAND* STNeuLAND::fInstance = nullptr;

STNeuLAND* STNeuLAND::GetNeuLAND() {
  if (fInstance != nullptr)
    return fInstance;
  return new STNeuLAND();
}


STNeuLAND::STNeuLAND()
{
  if (fHistPulse == nullptr)
  {
    auto pulseTimeRange1 = 2*(-fRiseTime-fTimeErrorSigma)*TMath::Log(2);
    auto pulseTimeRange2 = 8*(fDecayTime+fTimeErrorSigma)*TMath::Log(2);
    fPulseTDCRange = Int_t(ConvertTimeToTDC(pulseTimeRange2));

    fFuncPulse = new TF1("NLPulse", this, &STNeuLAND::PulseWithError, pulseTimeRange1, pulseTimeRange2, 4, "STNeuLAND", "PulseWithError");
    fFuncPulse -> SetParameters(0, fTimeErrorSigma, 1/fRiseTime, 1/fDecayTime);

    auto normalizePulse = 0.;
    fHistPulse = new TH1D("hist_pulse","pulse function;TDC (ns)",fPulseTDCRange,0,fPulseTDCRange);
    for (auto bin=1; bin<=fNumTDCBins; ++bin) {
      auto bin_center = fHistPulse -> GetBinCenter(bin);
      auto bin_content = fFuncPulse->Eval(ConvertTDCToTime(bin_center));
      fHistPulse -> Fill(bin_center, ConvertTimeToTDC(bin_content));
      normalizePulse += ConvertTimeToTDC(bin_content);
    }
    fHistPulse -> Scale(1./normalizePulse);
  }

  fInstance = this;
}

/// convolution of pulse and gaussian (error)
Double_t STNeuLAND::PulseWithError(Double_t *xx, Double_t *pp)
{
  Double_t x = xx[0] - pp[0];
  //if (x < 0) return 0;

  Double_t sigma = pp[1];
  Double_t lambda1 = pp[2];
  Double_t lambda2 = pp[3];

  Double_t value = 0;

  /// convolution of norm-gaus : 1./sigma/sqrt(2pi) * exp( -x^2 / (2*sigma^2) )
  //           and exponential : exp( -lambda * x ) 

  /// slow component lambda1
  {
    Double_t ssl = sigma * sigma * lambda1;
    Double_t part1 = exp( -lambda1 * (x - ssl/2.) );
    Double_t inerf = (x - ssl) / sqrt(2.) / sigma;
    Double_t part2 = 1 + TMath::Erf(inerf);
    value -= 0.5 * part1 * part2;
  }

  /// fast component lambda2
  {
    Double_t ssl = sigma * sigma * lambda2;
    Double_t part1 = exp( -lambda2 * (x - ssl/2.) );
    Double_t inerf = (x - ssl) / sqrt(2.) / sigma;
    Double_t part2 = 1 + TMath::Erf(inerf);
    value += 0.5 * part1 * part2;
  }

  return value;
}

TVector3 STNeuLAND::GlobalPos(TVector3 localPos)
{
  TVector3 globalPos = localPos;
  globalPos.RotateY( fRotYNeuland_deg * TMath::DegToRad() );
  globalPos += TVector3(fOffxNeuland, fOffyNeuland, fOffzNeuland);
  return globalPos;
}



/// Convert global position to local position
TVector3 STNeuLAND::LocalPos(TVector3 globalPos)
{
  TVector3 localPos = globalPos;
  localPos -= TVector3(fOffxNeuland, fOffyNeuland, fOffzNeuland);
  localPos.RotateY( -fRotYNeuland_deg * TMath::DegToRad() );
  return localPos;
}



/// Get layer from bar-id
Int_t STNeuLAND::GetLayer(Int_t mcDetID)
{
  if (mcDetID < fFirstMCDetectorID || mcDetID >= fLastMCDetectorID )
    return -1;

  Int_t layer = Int_t((mcDetID-fFirstMCDetectorID)/fWidthBar);
  return layer;
}



/// Get row from bar-id
Int_t STNeuLAND::GetRow(Int_t mcDetID)
{
  auto layer = GetLayer(mcDetID);
  if (layer < 0)
    return -1;
  Int_t row = (mcDetID - fFirstMCDetectorID) - layer*fNumRows;
  return row;
}



/* Get (local)  center position from mc-det-id
 *
 * [neuland-bar-id]  = [mc-detector-id] - 4000
 * [neuland-veto-id] = [mc-detector-id] - 5000
 *
 * even 50s
 *   0 + 0-49 : layer 0, from bottom(0) to top(49)
 * 100 + 0-49 : layer 2, from bottom(0) to top(49)
 * 200 + 0-49 : layer 4, from bottom(0) to top(49)
 * 300 + 0-49 : layer 6, from bottom(0) to top(49)
 *
 * odd 50s
 *  50 + 0-49 : layer 1, from     -x(0) to  +x(49)
 * 150 + 0-49 : layer 3, from     -x(0) to  +x(49)
 * 250 + 0-49 : layer 5, from     -x(0) to  +x(49)
 * 250 + 0-49 : layer 7, from     -x(0) to  +x(49)
 */
TVector3 STNeuLAND::GetBarLocalPosition(Int_t mcDetID)
{
  auto layer = GetLayer(mcDetID);
  auto row = GetRow(mcDetID);

  TVector3 localPosition(0,0,0);
  if (layer >= 0 && row >= 0 && layer < fNumLayers && row < fNumRows) {

    localPosition.SetZ((layer+.5)*fWidthBar);

    if (IsAlongXNotY(mcDetID)) localPosition.SetY(-fHalfLengthBar + (row+.5)*fWidthBar);
    else                       localPosition.SetX(-fHalfLengthBar + (row+.5)*fWidthBar);
  }

  return localPosition;
}



Int_t STNeuLAND::IsAlongXNotY(Int_t mcDetID)
{
  if (GetLayer(mcDetID)%2==0)
    return true;
  return false;
}



Int_t STNeuLAND::FindBarID(TVector3 pos)//Double_t x, Double_t y, Double_t z)
{
  Double_t x = pos.x();
  Double_t y = pos.y();
  Double_t z = pos.z();

  Int_t layer = z / fdzLayer;
  Int_t row = -1;

  if (layer%2==0) // x
    row = (y + fHalfLengthBar) / fWidthBar;
  else // y
    row = (x + fHalfLengthBar) / fWidthBar;

  return FindBarID(layer, row);
}


Int_t STNeuLAND::FindBarID(Int_t layer, Int_t row)
{
  if (layer < 0 || layer >= fNumLayers || row < 0 || row >= fNumRows)
    return -1;

  auto id = 4000 + layer * 50 + row;
  return id;
}



/// Get (global) center position from mc-det-id
TVector3 STNeuLAND::GetBarGlobalPosition(Int_t mcDetID)
{
  return LocalPos(GetBarLocalPosition(mcDetID));
}





TCanvas *STNeuLAND::DrawLocal(TString name)
{
  name = name + "_cvs_local";
  auto cvs = new TCanvas(name,name,1100,500);
  cvs -> Divide(2,1);
  for (auto i : {0,1,2}) cvs -> cd(i) -> SetMargin(0.19,0.055,0.16,0.12);

  cvs -> cd(1);
  DrawLocalFrameDownStream();
  DrawLocalNLGraphDownStream("same");

  cvs -> cd(2);
  DrawLocalFrameSide();
  DrawLocalNLGraphSide("same");

  return cvs;
}

void STNeuLAND::DrawLocalFrameDownStream(Option_t *opt)
{
  if (fLocalFrameDownStream == nullptr) {
    fLocalFrameDownStream  = new TH2D("NLTopLocalFrame","Local DownStream;#it{x} (mm);#it{y} (mm)",
        50,-fHalfLengthBar,fHalfLengthBar, 50,-fHalfLengthBar,fHalfLengthBar);
  }

  fLocalFrameDownStream -> Draw(opt);
}


void STNeuLAND::DrawLocalFrameSide(Option_t *opt)
{
  if (fLocalFrameSide == nullptr)
    fLocalFrameSide = new TH2D("NLSideLocalFrame","Local Side;#it{z} (mm);#it{y} (mm)", 8, 0, fdzNl, 50, -fHalfLengthBar,fHalfLengthBar);
  fLocalFrameSide -> Draw(opt);
}

void STNeuLAND::DrawLocalNLGraphDownStream(Option_t *opt)
{
  if (fLocalNLGraphDownStreamArray.size() == 0)
  {
    for (auto row=1; row<50; ++row)
    {
      auto line1 = new TLine(-fHalfLengthBar,row*fWidthBar-fHalfLengthBar,+fHalfLengthBar,row*fWidthBar-fHalfLengthBar);
      auto line2 = new TLine(row*fWidthBar-fHalfLengthBar,-fHalfLengthBar,row*fWidthBar-fHalfLengthBar,+fHalfLengthBar);
      line1 -> SetLineColor(kGray);
      line2 -> SetLineColor(kGray);
      fLocalNLGraphDownStreamArray.push_back(line1);
      fLocalNLGraphDownStreamArray.push_back(line2);
    }
  }

  for (auto line : fLocalNLGraphDownStreamArray)
    line -> Draw(opt);
}

void STNeuLAND::DrawLocalNLGraphSide(Option_t *opt)
{
  if (fLocalNLGraphSideArray.size() == 0)
  {
    for (auto layer=1; layer<8; ++layer) {
      auto line1 = new TLine(layer*fWidthBar,-fHalfLengthBar,layer*fWidthBar,+fHalfLengthBar);
      line1 -> SetLineColor(kGray);
      fLocalNLGraphSideArray.push_back(line1);
    }

    for (auto layer : {0,2,4,6})
      for (auto row=1; row<50; ++row) {
        //auto line2 = new TLine(0,row*fWidthBar-fHalfLengthBar,fdzNl,row*fWidthBar-fHalfLengthBar);
        auto line2 = new TLine( layer   *fdzLayer ,row*fWidthBar-fHalfLengthBar,
                               (layer+1)*fdzLayer ,row*fWidthBar-fHalfLengthBar);
        line2 -> SetLineColor(kGray);
        fLocalNLGraphSideArray.push_back(line2);
      }
  }

  for (auto line : fLocalNLGraphSideArray)
    line -> Draw(opt);
}





TCanvas *STNeuLAND::DrawGlobal(TString name)
{
  name = name + "_cvs_global";
  auto cvs = new TCanvas(name,name,1100,500);
  cvs -> Divide(2,1);
  for (auto i : {0,1,2}) cvs -> cd(i) -> SetMargin(0.19,0.055,0.16,0.12);

  cvs -> cd(1);
  DrawGlobalFrameTop();
  DrawGlobalNLGraphTop("same");

  cvs -> cd(2);
  DrawGlobalFrameSide();
  DrawGlobalNLGraphSide("same");

  return cvs;
}

void STNeuLAND::DrawGlobalFrameTop(Option_t *opt)
{
  if (fGlobalFrameTop == nullptr)
    fGlobalFrameTop  = new TH2D("NLTopGlobalFrame","Global Top;#it{z} (mm);#it{x} (mm)",400,-500,9000,400,-800,6000);
  fGlobalFrameTop -> Draw(opt);
}


void STNeuLAND::DrawGlobalFrameSide(Option_t *opt)
{
  if (fGlobalFrameSide == nullptr)
    fGlobalFrameSide = new TH2D("NLSideGlobalFrame","Global Side;#it{z} (mm);#it{y} (mm)",400,-500,9000,400,-1500,1500);
  fGlobalFrameSide -> Draw(opt);
}

void STNeuLAND::DrawGlobalNLGraphTop(Option_t *opt)
{
  if (fGlobalNLGraphTop == nullptr)
  {
    fGlobalNLGraphTop = new TGraph();
    for (auto point :
        {TVector3(0,0,0),
        TVector3(-fHalfLengthBar,0,0),
        TVector3(-fHalfLengthBar,0,fdzNl),
        TVector3(+fHalfLengthBar,0,fdzNl),
        TVector3(+fHalfLengthBar,0,0),
        TVector3(0,0,0)})
    {
      auto gpoint = GlobalPos(point);
      fGlobalNLGraphTop -> SetPoint(fGlobalNLGraphTop->GetN(), gpoint.Z(), gpoint.X());
    }

    TVector3 posNLC = GlobalPos(TVector3(0,0,0));
    TVector3 posNLL = GlobalPos(TVector3(-fHalfLengthBar,0,0));
    TVector3 posNLR = GlobalPos(TVector3(+fHalfLengthBar,0,0));



    fGlobalNLGuideLineC = new TLine(fZTarget,0, posNLC.Z(),posNLC.X());
    fGlobalNLGuideLineC -> SetLineColor(kGray);
    fGlobalNLGuideLineC -> SetLineStyle(1);

    auto angleC = atan2(abs(posNLC.X()),posNLC.Z()-fZTarget)*TMath::RadToDeg();
    fGlobalNLGuideTextC = new TLatex(posNLC.Z(), posNLC.X(),Form("%.1f^{#circ}",angleC));
    fGlobalNLGuideTextC -> SetTextAlign(32);
    fGlobalNLGuideTextC -> SetTextFont(132);
    fGlobalNLGuideTextC -> SetTextColor(kGray+1);



    fGlobalNLGuideLineR = new TLine(fZTarget,0, posNLR.Z(),posNLR.X());
    fGlobalNLGuideLineR -> SetLineColor(kGray);
    fGlobalNLGuideLineR -> SetLineStyle(1);

    auto angleR = atan2(abs(posNLR.X()),posNLR.Z()-fZTarget)*TMath::RadToDeg();
    fGlobalNLGuideTextR = new TLatex(posNLR.Z(), posNLR.X(),Form("%.1f^{#circ}",angleR));
    fGlobalNLGuideTextR -> SetTextAlign(31);
    fGlobalNLGuideTextR -> SetTextFont(132);
    fGlobalNLGuideTextR -> SetTextColor(kGray+1);



    fGlobalNLGuideLineL = new TLine(fZTarget,0, posNLL.Z(),posNLL.X());
    fGlobalNLGuideLineL -> SetLineColor(kGray);
    fGlobalNLGuideLineL -> SetLineStyle(1);

    auto angleL = atan2(abs(posNLL.X()),posNLL.Z()-fZTarget)*TMath::RadToDeg();
    fGlobalNLGuideTextL = new TLatex(posNLL.Z(), posNLL.X(),Form("%.1f^{#circ}",angleL));
    fGlobalNLGuideTextL -> SetTextAlign(33);
    fGlobalNLGuideTextL -> SetTextFont(132);
    fGlobalNLGuideTextL -> SetTextColor(kGray+1);
  }

  fGlobalNLGraphTop   -> Draw(opt);

  fGlobalNLGuideLineC -> Draw(opt);
  fGlobalNLGuideLineR -> Draw(opt);
  fGlobalNLGuideLineL -> Draw(opt);

  fGlobalNLGuideTextC -> Draw(opt);
  fGlobalNLGuideTextR -> Draw(opt);
  fGlobalNLGuideTextL -> Draw(opt);
}

void STNeuLAND::DrawGlobalNLGraphSide(Option_t *opt)
{
  if (fGlobalNLGraphSideOut == nullptr)
  {
    fGlobalNLGraphSideOut = new TGraph();
    for (auto point :
        {TVector3(-fHalfLengthBar,0,0),
        TVector3(-fHalfLengthBar,-fHalfLengthBar,0),
        TVector3(-fHalfLengthBar,-fHalfLengthBar,fdzNl),
        TVector3(-fHalfLengthBar,+fHalfLengthBar,fdzNl),
        TVector3(-fHalfLengthBar,+fHalfLengthBar,0),
        TVector3(-fHalfLengthBar,0,0)})
    {
      auto gpoint = GlobalPos(point);
      fGlobalNLGraphSideOut -> SetPoint(fGlobalNLGraphSideOut->GetN(), gpoint.Z(), gpoint.Y());
    }


    fGLobalNLGraphSideIn = new TGraph();
    fGLobalNLGraphSideIn -> SetLineStyle(2);
    for (auto point :
        {TVector3(+fHalfLengthBar,0,0),
        TVector3(+fHalfLengthBar,-fHalfLengthBar,0),
        TVector3(+fHalfLengthBar,-fHalfLengthBar,fdzNl),
        TVector3(+fHalfLengthBar,+fHalfLengthBar,fdzNl),
        TVector3(+fHalfLengthBar,+fHalfLengthBar,0),
        TVector3(+fHalfLengthBar,0,0)})
    {
      auto gpoint = GlobalPos(point);
      fGLobalNLGraphSideIn -> SetPoint(fGLobalNLGraphSideIn->GetN(), gpoint.Z(), gpoint.Y());
    }

    TVector3 posNLT = GlobalPos(TVector3(0,+fHalfLengthBar,0));
    TVector3 posNLB = GlobalPos(TVector3(0,-fHalfLengthBar,0));



    fGlobalNLGuideLineT = new TLine(fZTarget,0, posNLT.Z(),posNLT.Y());
    fGlobalNLGuideLineT -> SetLineColor(kGray);
    fGlobalNLGuideLineT -> SetLineStyle(1);

    auto angleT = atan2(abs(posNLT.Y()),posNLT.Z()-fZTarget)*TMath::RadToDeg();
    fGlobalNLGuideTextT = new TLatex(posNLT.Z(), posNLT.Y(),Form("%.1f^{#circ}",angleT));
    fGlobalNLGuideTextT -> SetTextAlign(11);
    fGlobalNLGuideTextT -> SetTextFont(132);
    fGlobalNLGuideTextT -> SetTextColor(kGray+1);



    fGlobalNLGuideLineB = new TLine(fZTarget,0, posNLB.Z(),posNLB.Y());
    fGlobalNLGuideLineB -> SetLineColor(kGray);
    fGlobalNLGuideLineB -> SetLineStyle(1);

    auto angleB = atan2(abs(posNLB.Y()),posNLB.Z()-fZTarget)*TMath::RadToDeg();
    fGlobalNLGuideTextB = new TLatex(posNLB.Z(), posNLB.Y(),Form("%.1f^{#circ}",angleB));
    fGlobalNLGuideTextB -> SetTextAlign(13);
    fGlobalNLGuideTextB -> SetTextFont(132);
    fGlobalNLGuideTextB -> SetTextColor(kGray+1);
  }




  fGlobalNLGraphSideOut -> Draw(opt);
  fGLobalNLGraphSideIn -> Draw(opt);

  fGlobalNLGuideLineT -> Draw(opt); 
  fGlobalNLGuideTextT -> Draw(opt);

  fGlobalNLGuideLineB -> Draw(opt);
  fGlobalNLGuideTextB -> Draw(opt);
}
