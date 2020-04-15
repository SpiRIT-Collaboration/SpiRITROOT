#define cRED "\033[1;31m"
#define cYELLOW "\033[1;33m"
#define cNORMAL "\033[0m"

#include "STParameters.hh"

#include "TObjArray.h"
#include "TObjString.h"

#include <iostream>
#include <fstream>

using std::cout;
using std::cerr;
using std::endl;
using std::ifstream;

ClassImp(STParameters)

STParameters::STParameters()
{
  cout << "= [STParameters] " << cRED << "DB files are not specified!" << cNORMAL << endl;
  cout << "                " << cYELLOW << "SetRunID()" << cRED << ", " << cYELLOW << "ReadSystemDB() " << cRED << "and " << cYELLOW << "ReadRunDB() " << cRED << "must be called!" << cNORMAL << endl;
}

STParameters::STParameters(Int_t runID, TString systemDB, TString runDB)
{
  SetRunID(runID);
  ReadSystemDB(systemDB);
  ReadRunDB(runDB);
}

void STParameters::SetRunID(Int_t runID)
{
  fRunID = runID;
}

void STParameters::ReadSystemDB(TString systemDB)
{
  TString line;

  ifstream dbFile(systemDB.Data());
  line.ReadLine(dbFile); // Skip the first line
  while (line.ReadLine(dbFile)) {
    auto lineArray = line.Tokenize(",");

    auto systemID = ((TObjString *) lineArray -> At(0)) -> String().Atoi();
    fDriftVelocity[systemID] = ((TObjString *) lineArray -> At(1)) -> String().Atof();
    fFieldOffsetX[systemID] = ((TObjString *) lineArray -> At(2)) -> String().Atof();
    fFieldOffsetY[systemID] = ((TObjString *) lineArray -> At(3)) -> String().Atof();
    fFieldOffsetZ[systemID] = ((TObjString *) lineArray -> At(4)) -> String().Atof();
    fTargetZ[systemID] = ((TObjString *) lineArray -> At(5)) -> String().Atof();
    fParameterFile[systemID] = ((TObjString *) lineArray -> At(6)) -> String();

    delete lineArray;
  }

  fIsSystemDBSet = kTRUE;
}

void STParameters::ReadRunDB(TString runDB)
{
  TString line;

  ifstream dbFile(runDB.Data());
  line.ReadLine(dbFile); // Skip the first line
  while (line.ReadLine(dbFile)) {
    auto lineArray = line.Tokenize(",");

    auto runID = ((TObjString *) lineArray -> At(0)) -> String().Atoi();
    fSystem[runID] = ((TObjString *) lineArray -> At(1)) -> String().Atoi();
    fTotalEvents[runID] = ((TObjString *) lineArray -> At(2)) -> String().Atoi();
    fSheetChargeDensity[runID] = ((TObjString *) lineArray -> At(3)) -> String().Atof();
    fYPedestal[runID] = ((TObjString *) lineArray -> At(4)) -> String().Atof();
    fBDCOffsetX[runID] = ((TObjString *) lineArray -> At(5)) -> String().Atof();
    fBDCOffsetY[runID] = ((TObjString *) lineArray -> At(6)) -> String().Atof();
    fGGRun[runID] = ((TObjString *) lineArray -> At(7)) -> String().Atoi();
    fRelativeGainRun[runID] = ((TObjString *) lineArray -> At(8)) -> String().Atoi();

    delete lineArray;
  }

  fIsRunDBSet = kTRUE;
}

void STParameters::CheckOk()
{
  if (!fIsRunDBSet) {
    cout << "= [STParameters] " << cRED << "Call " << cYELLOW << "ReadRunDB() " << cRED << "first!" << cNORMAL << endl;

    exit(EXIT_FAILURE);
  }

  if (!fIsSystemDBSet) {
    cout << "= [STParameters] " << cRED << "Call " << cYELLOW << "ReadSystemDB() " << cRED << "first!" << cNORMAL << endl;

    exit(EXIT_FAILURE);
  }

  if (fRunID == -9999) {
    cout << "= [STParameters] " << cRED << "Call " << cYELLOW << "SetRunID() " << cRED << "first!" << cNORMAL << endl;

    exit(EXIT_FAILURE);
  }
}

Int_t STParameters::GetSystemID()
{
  CheckOk();

  return fSystem[fRunID];
}

Int_t STParameters::GetNumTotalEvents()
{
  CheckOk();

  return fTotalEvents[fRunID];
}

Double_t STParameters::GetSheetChargeDensity()
{
  CheckOk();

  return fSheetChargeDensity[fRunID];
}

Double_t STParameters::GetBDCOffsetX()
{
  CheckOk();

  return fBDCOffsetX[fRunID];
}

Double_t STParameters::GetBDCOffsetY()
{
  CheckOk();

  return fBDCOffsetY[fRunID];
}

Int_t STParameters::GetGGRunID()
{
  CheckOk();

  return fGGRun[fRunID];
}

Int_t STParameters::GetRelativeGainRunID()
{
  CheckOk();

  return fRelativeGainRun[fRunID];
}

Double_t STParameters::GetYPedestal()
{
  CheckOk();

  return fYPedestal[fRunID];
}

Double_t STParameters::GetDriftVelocity()
{
  CheckOk();

  return fDriftVelocity[GetSystemID()];
}

Double_t STParameters::GetFieldOffsetX()
{
  CheckOk();

  return fFieldOffsetX[GetSystemID()];
}

Double_t STParameters::GetFieldOffsetY()
{
  CheckOk();

  return fFieldOffsetY[GetSystemID()];
}

Double_t STParameters::GetFieldOffsetZ()
{
  CheckOk();

  return fFieldOffsetZ[GetSystemID()];
}

Double_t STParameters::GetTargetZ()
{
  CheckOk();

  return fTargetZ[GetSystemID()];
}

const Char_t *STParameters::GetParameterFile()
{
  CheckOk();

  return fParameterFile[GetSystemID()];
}
