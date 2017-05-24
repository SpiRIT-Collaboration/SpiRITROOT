#include "STEventPreviewTask.hh"
#include "STPad.hh"
#include <iostream>
#include <fstream>
#include "TMath.h"

using namespace std;

ClassImp(STEventPreviewTask)

STEventPreviewTask::STEventPreviewTask()
: STEventPreviewTask(true, false, false)
{
}

STEventPreviewTask::STEventPreviewTask(Bool_t persistence, Bool_t identifyEvent, Bool_t calibrateTb)
: STRecoTask("Event Preivew Task", 1, persistence)
{
  fIdentifyEvent = identifyEvent;
  fCalibrateTb = calibrateTb;
}

STEventPreviewTask::~STEventPreviewTask()
{
}

InitStatus STEventPreviewTask::Init()
{
  if (STRecoTask::Init() == kERROR)
    return kERROR;

  fRawEventArray = (TClonesArray *) fRootManager -> GetObject("STRawEvent");
  if (fRawEventArray == nullptr) {
    LOG(ERROR) << "Cannot find STRawEvent array!" << FairLogger::endl;
    //return kERROR;
  }

  fEventHeader = new STEventHeader();
  fRootManager -> Register("STEventHeader", "SpiRIT", fEventHeader, fIsPersistence);

  if (fRecoHeader != nullptr) {
    fRecoHeader -> SetPar("pre_identifyEvent", fIdentifyEvent);
    fRootManager -> GetOutFile() -> cd();
    fRecoHeader -> Write("RecoHeader", TObject::kWriteDelete);
  }

  if (fCalibrateTb) {
    fPadMap = new STMap();
    fPadMap -> SetUAMap(fDigiPar -> GetUAMapFileName());
    fPadMap -> SetAGETMap(fDigiPar -> GetAGETMapFileName());

    for (auto coboIdx = 0; coboIdx < 12; ++coboIdx)
      fHistGG[coboIdx] = new TH1D(Form("histGG%d",coboIdx),"",NTB_GGCALIB,0,NTB_GGCALIB);
    fF1GG = new TF1("f1GG","[0]*TMath::Landau(x,[1],[2])+[3]",0,50);
  }

  return kSUCCESS;
}

void STEventPreviewTask::Exec(Option_t *opt)
{
  fEventHeader -> Clear();

  STRawEvent *rawEvent = (STRawEvent *) fRawEventArray -> At(0);
  fEventHeader -> SetEventID(rawEvent -> GetEventID());

  if (fIdentifyEvent)
    LayerTest(rawEvent);

  TString status = "Unidentified Event";
       if (fEventHeader -> IsEmptyEvent())        status = "Empty Event";
  else if (fEventHeader -> IsCollisionEvent())    status = "Collision Event";
  else if (fEventHeader -> IsActiveTargetEvent()) status = "Active Target Event";
  else if (fEventHeader -> IsOffTargetEvent())    status = "Off Target Event";
  else if (fEventHeader -> IsBeamEvent())         status = "Beam Event";
  else if (fEventHeader -> IsCosmicEvent())       status = "Cosmic Event";
  else if (fEventHeader -> IsBadEvent())          status = "Bad Event";

  LOG(INFO) << "Event " << fEventHeader -> GetEventID() << " : " << status << FairLogger::endl;


  if (fCalibrateTb) {
    Double_t tbRef0;
    Double_t tbOffsets[12];
    CalculateTbOffsets(rawEvent, tbOffsets);
    fEventHeader -> SetTbOffsets(tbOffsets);

    LOG(INFO) << Space() << "tb-calib.:";
    for (auto coboIdx = 0; coboIdx < 12; ++coboIdx)
      LOG(INFO) << Form(" %d:%.2f",coboIdx,tbOffsets[coboIdx]);
    LOG(INFO) << "." << FairLogger::endl;
  }
}

void STEventPreviewTask::LayerTest(STRawEvent *rawEvent)
{
  Double_t charge[12] = {0};

  Int_t numPads = rawEvent -> GetNumPads();
  for (Int_t iPad = 0; iPad < numPads; iPad++) 
  {
    STPad *pad = rawEvent -> GetPad(iPad);
    if (!(pad -> GetRow() < 56 && pad -> GetRow() > 35))
      continue;

    Double_t *adc = pad -> GetADC();
    for (Int_t iTb = 100; iTb < 130; iTb++)
      charge[pad -> GetLayer()/28] += adc[iTb];
  }

  if (charge[0] > charge[1] && charge[1] > charge[2] && charge[2] > charge[3])
    fEventHeader -> SetIsCollisionEvent();
  else
    fEventHeader -> SetIsBadEvent();
}

void STEventPreviewTask::CalculateTbOffsets(STRawEvent *rawEvent, Double_t *tbOffsets)
{
  Double_t tbRef[12] = {0};

  for (auto coboIdx = 0; coboIdx < 12; ++coboIdx) {
    SetAverageHistograms(rawEvent);
    auto f1 = FitGG(fHistGG[coboIdx]);
    tbRef[coboIdx] = f1 -> GetParameter(1);
  }

  for (auto coboIdx = 0; coboIdx < 12; ++coboIdx)
    tbOffsets[coboIdx] = tbRef[coboIdx] - tbRef[0];
}

