#include "STChannelBar.hh"

#include "TMath.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TPaveStats.h"
#include "TLine.h"
#include "TLatex.h"

#include <iostream>
using namespace std;

ClassImp(STChannelBar);

STChannelBar::STChannelBar()
{
  Clear();
}

STChannelBar::STChannelBar(
    TString name,
    Int_t id, Int_t layer, Int_t row, Bool_t xy, TVector3 center, Int_t nbins,
    Double_t barl, Double_t attl, Double_t tdcy, Double_t tris, Double_t terr, Double_t effc)
{
  Clear();
  SetBar(name, id, layer, row, xy, center, nbins, barl, attl, tdcy, tris, terr, effc);
}

void STChannelBar::SetBar(
    TString name,
    Int_t id, Int_t layer, Int_t row, Bool_t xy, TVector3 center, Int_t nbins,
    Double_t barl, Double_t attl, Double_t tdcy, Double_t tris, Double_t terr, Double_t effc)
{
  fChannelID = id;
  fLayer = layer;
  fRow = row;

  fIsAlongXNotY = xy;
  fBarCenter = center;
  fNumTDCBins = nbins;

  fBarLength = barl;
  fAttenuationLength = attl;
  fDecayTime = tdcy;
  fRiseTime = tris;
  fTimeErrorSigma = terr;
  fEffc = effc;

  Init(name);
}

void STChannelBar::Init(TString name)
{
  if (fChannelL == nullptr)
  {
    TString namet = Form("Ch%d_L%d_R%d",fChannelID,fLayer,fRow);
    if (name.IsNull()) name = namet;
    else name = name + "_" + namet;

    fChannelL = new TH1D(name+"_L",";TDC;ADC_{L}",fNumTDCBins,0,fNumTDCBins);
    fChannelR = new TH1D(name+"_R",";TDC;ADC_{R}",fNumTDCBins,0,fNumTDCBins);

    for (auto h : {fChannelL, fChannelR})
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
  }

  if (fHistPulse == nullptr)
  {
    fHistPulse = STNeuLAND::GetNeuLAND() -> fHistPulse;
    fPulseTDCRange = STNeuLAND::GetNeuLAND() -> fPulseTDCRange;
    /*
    auto pulseTimeRange1 = 2*(-fRiseTime-fTimeErrorSigma)*TMath::Log(2);
    auto pulseTimeRange2 = 8*(fDecayTime+fTimeErrorSigma)*TMath::Log(2);
    fPulseTDCRange = Int_t(ConvertTimeToTDC(pulseTimeRange2));

    fFuncPulse = new TF1("NLPulse", this, &STChannelBar::PulseWithError, pulseTimeRange1, pulseTimeRange2, 4, "STChannelBar", "PulseWithError");
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
    */
  }
}

void STChannelBar::Clear(Option_t *)
{
  fChannelID = -1;
  fLayer = -1;
  fRow = -1;

  fIsAlongXNotY = true;
  fBarCenter = TVector3(-999,-999,-999);
  fNumTDCBins = 1;

  fBarLength = -1;
  fAttenuationLength = DBL_MAX;
  fDecayTime = 1;
  fRiseTime = 0.1;
  fTimeErrorSigma = 0;
  fEffc = 300.;

  fFlagFindHit = false;
  fThreshold = 0;

  fADC[0] = -1;
  fADC[1] = -1;

  fQDC[0] = -1;
  fQDC[1] = -1;

  fTDC[0] = -1;
  fTDC[1] = -1;

  if (fChannelL != nullptr) {
    fChannelR -> Reset(0);
    fChannelL -> Reset(0);
  }
}

