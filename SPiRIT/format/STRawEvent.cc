// =================================================
//  STRawEvent Class                          
//                                                  
//  Description:                                    
//    Container for a raw event data
//                                                  
//  Genie Jhang ( geniejhang@majimak.com )          
//  2013. 08. 16                                    
// =================================================

#include <iostream>
#include <iomanip>

#include "STRawEvent.hh"
#include "STPad.hh"

ClassImp(STRawEvent);

STRawEvent::STRawEvent()
:TNamed("STRawEvent", "Raw event container")
{
  fEventID = -1;
  fPadArray.reserve(108*112);
}

STRawEvent::STRawEvent(STRawEvent *object)
:TNamed("STRawEvent", "Raw event container")
{
  fEventID = object -> GetEventID();
  fPadArray = object -> GetPads();
}

STRawEvent::~STRawEvent()
{
}

void STRawEvent::PrintPads()
{
  for (Int_t iPad = 0; iPad < fPadArray.size(); iPad++) {
    std::cout << "Pad: " << std::setw(5) << iPad;
    std::cout << " (" << std::setw(3) << fPadArray[iPad].GetRow();
    std::cout << ", " << std::setw(3) << fPadArray[iPad].GetLayer() << ")";
    std::cout << std::endl;
  }
}

// setters
void STRawEvent::SetEventID(Int_t evtid)
{
  fEventID = evtid;
}

void STRawEvent::SetPad(STPad *pad)
{
  fPadArray.push_back(*pad);
}

// getters
Int_t STRawEvent::GetEventID()
{
  return fEventID;
}

Int_t STRawEvent::GetNumPads()
{
  return fPadArray.size();
}

std::vector<STPad> STRawEvent::GetPads()
{
  return fPadArray;
}

STPad *STRawEvent::GetPad(Int_t padNo)
{
  return (padNo < GetNumPads() ? &fPadArray[padNo] : NULL);
}

STPad *STRawEvent::GetPad(Int_t row, Int_t layer)
{
  for (Int_t iPad = 0; iPad < GetNumPads(); iPad++) {
    Int_t padRow = fPadArray[iPad].GetRow();
    Int_t padLayer = fPadArray[iPad].GetLayer();

    if (row == padRow && layer == padLayer)
      return &fPadArray[iPad];
  }

  return 0;
}
