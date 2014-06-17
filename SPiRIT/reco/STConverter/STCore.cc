// =================================================
//  STCore Class
// 
//  Description:
//    Process CoBoFrame data into STRawEvent data
// 
//  Genie Jhang ( geniejhang@majimak.com )
//  2013. 09. 25
// =================================================

#include <iostream>
#include <fstream>

#include "STCore.hh"

#include "STMap.hh"
#include "STPedestal.hh"
#include "STRawEvent.hh"

#include "GETDecoder.hh"
#include "GETFrame.hh"

ClassImp(STCore);

STCore::STCore()
{
  Initialize();
}

STCore::STCore(Char_t *filename)
{
  Initialize();
  AddGraw(filename);
}

STCore::STCore(Char_t *filename, Int_t numTbs)
{
  Initialize();
  AddGraw(filename);
  SetNumTbs(numTbs);
}

STCore::~STCore()
{
  delete fDecoderPtr;
  
  delete fPedestalPtr;
  delete fMapPtr;
}

void STCore::Initialize()
{
  fRawEventPtr = NULL;

  fMapPtr = new STMap();
  fPedestalPtr = new STPedestal();

  fDecoderPtr = new GETDecoder();
//  fDecoderPtr -> SetDebugMode(1);

  fIsGraw = 0;
  fIsPedestalData = 0;
  fIsInternalPedestal = 0;

  fStartTb = 0;
  fNumTbs = 0;

  fPrevEventNo = -1;
  fCurrEventNo = -1;
  fCurrFrameNo = 0;
}

void STCore::AddGraw(Char_t *filename)
{
  fDecoderPtr -> AddGraw(filename);
  fIsGraw = fDecoderPtr -> SetData(0);
}

void STCore::SetNumTbs(Int_t value)
{
  fDecoderPtr -> SetNumTbs(value);
}

void STCore::SetInternalPedestal(Int_t startTb, Int_t numTbs)
{
  fIsInternalPedestal = 1;
  fIsPedestalData = 0;

  fStartTb = startTb;
  fNumTbs = numTbs;
}

void STCore::SetPedestalData(Char_t *filename)
{
  fIsPedestalData = fPedestalPtr -> SetPedestalData(filename);

  if (fIsPedestalData)
    fIsInternalPedestal = 0;
  else
    std::cout << "== Pedestal data is not set! Check it exists!" << std::endl;
}

STRawEvent *STCore::GetRawEvent(Int_t eventID)
{
  if (!fIsGraw) {
    std::cout << "== Graw file is not set!" << std::endl;

    return NULL;
  }

  if (!fIsPedestalData && !fIsInternalPedestal) {
    std::cout << "== Pedestal data file is not set!" << std::endl;
  }

  fPrevEventNo = eventID;

  if (fRawEventPtr != NULL)
    delete fRawEventPtr;

  fRawEventPtr = new STRawEvent();

  GETFrame *frame = NULL;
  while ((frame = fDecoderPtr -> GetFrame(fCurrFrameNo))) {
    if (fPrevEventNo == -1)
      fPrevEventNo = frame -> GetEventID();

    fCurrEventNo = frame -> GetEventID();

    if (fCurrEventNo == fPrevEventNo + 1) {
      fPrevEventNo = fCurrEventNo;
      return fRawEventPtr;
    } else if (fCurrEventNo > fPrevEventNo + 1) {
      fCurrFrameNo = 0;
      continue;
    } else if (fCurrEventNo < fPrevEventNo) {
      fCurrFrameNo++;
      continue;
    }

    fRawEventPtr -> SetEventID(fCurrEventNo);

    Int_t coboID = frame -> GetCoboID();
    Int_t asadID = frame -> GetAsadID();

    if (fIsInternalPedestal)
      frame -> CalcPedestal(fStartTb, fNumTbs);

    for (Int_t iAget = 0; iAget < 4; iAget++) {
      for (Int_t iCh = 0; iCh < 68; iCh++) {
        Int_t row, layer;
        fMapPtr -> GetRowNLayer(coboID, asadID, iAget, iCh, row, layer);

        if (row == -2 || layer == -2)
          continue;

        STPad *pad = new STPad(row, layer);
        Int_t *rawadc = frame -> GetRawADC(iAget, iCh);
        for (Int_t iTb = 0; iTb < fDecoderPtr -> GetNumTbs(); iTb++)
          pad -> SetRawADC(iTb, rawadc[iTb]);

        if (fIsInternalPedestal) {
          Double_t *adc = frame -> GetADC(iAget, iCh);
          for (Int_t iTb = 0; iTb < fDecoderPtr -> GetNumTbs(); iTb++)
            pad -> SetADC(iTb, adc[iTb]);

          pad -> SetMaxADCIdx(frame -> GetMaxADCIdx(iAget, iCh));
        } else if (fIsPedestalData) {
          // Code using pedestal data file will be set here.
        }

        fRawEventPtr -> SetPad(pad);
      }
    }

    fCurrFrameNo++;
  }

  return NULL;
}

Int_t STCore::GetNumTbs()
{
  return fDecoderPtr -> GetNumTbs();
}
