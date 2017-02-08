// =================================================
//  STPadRemover Class
// 
//  Description:
//    Removes pads from being analyzed
// 
//  Genie Jhang ( geniejhang@majimak.com )
//  2017. 02. 06
// =================================================

#include "STPadRemover.hh"

ClassImp(STPadRemover)

STPadRemover::STPadRemover()
{
  ClassInit();
}

STPadRemover::STPadRemover(TString datafile)
{
  ClassInit();

  SetDatafile(datafile);
}

Bool_t STPadRemover::Init()
{
  if (fDatafile.EqualTo("")) {
    std::cout << "= [STPadRemover] Data file is not set specified!" << std::endl;

    fIsInitialized = kFALSE;
    return fIsInitialized;
  }

  FindEvent();

/*
  std::ifstream file(fDatafile.Data());
  Int_t row = 0, layer = 0, eventid = 0, numPads = 0;
  while (file >> row >> layer) {
    if (file.eof())
      break;

    if (row == -1) {
      eventid = layer;
      continue;
    }

    fPadList.push_back(std::make_pair(row, layer));
    numPads++;
  }

  fEventList.push_back(std::make_pair(eventid, numPads));
*/

  fIsInitialized = kTRUE;
  return fIsInitialized;
}

void STPadRemover::SetDatafile(TString datafile)
{
  fDatafile = datafile;
}

Bool_t STPadRemover::CheckBadPad(Int_t row, Int_t layer, Int_t eventid)
{
  if (!fIsInitialized) {
    std::cout << "= [STPadRemover] Class is not initialized properly!" << std::endl;

    return fIsInitialized;
  }

  Bool_t found = FindEvent(eventid);

  if (!found)
    return kFALSE;

  return fPads[layer*108 + row];

/*
  Int_t numEvents = fEventList.size();
  Int_t numEntries = -1;
  for (Int_t iEvent = 0; iEvent < numEvents; iEvent++)
    if (fEventList[iEvent].first == eventid)
      numEntries = fEventList[iEvent].second;
  
  if (numEntries == -1)
    return kFALSE;

  for (Int_t iEntry = 0; iEntry < numEntries; iEntry++)
    if (fPadList[iEntry].first == row && fPadList[iEntry].second == layer)
      return kTRUE;
*/

  return kFALSE;
}

void STPadRemover::ClassInit()
{
  fIsInitialized = kFALSE;
  fDatafile = "";

  memset(fPads, 0, sizeof(Bool_t)*12096);
}

Bool_t STPadRemover::FindEvent(Int_t eventid)
{
  if (fEventID == eventid)
    return kTRUE;

  std::ifstream file(fDatafile.Data());
  Bool_t found = kFALSE;
  Int_t row = 0, layer = 0;
  while (file >> row >> layer) {
    if (file.eof())
      break;

    if (row == -1) {
      if (found)
        break;

      fEventID = layer;

      memset(fPads, 0, sizeof(Bool_t)*12096);

      if (fEventID == eventid)
        found = kTRUE;

      continue;
    }

    if (fEventID != eventid)
      continue;

    fPads[layer*108 + row] = 1;
  }
  file.close();

  return found;
}
