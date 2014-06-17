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
{
  fEventID = -1;
  fNumPads = 0;

  memset(fPadsArray, 0, sizeof(fPadsArray));
}

STRawEvent::~STRawEvent()
{
  for (Int_t iPad = 0; iPad < fNumPads; iPad++)
    delete fPadsArray[iPad];
}

void STRawEvent::PrintPads()
{
  for (Int_t iPad = 0; iPad < fNumPads; iPad++) {
    std::cout << "Pad: " << std::setw(5) << iPad;
    std::cout << " (" << std::setw(3) << fPadsArray[iPad] -> GetRow();
    std::cout << ", " << std::setw(3) << fPadsArray[iPad] -> GetLayer() << ")";
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
  fPadsArray[fNumPads] = pad;
  fNumPads++;
}

// getters
Int_t STRawEvent::GetEventID()
{
  return fEventID;
}

Int_t STRawEvent::GetNumPads()
{
  return fNumPads;
}

STPad *STRawEvent::GetPad(Int_t padNo)
{
  return (padNo < fNumPads ? fPadsArray[padNo] : 0);
}

STPad *STRawEvent::GetPad(Int_t row, Int_t layer)
{
  for (Int_t iPad = 0; iPad < fNumPads; iPad++) {
    Int_t padRow = fPadsArray[iPad] -> GetRow();
    Int_t padLayer = fPadsArray[iPad] -> GetLayer();

    if (row == padRow && layer == padLayer)
      return fPadsArray[iPad];
  }

  return 0;
}
