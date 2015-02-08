/**
 * @brief SPiRIT Simple Event Generator
 * @author JungWoo Lee (Korea Univ.)
 */

#include "STSimpleEventGenerator.hh"
#include <iostream>

using namespace std;

ClassImp(STSimpleEventGenerator);

STSimpleEventGenerator::STSimpleEventGenerator()
: FairGenerator(),
  fGenFileName(""),
  fGenFile(NULL),
  fV3Vertex(TVector3(0,0,0))
{
}

STSimpleEventGenerator::STSimpleEventGenerator(TString fileName)
: FairGenerator("STEventGen",fileName),
  fGenFileName(fileName),
  fGenFile(NULL),
  fV3Vertex(TVector3(0,0,0))
{
  LOG(INFO)<<"-I Opening EventGen file "<<fGenFileName<<FairLogger::endl;
  fGenFile.open(fGenFileName.Data());
  if(!fGenFile.is_open())
    LOG(FATAL)<<"Cannont open EventGen file."<<fGenFileName<<FairLogger::endl;
}

STSimpleEventGenerator::~STSimpleEventGenerator()
{
  if(fGenFile) fGenFile.close();
}

Bool_t
STSimpleEventGenerator::ReadEvent(FairPrimaryGenerator* primGen)
{
  Int_t pdg;
  Double_t px;
  Double_t py;
  Double_t pz;
  Int_t nTracks;

  if(!(fGenFile>>pdg>>px>>py>>pz>>nTracks)){
    LOG(INFO)<<"End of Events."<<FairLogger::endl;
    return kFALSE;
  }

  for(Int_t i=0; i<nTracks; i++)
    primGen->AddTrack(pdg,px,py,pz,fV3Vertex.X(),fV3Vertex.Y(),fV3Vertex.Z());

  return kTRUE;
}