void STChannelBar::Print(Option_t *option) const
{
  cout << "[STChannelBar]" << endl;

  cout << "  [Bar infomation]"
    << " ch=" << fChannelID
    << " ly=" << fLayer
    << " rw=" << fRow
    << " <-" << (fIsAlongXNotY ? "x" : "y") << "->"
    << " pos=("  << fBarCenter.X() << ","<< fBarCenter.Y() << ","<< fBarCenter.Z()<<")"
    << " nbins=" << fNumTDCBins
    << endl;

  cout << "  [Bar properties]"
    << " barl=" << fBarLength 
    << " attl=" << fAttenuationLength 
    << " tdcy=" << fDecayTime 
    << " tris=" << fRiseTime 
    << " terr=" << fTimeErrorSigma 
    << " effc=" << fEffc 
    << endl;

  cout << "  [Reconstructed1]"
    << " threshold=" << fThreshold
    << " adc(L/R)=" << fADC[0] << "/" << fADC[1]
    << " qdc(L/R)=" << fQDC[0] << "/" << fQDC[1]
    << " tdc(L/R)=" << fTDC[0] << "/" << fTDC[1]
    << endl;

  cout << "  [Reconstructed2]"
    << " charge(Q/A)=" << GetChargeQ() << "/" << GetChargeA()
    << " adc(t/x)=" << GetTDCByA() << "/" << ConvertTDCToPos(GetTDCByA())
    << " qdc(t/x)=" << GetTDCByQ() << "/" << ConvertTDCToPos(GetTDCByQ())
    << " tdc(t/x)=" << GetTDCByT() << "/" << ConvertTDCToPos(GetTDCByT())
    << endl;

  auto post = GetTDCHitPosition();
  auto posa = GetADCHitPosition();
  auto posq = GetQDCHitPosition();

  cout << "  [Final position]" << endl;
  cout << "  post=(" << post.X() << ", " << post.Y() << ", " << post.Z() << ")" << endl;
  cout << "  posa=(" << posa.X() << ", " << posa.Y() << ", " << posa.Z() << ")" << endl;
  cout << "  posq=(" << posq.X() << ", " << posq.Y() << ", " << posq.Z() << ")" << endl;
}

