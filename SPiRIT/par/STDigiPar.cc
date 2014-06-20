//---------------------------------------------------------------------
// Description:
//      STDigiPar reads in parameters and stores them for later use.
//
// Author List:
//      Genie Jhang     Korea Univ.            (original author)
//----------------------------------------------------------------------

#include "STDigiPar.hh"

ClassImp(STDigiPar)

STDigiPar::STDigiPar(const Char_t *name, const Char_t *title, const Char_t *context)
:FairParGenericSet("STDigiPar", "SPiRIT Parameter Container", "")
{
  fInitialized = kFALSE;

  fLogger = FairLogger::GetLogger();
}

STDigiPar::~STDigiPar()
{
}

STGas *STDigiPar::GetGas()
{
  return fGas;
}

Bool_t STDigiPar::getParams(FairParamList *paramList)
{
  if (!paramList) {
    fLogger -> Fatal(MESSAGE_ORIGIN, "Parameter list doesn't exist!");
    return kFALSE;
  }

  if (!fInitialized) {
    if (!(paramList -> fill("STGasFile", &fGasFile))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find STGasFile parameter!");
      return kFALSE;
    }
    if (!(paramList -> fill("EField", &fEField))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find EField parameter!");
      return kFALSE;
    }
    if (!(paramList -> fill("PadPlaneFile", &fPadPlaneFile))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find PadPlaneFile parameter!");
      return kFALSE;
    }
    if (!(paramList -> fill("PadShapeFile", &fPadShapeFile))) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find PadShapeFile parameter!");
      return kFALSE;
    }
  }

  return kTRUE;
}

void STDigiPar::putParams(FairParamList *paramList)
{
  if (!paramList) {
    fLogger -> Fatal(MESSAGE_ORIGIN, "Parameter list doesn't exist!");
    return;
  }

  paramList -> add("STGasFile", fGasFile);
  paramList -> add("EField", fEField);
  paramList -> add("PadPlaneFile", fPadPlaneFile);
  paramList -> add("PadShapeFile", fPadShapeFile);
}
