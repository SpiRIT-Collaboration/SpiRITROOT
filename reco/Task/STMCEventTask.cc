#include "STMCEventTask.hh"

#include "FairRootManager.h"

STMCEventTask::STMCEventTask()
: FairTask("STMCEventTask"),
  fIsPersistence(kFALSE),
  fPrimaryTrackArray(NULL),
  fFairMCEventHeader(NULL),
  fTAMCPointArray(NULL),
  fMCEventHeader(NULL),
  fMCTriggerResponse(NULL),
  fNProbBin(100),
  fProbGraph(nullptr)
{}

STMCEventTask::~STMCEventTask()
{}

InitStatus STMCEventTask::Init()
{

   FairRootManager* ioman = FairRootManager::Instance();
   fPrimaryTrackArray = (TClonesArray*) ioman->GetObject("PrimaryTrack");
   fFairMCEventHeader = (FairMCEventHeader*) ioman->GetObject("MCEventHeader.");
   fTAMCPointArray    = (TClonesArray*) ioman->GetObject("TAMCPoint");

   fMCEventHeader = new STMCEventHeader();
   fMCTriggerResponse = new STMCTriggerResponse();
   ioman -> Register("STMCEventHeader","ST",fMCEventHeader,fIsPersistence);
   ioman -> Register("STMCTriggerResponse","ST",fMCTriggerResponse,fIsPersistence);

   fMCEventHeader -> SetBeamA(132);
   fMCEventHeader -> SetBeamZ(50);
   fMCEventHeader -> SetBeamE(270.);
   fMCEventHeader -> SetTargetA(124);
   fMCEventHeader -> SetTargetZ(50);

/*
   TString probFileName = gSystem->Getenv("VMCWORKDIR") + "/parameters/probability.dat";
   ifstream probFile(probFileName.Data());
   for(Int_t iBin=0; iBin<fNProbBin; iBin++)
      probFile <<  <<  ;
*/
   return kSUCCESS;
}

void STMCEventTask::SetParContainers()
{}

void STMCEventTask::Exec(Option_t* opt)
{
   fMCEventHeader->Clear();

   fMCEventHeader->SetB(fFairMCEventHeader->GetB());
   TVector3 pv(fFairMCEventHeader->GetX(),fFairMCEventHeader->GetY(),fFairMCEventHeader->GetZ());
   fMCEventHeader->SetPrimaryVertex(pv);
   fMCEventHeader->SetBeamAngle(TVector2(fFairMCEventHeader->GetRotX(),fFairMCEventHeader->GetRotY()));
   fMCEventHeader->SetReactionPlane(fFairMCEventHeader->GetRotZ());
   fMCEventHeader->SetPrimaryTracks(fPrimaryTrackArray);


}

void STMCEventTask::SetPersistence(Bool_t value) { fIsPersistence = value; }

ClassImp(STMCEventTask);
