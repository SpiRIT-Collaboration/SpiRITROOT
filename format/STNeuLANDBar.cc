#include "STNeuLANDBar.hh"

#include "TMath.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TPaveStats.h"
#include "TLine.h"
#include "TLatex.h"

#include <iostream>
using namespace std;

ClassImp(STNeuLANDBar);

STNeuLANDBar::STNeuLANDBar()
{
  fHistPulse = STNeuLAND::GetNeuLAND() -> fHistPulse;
  fPulseTDCRange = STNeuLAND::GetNeuLAND() -> fPulseTDCRange;

  Clear();
}

void STNeuLANDBar::Clear(Option_t *opt)
{
  SetBarID(-1,-1,-1,-1,-1);
  SetBar(0, TVector3(-999.,-999.,-999.), 25., .125, 2500., 140., 1250.);

  fFoundHit = false;
  fThreshold = -1;

  fLeft.Reset(0);
  fRight.Reset(0);

  fMCIDs.clear();

  fFired[0] = 0;
  fADC[0] = -1;
  fQDC[0] = -1;
  fTDC[0] = -1;

  fFired[1] = 0;
  fADC[1] = -1;
  fQDC[1] = -1;
  fTDC[1] = -1;
}

void STNeuLANDBar::Print(Option_t *) const
{
  cout << "  [Bar infomation]"
    << " ch(l/r)=" << fBarID << "(" << fLChannelID << "/" << fRChannelID << ")"
    << " ly=" << fLayer
    << " rw=" << fRow
    << " <-" << (fIsAlongXNotY ? "x" : "y") << "->"
    << " pos=("  << fBarCenter.X() << ","<< fBarCenter.Y() << ","<< fBarCenter.Z()<<")"
    << endl;

  cout << "  [Reconstructed1]"
    << " threshold=" << fThreshold
    << " fired(L/R)=" << fFired[0] << "/" << fFired[1]
    << " adc(L/R)=" << fADC[0] << "/" << fADC[1]
    << " qdc(L/R)=" << fQDC[0] << "/" << fQDC[1]
    << " tdc(L/R)=" << fTDC[0] << "/" << fTDC[1]
    << endl;

  /*
  cout << "  [Reconstructed2]"
    << " charge(Q/A)=" << GetChargeQ() << "/" << GetChargeA()
    << " adc(t/x)=" << GetTDCByA() << "/" << ConvertTDCLToLocalX(GetTDCByA())
    << " qdc(t/x)=" << GetTDCByQ() << "/" << ConvertTDCLToLocalX(GetTDCByQ())
    << " tdc(t/x)=" << GetTDCByT() << "/" << ConvertTDCLToLocalX(GetTDCByT())
    << endl;

  auto post = GetTDCHitPosition();
  auto posa = GetADCHitPosition();
  auto posq = GetQDCHitPosition();

  cout << "  [Final position]" << endl;
  cout << "  post=(" << post.X() << ", " << post.Y() << ", " << post.Z() << ")" << endl;
  cout << "  posa=(" << posa.X() << ", " << posa.Y() << ", " << posa.Z() << ")" << endl;
  cout << "  posq=(" << posq.X() << ", " << posq.Y() << ", " << posq.Z() << ")" << endl;
  */
}

