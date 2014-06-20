// =================================================
//  STMap Class
// 
//  Description:
//    This class is used for finding the local pad
//    coordinates corresponding to user-input
//    agetIdx & chIdx using map by Tadaaki Isobe.
// 
//  Genie Jhang ( geniejhang@majimak.com )
//  2013. 08. 13
// =================================================

#include "STMap.hh"

#include <fstream>
#include <iostream>

ClassImp(STMap);

STMap::STMap()
{
  fIsSetUAMap = 0;
  fIsSetAGETMap = 0;
}

// Getters
void STMap::GetRowNLayer(Int_t coboIdx, Int_t asadIdx, Int_t agetIdx, Int_t chIdx, Int_t &padRow, Int_t &padLayer) {
  if (padLayerOfCh[chIdx] == -2) {
    padLayer = -2;
    padRow = -2;

    return;
  }

  Int_t UAIdx = GetUAIdx(coboIdx, asadIdx);
  if (UAIdx < 24) {
    padRow = (UAIdx/4)*9 + padRowOfCh[chIdx];
    padLayer = (UAIdx%4)*28 + agetIdx*7 + padLayerOfCh[chIdx];
  } else {
    padRow = (UAIdx/4)*9 + (8 - padRowOfCh[chIdx]); 
    padLayer = (UAIdx%4)*28 + (3 - agetIdx)*7 + (6 - padLayerOfCh[chIdx]);
  }

  return;
}

void STMap::GetMapData(Int_t padRow, Int_t padLayer, Int_t &UAIdx, Int_t &coboIdx, Int_t &asadIdx, Int_t &agetIdx, Int_t &chIdx)
{
  if (padRow < 0 || padRow >= 108 || padLayer < 0 || padLayer >= 112) {
    UAIdx = -1;
    coboIdx = -1;
    asadIdx = -1;
    agetIdx = -1;
    chIdx = -1;

    return;
  }

  UAIdx = (padRow/9)*4 + padLayer/28;
  coboIdx = GetCoboIdx(UAIdx);
  asadIdx = GetAsadIdx(UAIdx);

  if (UAIdx < 24) {
    Int_t agetRow = padRow%9;
    Int_t uaLayer = padLayer%28;

    agetIdx = uaLayer/7;
    Int_t agetLayer = uaLayer%7;

    for (Int_t iCh = 0; iCh < 68; iCh++)
      if (padRowOfCh[iCh] == agetRow && padLayerOfCh[iCh] == agetLayer) {
        chIdx = iCh;
        break;
      }
  } else {
    Int_t agetRow = (8 - padRow%9);
    Int_t uaLayer = (27 - padLayer%28);

    agetIdx = uaLayer/7;
    Int_t agetLayer = uaLayer%7;

    for (Int_t iCh = 0; iCh < 68; iCh++)
      if (padRowOfCh[iCh] == agetRow && padLayerOfCh[iCh] == agetLayer) {
        chIdx = iCh;
        break;
      }
  }
}

Bool_t STMap::IsSetUAMap()
{
  return fIsSetUAMap;
}

Bool_t STMap::IsSetAGETMap()
{
  return fIsSetAGETMap;
}

void STMap::SetUAMap(TString filename)
{
  char dummy[25];
  std::ifstream uaMap(filename.Data());
  uaMap.getline(dummy, 200);

  if (!uaMap.is_open()) {
    std::cout << filename << " is not loaded! Check the existance of the file!" << std::endl;

    fIsSetUAMap = 0;

    return;
  }

  Int_t idx = -1;
  while (!(uaMap.eof())) {
    uaMap >> idx;
    uaMap >> cobo[idx] >> asad[idx];

    ua[idx] = idx;
  }

  fIsSetUAMap = 1;
}

void STMap::SetAGETMap(TString filename)
{
  char dummy[25];
  std::ifstream chToPadMap(filename.Data());
  chToPadMap.getline(dummy, 200);

  if (!chToPadMap.is_open()) {
    std::cout << filename << " is not loaded! Check the existance of the file!" << std::endl;

    fIsSetAGETMap = 0;

    return;
  }

  Int_t ch = -1;
  while (!(chToPadMap.eof())) {
    chToPadMap >> ch;
    chToPadMap >> padLayerOfCh[ch] >> padRowOfCh[ch];
  }

  fIsSetAGETMap = 1;
}

Int_t STMap::GetUAIdx(Int_t coboIdx, Int_t asadIdx)
{
  for (Int_t iUA = 0; iUA < 48; iUA++)
    if (cobo[iUA] == coboIdx && asad[iUA] == asadIdx)
      return ua[iUA];

  return -1;
}

Int_t STMap::GetCoboIdx(Int_t uaIdx)
{
  for (Int_t iUA = 0; iUA < 48; iUA++)
    if (ua[iUA] == uaIdx)
      return cobo[iUA];

  return -1;
}

Int_t STMap::GetAsadIdx(Int_t uaIdx)
{
  for (Int_t iUA = 0; iUA < 48; iUA++)
    if (ua[iUA] == uaIdx)
      return asad[iUA];

  return -1;
}