TH1D *STEventPreviewTask::GetAverageHist(Int_t coboIdx)
{
  return fHistGG[coboIdx];
}

void STEventPreviewTask::SetAverageHistograms(STRawEvent *rawEvent)
{
  for (auto coboIdx = 0; coboIdx < 12; ++coboIdx)
    fHistGG[coboIdx] -> Reset();

  Int_t uaIdx, coboIdx, asadIdx, agetIdx, chIdx;
  for (auto row = 0; row < 108; row++) {
    for (auto layer = 0; layer < 112; layer++) {
      auto pad = rawEvent -> GetPad(row,layer);
      auto adc = pad -> GetADC();

      // test if gating grid noise is suppressed or shaded by signal
      Double_t max = 0;
      for (auto itb = 0; itb < NTB_GGCALIB; ++itb) {
        auto content = adc[itb];
        if (TMath::Abs(content) > max)
          max = TMath::Abs(content);
      }
      if (max > 300 || max < 100)
        continue;

      fPadMap -> GetMapData(row, layer, uaIdx, coboIdx, asadIdx, agetIdx, chIdx);

      auto histAverage = fHistGG[coboIdx];
      auto nContents = histAverage -> GetBinContent(0);
      for (auto itb = 0; itb < NTB_GGCALIB; ++itb) {
        auto contentAverage = histAverage -> GetBinContent(itb+1);
        contentAverage = (nContents*contentAverage + adc[itb]) / (nContents+1);
        histAverage -> SetBinContent(itb+1, contentAverage);
      }
      histAverage -> Fill(-1);
    }
  }
}

TF1 *STEventPreviewTask::FitGG(TH1D *hist)
{
  // find rough position and value of first(-) and second(+) peak
  Double_t tbStart = 0;
  Double_t tbFirstPeak = 0;
  Double_t tbSecondPeak = 0;
  Double_t tbThirdPeak = 0;

  Double_t adcFirstPeak = 0;
  Double_t adcSecondPeak = 0;

  for (auto itb = 0; itb < 100; ++itb) {
    Double_t tb = itb + 0.5;
    Double_t adc1 = hist -> GetBinContent(itb+1);
    Double_t adc2 = hist -> GetBinContent(itb+2);

    if (tbStart == 0) {
      Double_t diffp = TMath::Abs(adc2 - adc1);
      if (diffp > 20) {
        tbStart = itb;
        continue;
      } else
        continue;
    }
    else if (tbFirstPeak == 0) {
      Double_t diff = adc2 - adc1;
      if (diff > 0) {
        tbFirstPeak = itb;
        adcFirstPeak = adc1;
        continue;
      } else
        continue;
    }
    else if (tbSecondPeak == 0) {
      Double_t diff = adc2 - adc1;
      if (diff < 0) {
        tbSecondPeak = itb;
        adcSecondPeak = adc1;
        continue;
      } else
        continue;
    }
    else if (tbThirdPeak == 0) {
      Double_t diff = adc2 - adc1;
      if (diff > 0) {
        tbThirdPeak = itb;
        break;
      } else
        continue;
    }
  }

  fF1GG -> SetParameters(adcSecondPeak*10,tbSecondPeak,(tbSecondPeak-tbFirstPeak)/2.,adcFirstPeak);
  fF1GG -> SetRange(tbFirstPeak+1,tbThirdPeak-3);
  hist -> Fit(fF1GG,"Q0R");

  return fF1GG;
}

Double_t STEventPreviewTask::FindSecondPeak(TH1D *hist)
{
  Double_t tbStart = 0;
  Double_t tbFirstPeak = 0;
  Double_t tbSecondPeak = 0;

  Double_t adcFirstPeak = 0;
  Double_t adcSecondPeak = 0;

  for (auto itb = 0; itb < 100; ++itb) {
    Double_t tb = itb + 0.5;
    Double_t adc1 = hist -> GetBinContent(itb+1);
    Double_t adc2 = hist -> GetBinContent(itb+2);

    if (tbStart == 0) {
      Double_t diffp = TMath::Abs(adc2 - adc1);
      if (diffp > 20) {
        tbStart = itb;
        continue;
      } else
        continue;
    }
    else if (tbFirstPeak == 0) {
      Double_t diff = adc2 - adc1;
      if (diff > 0) {
        tbFirstPeak = itb;
        adcFirstPeak = adc1;
        continue;
      } else
        continue;
    }
    else if (tbSecondPeak == 0) {
      Double_t diff = adc2 - adc1;
      if (diff < 0) {
        //adcSecondPeak = adc1;
        Double_t adc0 = hist -> GetBinContent(itb);
        tbSecondPeak = (itb*adc0+(itb+1)*adc1+(itb+2)*adc2)/(adc0+adc1+adc2);
        break;
      } else
        continue;
    }
  }

  return tbSecondPeak-0.5;
}

void STEventPreviewTask::IdentifyEvent(Bool_t val) { fIdentifyEvent = val; }
void STEventPreviewTask::CalibrateTb(Bool_t val) { fCalibrateTb = val; }