void STNeuLANDBar::Draw(Option_t *option)
{
  gStyle -> SetOptStat(0);

  TString nameid = Form("Bar%d_Layer%d_Row%d",fBarID,fLayer,fRow);
  TString namehead = TString(option);
  TString namefull = namehead + "  " + nameid + (fIsAlongXNotY ? "  <-x->" : "  <-y->");

  TCanvas *cvs;
  if (gPad == nullptr) {
    cvs = new TCanvas(nameid,nameid,550,750);
  }
  else
    cvs = (TCanvas *) gPad;

  cvs -> Divide(1,3,0.001,0.001);
  cvs -> SetMargin(0.02,0.02,0.02,0.02);

  if (fHistLeft == nullptr)
  {
    TString namet = Form("Ch%d_L%d_R%d",fBarID,fLayer,fRow);
    if (namehead.IsNull()) namehead = namet;
    else namehead = namehead + "_" + namet;

    if (fIsAlongXNotY) {
      fHistLeft = new TH1D(namehead+"_L",";TDC;ADC Left",fNumTDCBins,0,fNumTDCBins);
      fHistRight = new TH1D(namehead+"_R",";TDC;ADC Right",fNumTDCBins,0,fNumTDCBins);
    }
    else {
      fHistLeft = new TH1D(namehead+"_B",";TDC;ADC Below",fNumTDCBins,0,fNumTDCBins);
      fHistRight = new TH1D(namehead+"_A",";TDC;ADC Top",fNumTDCBins,0,fNumTDCBins);
    }

    for (auto h : {fHistLeft, fHistRight})
    {
      for (auto axis : {h -> GetXaxis(), h -> GetYaxis()})
      {
        axis -> CenterTitle();
        axis -> SetTitleOffset(1.10);
        axis -> SetTitleSize(0.08);
        axis -> SetLabelSize(0.07);
        axis -> SetTitleFont(132);
        axis -> SetLabelFont(132);
        axis -> SetNdivisions(506);
      }
    }

    for (auto idx=0; idx<fNumTDCBins; ++idx)
    {
      fHistLeft  -> SetBinContent(idx+1, fLeft.At(idx));
      fHistRight -> SetBinContent(idx+1, fRight.At(idx));
    }
  }

  Double_t maxl = fHistLeft -> GetBinContent(fHistLeft -> GetMaximumBin());
  Double_t maxr = fHistRight -> GetBinContent(fHistRight -> GetMaximumBin());
  Double_t hist_max = (maxl > maxr) ? maxl : maxr;

  for (auto iLR : {0,1}) {
    auto cvsi = cvs -> cd(iLR+1);
    TH1D *hist;
    TString nameLR;
    if (iLR==0) { hist = fHistLeft; nameLR = "Channel_Left"; }
    else { hist = fHistRight; nameLR = "Channel_Right"; }
    hist -> SetMaximum(hist_max*1.1);
    cvsi -> SetName(nameLR);
    cvsi -> SetTitle(nameLR);
    cvsi -> SetMargin(0.17,0.055,0.18,0.05);

    if (fFired[iLR])
      hist -> SetLineColor(kRed+1);
    else
      hist -> SetLineColor(kBlue+1);

    hist -> Draw("hist");
    if (fFoundHit)
    {
      auto line1 = new TLine(0,fThreshold,fNumTDCBins,fThreshold);
      line1 -> SetLineStyle(2);
      line1 -> Draw();

      auto line2 = new TLine(fTDC[iLR],0,fTDC[iLR],fThreshold);
      line2 -> SetLineStyle(2);
      line2 -> Draw();

      auto text = new TLatex(fTDC[iLR],fThreshold+0.05*hist_max,Form(" (t=%.2f, q=%.2f, a=%.2f) ",fTDC[iLR],fQDC[iLR],fADC[iLR]));
      text -> SetTextAlign(11);
      if (fTDC[iLR] > fNumTDCBins/2.)
        text -> SetTextAlign(31);
      text -> SetTextFont(132);
      text -> SetTextSize(0.1);
      text -> Draw();
    }
  }

  auto cvsi = cvs -> cd(3);
  cvsi -> SetMargin(0.17,0.055,0.18,0.05);
  TString namexy;
  if (fHistSpace == nullptr)
  {
    if (fIsAlongXNotY) namexy = "#it{x}";
    else               namexy = "#it{x}";

    TString namehist = fHistLeft -> GetName();
    namehist.Replace(namehist.Sizeof()-3,2,"_Bar");

    fHistSpace = new TH1D(namehist,TString(";")+namexy,fNumTDCBins,TDCLToLocalX(0),TDCLToLocalX(fNumTDCBins));

    for (auto axis : {fHistSpace -> GetXaxis(), fHistSpace -> GetYaxis()})
    {
      axis -> CenterTitle();
      axis -> SetTitleOffset(1.10);
      axis -> SetTitleSize(0.08);
      axis -> SetLabelSize(0.07);
      axis -> SetTitleFont(132);
      axis -> SetLabelFont(132);
      axis -> SetNdivisions(506);
    }
  }

  fHistSpace -> SetMinimum(0);
  fHistSpace -> SetMaximum(1);
  fHistSpace -> Draw();

  auto lmax = .75;

  auto pos_by_tdc = TDCLToLocalX(GetTDCByT());

  auto line1 = new TLine(pos_by_tdc,0,pos_by_tdc,lmax);
  line1 -> SetLineColor(kBlue);
  line1 -> Draw("samel");

  auto tt1 = new TLatex(pos_by_tdc,.125,Form(" %s_{TDC}=%.2f ",namexy.Data(),pos_by_tdc));
  tt1 -> SetTextColor(kBlue);
  tt1 -> SetTextFont(132);
  tt1 -> SetTextSize(0.1);
  tt1 -> SetTextAlign(12);
  if (pos_by_tdc > 0)
    tt1 -> SetTextAlign(32);
  tt1 -> Draw();

  auto pos_by_qdc = TDCLToLocalX(GetTDCByQ());

  auto line2 = new TLine(pos_by_qdc,0,pos_by_qdc,lmax);
  line2 -> SetLineColor(kRed);
  line2 -> Draw("samel");

  auto tt2 = new TLatex(pos_by_qdc,.375,Form(" %s_{QDC}=%.2f ",namexy.Data(),pos_by_qdc));
  tt2 -> SetTextColor(kRed);
  tt2 -> SetTextFont(132);
  tt2 -> SetTextSize(0.1);
  tt2 -> SetTextAlign(12);
  if (pos_by_qdc > 0)
    tt2 -> SetTextAlign(32);
  tt2 -> Draw();

  auto pos_by_adc = TDCLToLocalX(GetTDCByA());

  auto line3 = new TLine(pos_by_adc,0,pos_by_adc,lmax);
  line3 -> SetLineColor(kBlack);
  line3 -> Draw("samel");

  auto tt3 = new TLatex(pos_by_adc,.625,Form(" %s_{ADC}=%.2f ",namexy.Data(),pos_by_adc));
  tt3 -> SetTextColor(kBlack);
  tt3 -> SetTextFont(132);
  tt3 -> SetTextSize(0.1);
  tt3 -> SetTextAlign(12);
  if (pos_by_adc > 0)
    tt3 -> SetTextAlign(32);
  tt3 -> Draw();

  auto ttitle = new TLatex((TDCLToLocalX(0) + TDCLToLocalX(fNumTDCBins))/2.,.875,namefull);
  ttitle -> SetTextColor(kBlack);
  ttitle -> SetTextFont(132);
  ttitle -> SetTextSize(0.1);
  ttitle -> SetTextAlign(22);
  ttitle -> Draw();
}

