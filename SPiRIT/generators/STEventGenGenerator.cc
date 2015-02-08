/**
 * @brief SPiRIT Event Generator
 * @author JungWoo Lee (Korea Univ.)
 */

#include "STEventGenGenerator.hh"
#include <iostream>

using namespace std;

ClassImp(STEventGenGenerator);

STEventGenGenerator::STEventGenGenerator()
: FairGenerator(),
  fGenFileName(""),
  fGenFile(NULL),
  fV3Vertex(TVector3(0,0,0)),
  fNEvents(0)
{
}

STEventGenGenerator::STEventGenGenerator(TString fileName)
: FairGenerator("STEventGen",fileName),
  fGenFileName(fileName),
  fGenFile(NULL),
  fV3Vertex(TVector3(0,0,0)),
  fNEvents(0)
{
  LOG(INFO)<<"-I Opening EventGen file "<<fGenFileName<<FairLogger::endl;
  fGenFile.open(fGenFileName.Data());
  if(!fGenFile.is_open())
    LOG(FATAL)<<"Cannont open EventGen file."<<fGenFileName<<FairLogger::endl;
  fGenFile>>fNEvents;
}

STEventGenGenerator::~STEventGenGenerator()
{
  if(fGenFile) fGenFile.close();
}

Bool_t
STEventGenGenerator::ReadEvent(FairPrimaryGenerator* primGen)
{
  Int_t eventID;
  Int_t nTracks;

  if(!(fGenFile>>eventID>>nTracks)) {
    LOG(INFO)<<"End of EventGen."<<FairLogger::endl;
    return kFALSE;
  }

  Int_t pdg;
  Double_t px;
  Double_t py;
  Double_t pz;

  for(Int_t i=0; i<nTracks; i++){
    fGenFile>>pdg>>px>>py>>pz;
    primGen->AddTrack(pdg,px,py,pz,fV3Vertex.X(),fV3Vertex.Y(),fV3Vertex.Z());
    cout<<pdg<<" "<<px<<" "<<py<<" "<<pz<<endl;
  }

  return kTRUE;
}