void STChannelBar::Draw(Option_t *option)
{
  gStyle -> SetOptStat(0);

  TCanvas *cvs;
  if (gPad == nullptr) {
    TString name = Form("Ch%d_L%d_R%d",fChannelID,fLayer,fRow);
    cvs = new TCanvas(name,name,550,750);
  }
  else
    cvs = (TCanvas *) gPad;

  cvs -> Divide(1,3,0.001,0.001);
  cvs -> SetMargin(0.02,0.02,0.02,0.02);

  Double_t maxl = fChannelL -> GetBinContent(fChannelL -> GetMaximumBin());
  Double_t maxr = fChannelR -> GetBinContent(fChannelR -> GetMaximumBin());
  Double_t hist_max = (maxl > maxr) ? maxl : maxr;

  for (auto iLR : {0,1}) {
    auto cvsi = cvs -> cd(iLR+1);
    TH1D *hist;
    TString name;
    if (iLR==0) { hist = fChannelL; name = "Channel_Left"; }
    else { hist = fChannelR; name = "Channel_Right"; }
    hist -> SetMaximum(hist_max*1.1);
    cvsi -> SetName(name);
    cvsi -> SetTitle(name);
    cvsi -> SetMargin(0.17,0.055,0.18,0.05);
    hist -> Draw("hist");
    if (fFlagFindHit)
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

    TString namehist = fChannelL -> GetName();
    namehist.Replace(namehist.Sizeof()-3,2,"_Bar");

    fHistSpace = new TH1D(namehist,TString(";")+namexy,fNumTDCBins,ConvertTDCToPos(0),ConvertTDCToPos(fNumTDCBins));

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

  auto pos_by_tdc = ConvertTDCToPos(GetTDCByT());

  auto line1 = new TLine(pos_by_tdc,0,pos_by_tdc,1);
  line1 -> SetLineColor(kBlue);
  line1 -> Draw("samel");

  auto tt1 = new TLatex(pos_by_tdc,0.15,Form(" %s_{TDC}=%.2f ",namexy.Data(),pos_by_tdc));
  tt1 -> SetTextColor(kBlue);
  tt1 -> SetTextFont(132);
  tt1 -> SetTextSize(0.1);
  tt1 -> SetTextAlign(12);
  if (pos_by_tdc > 0)
    tt1 -> SetTextAlign(32);
  tt1 -> Draw();

  auto pos_by_qdc = ConvertTDCToPos(GetTDCByQ());

  auto line2 = new TLine(pos_by_qdc,0,pos_by_qdc,1);
  line2 -> SetLineColor(kRed);
  line2 -> Draw("samel");

  auto tt2 = new TLatex(pos_by_qdc,0.85,Form(" %s_{QDC}=%.2f ",namexy.Data(),pos_by_qdc));
  tt2 -> SetTextColor(kRed);
  tt2 -> SetTextFont(132);
  tt2 -> SetTextSize(0.1);
  tt2 -> SetTextAlign(12);
  if (pos_by_qdc > 0)
    tt2 -> SetTextAlign(32);
  tt2 -> Draw();

  auto pos_by_adc = ConvertTDCToPos(GetTDCByA());

  auto line3 = new TLine(pos_by_adc,0,pos_by_adc,1);
  line3 -> SetLineColor(kBlack);
  line3 -> Draw("samel");

  auto tt3 = new TLatex(pos_by_adc,0.5,Form(" %s_{ADC}=%.2f ",namexy.Data(),pos_by_adc));
  tt3 -> SetTextColor(kBlack);
  tt3 -> SetTextFont(132);
  tt3 -> SetTextSize(0.1);
  tt3 -> SetTextAlign(12);
  if (pos_by_adc > 0)
    tt3 -> SetTextAlign(32);
  tt3 -> Draw();

}

/// convolution of pulse and gaussian (error)
Double_t STChannelBar::PulseWithError(Double_t *xx, Double_t *pp)
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

void STChannelBar::Fill(TVector3 pos, Double_t adc)
{
  Double_t x0;

  if (fIsAlongXNotY) x0 = pos.X() - fBarCenter.X();
  else               x0 = pos.Y() - fBarCenter.Y();

  if (x0 < -fBarLength/2. || x0 >= fBarLength/2.)
    return;

  auto dt_chbin = 1.;
  auto attenuationTDC = ConvertLengthToTDC(fAttenuationLength);

  //

  {
    auto t0L = ConvertPosToTDC(x0);
    auto bint0L = fChannelL -> FindBin(t0L);
    auto t0_bincenterL = fChannelL -> GetXaxis() -> GetBinCenter(bint0L);
    auto dt0L = t0_bincenterL - t0L;

    auto adcL =  adc * TMath::Exp(-t0_bincenterL / attenuationTDC);

    for (auto binIdx=0; binIdx<100; ++binIdx)
    {
      auto t_pulse = binIdx * dt_chbin + dt0L;
      if (t_pulse > fPulseTDCRange)
        break;

      auto t_bincenter = fChannelL -> GetXaxis() -> GetBinCenter(binIdx + bint0L);
      if (t_bincenter > fNumTDCBins)
        break;

      auto charge = fHistPulse -> Interpolate(t_pulse);
      auto adc_bin = adcL * charge;
      fChannelL -> Fill(t_bincenter, adc_bin);
    }
  }

  {
    auto t0R =  fNumTDCBins - ConvertPosToTDC(x0);
    auto bint0R = fChannelR -> FindBin(t0R);
    auto t0_bincenterR = fChannelR -> GetXaxis() -> GetBinCenter(bint0R);
    auto dt0R = t0_bincenterR - t0R;

    auto adcR = adc * TMath::Exp(-t0_bincenterR / attenuationTDC);

    for (auto binIdx=0; binIdx<100; ++binIdx)
    {
      auto t_pulse = binIdx * dt_chbin - dt0R;
      if (t_pulse > fPulseTDCRange)
        return;

      auto t_bincenter = fChannelR -> GetXaxis() -> GetBinCenter(binIdx + bint0R);
      if (t_bincenter > fNumTDCBins)
        break;

      auto charge = fHistPulse -> Interpolate(t_pulse);
      auto adc_bin = adcR * charge;
      fChannelR -> Fill(t_bincenter, adc_bin);
    }
  }
}

TVector3 STChannelBar::FindHit(Double_t threshold, Bool_t forceFindHit)
{
  if (forceFindHit || !fFlagFindHit)
  {
    fFlagFindHit = true;
    fThreshold = threshold;

    for (auto iLR : {0,1})
    {
      TH1D *hist;
      if (iLR==0) hist = fChannelL;
      else hist = fChannelR;
      Double_t qdc = 0;
      Double_t adc = -1;
      Double_t tdcbin = -1;

      for (auto bin=1; bin<=fNumTDCBins; ++bin)
      {
        auto value = hist -> GetBinContent(bin);
        if ( value > fThreshold )
        {
          if (tdcbin < 0) tdcbin = bin;

          qdc += value - fThreshold;
        }

        if (value > adc)
          adc = value;
      }

      if (qdc == 0) qdc = 0;
      if (adc < 0) adc = 0;

      Double_t tdc = 0.;
      if (tdcbin < 1)
        tdc = 0;
      else if (tdcbin == 1)
        tdc = hist -> GetXaxis() -> GetBinCenter(tdcbin);
      else {
        auto x1 = hist -> GetXaxis() -> GetBinCenter(tdcbin-1);
        auto x2 = hist -> GetXaxis() -> GetBinCenter(tdcbin);

        auto y1 = hist -> GetBinContent(tdcbin-1);
        auto y2 = hist -> GetBinContent(tdcbin);

        tdc = (fThreshold - y1) * (x2 - x1) / (y2 - y1) + x1;
      }

      fTDC[iLR] = tdc;
      fQDC[iLR] = qdc;
      fADC[iLR] = adc;
    }
  }

  return GetTDCHitPosition();
}