void STNeuLANDBar::Fill(TVector3 localpos, Double_t adc, Int_t mcid)
{
  for (auto isLNotR : {true,false})
  {
    auto tdc0 = LocalXToTDC(PositionToLocalX(localpos),isLNotR); // tdc from left channel
    if (tdc0 > 0)
    {
      auto dtdc = floor(tdc0) + .5 - tdc0 ; // displacement from tdc bin center to tdc0  

      for (auto binIdx=0; binIdx<100; ++binIdx) // pulse bin index loop
      {
        auto tdc_pulse = binIdx + dtdc; // x-axis value of the pulse function
        if (tdc_pulse > fPulseTDCRange) // if larger than pulse range then stop filling
          break;
        auto pulse_value = fHistPulse -> Interpolate(tdc_pulse); // get corresponding pulse value of current bin

        Int_t tdc_current = floor(tdc0) + binIdx; // tdc to be filled
        if (tdc_current >= fNumTDCBins)
          break;
        auto adc_attenuation = adc * TMath::Exp(-tdc0 / LengthToTDC(fAttenuationLength)); // final adc input after attenuation effect
        auto adc_bin_final = adc_attenuation * pulse_value; // final value of adc to be filled in current bin

        FillToBin(isLNotR,tdc_current,adc_bin_final);
      }
    }
  }

  if (mcid >= 0)
    fMCIDs.push_back(mcid);
}

bool STNeuLANDBar::FindHit(Double_t threshold, Bool_t forceFindHit)
{
  if (forceFindHit || !fFoundHit)
  {
    fFoundHit = true;
    fThreshold = threshold;

    for (auto iLR : {0,1})
    {
      fFired[iLR] = 0;
      fADC[iLR] = -1;
      fQDC[iLR] = -1;
      fTDC[iLR] = -1;

      TArrayD buffer;
      if (iLR==0) buffer = fLeft;
      else buffer = fRight;
      Double_t qdc = 0;
      Double_t adc = -1;
      Double_t tdcbin = -1;

      for (auto bin=0; bin<fNumTDCBins; ++bin)
      {
        auto value = buffer.At(bin);
        if ( value > fThreshold )
        {
          if (tdcbin < 0) tdcbin = bin;

          qdc += value;// - fThreshold;
        }

        if (value > adc)
          adc = value;
      }

      if (qdc == 0) qdc = 0;
      if (adc < 0) adc = 0;

      Double_t tdc = 0.;
      fFired[iLR] = true;
      if (tdcbin < 1) {
        tdc = 0;
        fFired[iLR] = false;
      }
      else if (tdcbin == 1)
        tdc = tdcbin + .5;
      else {
        auto x1 = tdcbin - 1;
        auto x2 = tdcbin;

        auto y1 = buffer.At(tdcbin-1);
        auto y2 = buffer.At(tdcbin);

        tdc = (fThreshold - y1) * (x2 - x1) / (y2 - y1) + x1;
      }

      fTDC[iLR] = tdc;
      fQDC[iLR] = qdc;
      fADC[iLR] = adc;
    }
  }

  if (fTDC[0] < 0 || fTDC[1] < 0)
    return false;

  return true;
}

void STNeuLANDBar::FillToBin(Bool_t isLNotR, Int_t bin, Double_t value)
{
  if (isLNotR)  fLeft.AddAt(  fLeft.At(bin) + value, bin );
  else         fRight.AddAt( fRight.At(bin) + value, bin );
}
