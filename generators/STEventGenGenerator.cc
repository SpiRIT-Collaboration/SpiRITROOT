/**
 * @brief SPiRIT Event Generator
 * @author JungWoo Lee (Korea Univ.)
 */

#include "STEventGenGenerator.hh"
#include "TSystem.h"
#include "FairMCEventHeader.h"

ClassImp(STEventGenGenerator);

STEventGenGenerator::STEventGenGenerator()
: FairGenerator(),
  fGenFileName(""),
  fV3Vertex(TVector3(0,0,0)),
  fNEvents(0)
{
}

STEventGenGenerator::STEventGenGenerator(TString fileName)
: FairGenerator("STEventGen",fileName),
  fGenFileName(fileName),
  fV3Vertex(TVector3(0,0,0)),
  fNEvents(0)
{
  TString input_dir = gSystem->Getenv("VMCWORKDIR");
  fGenFileName = input_dir+"/input/"+fGenFileName;

  LOG(INFO)<<"-I Opening EventGen file "<<fGenFileName<<FairLogger::endl;
  fGenFile.open(fGenFileName.Data());
  if(!fGenFile.is_open())
    LOG(FATAL)<<"Cannont open EventGen file."<<fGenFileName<<FairLogger::endl;
  fGenFile>>fNEvents;
}

STEventGenGenerator::~STEventGenGenerator()
{
  if(fGenFile.is_open()) fGenFile.close();
}

Bool_t
STEventGenGenerator::ReadEvent(FairPrimaryGenerator* primGen)
{
  Int_t eventID;
  Int_t nTracks;
  Double_t b;

  if(!(fGenFile>>eventID>>nTracks>>b)) {
    LOG(INFO)<<"End of EventGen."<<FairLogger::endl;
    return kFALSE;
  }

  FairMCEventHeader* event = primGen -> GetEvent();
  if( event && (!event -> IsSet()) ){
    event -> SetEventID(eventID);
    event -> SetB(b);
    event -> SetNPrim(nTracks);
    event -> MarkSet(kTRUE);
  }

  Int_t pdg;
  Double_t px;
  Double_t py;
  Double_t pz;

  for(Int_t i=0; i<nTracks; i++){
    fGenFile>>pdg>>px>>py>>pz;
    primGen->AddTrack(pdg,px,py,pz,fV3Vertex.X(),fV3Vertex.Y(),fV3Vertex.Z());
  }

  return kTRUE;
}
