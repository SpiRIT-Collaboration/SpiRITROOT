#include "STDetector.hh"
#include "STMCPoint.hh"
#include "STGeo.hh"
#include "STGeoPar.hh"

#include "FairVolume.h"
#include "FairGeoVolume.h"
#include "FairGeoNode.h"
#include "FairRootManager.h"
#include "FairGeoLoader.h"
#include "FairGeoInterface.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"
#include "STDetectorList.h"
#include "STStack.h"

#include "TClonesArray.h"
#include "TVirtualMC.h"

#include <iostream>
#include <stdio.h>
#include <cstring>

using std::cout;
using std::endl;

STDetector::STDetector()
  : FairDetector("SPiRIT", kTRUE, kSPiRIT),
  fTrackID(-1),
  fVolumeID(-1),
  fPos(),
  fMom(),
  fTime(-1.),
  fLength(-1.),
  fELoss(-1),
  fPdg(0),
  fSTMCPointCollection(new TClonesArray("STMCPoint"))
{
}

STDetector::STDetector(const char* name, Bool_t active)
  : FairDetector(name, active, kSPiRIT),
  fTrackID(-1),
  fVolumeID(-1),
  fPos(),
  fMom(),
  fTime(-1.),
  fLength(-1.),
  fELoss(-1),
  fPdg(0),
  fSTMCPointCollection(new TClonesArray("STMCPoint"))
{
}

STDetector::~STDetector()
{
  if (fSTMCPointCollection) {
    fSTMCPointCollection->Delete();
    delete fSTMCPointCollection;
  }
}

  void 
STDetector::Initialize()
{
  FairDetector::Initialize();
  FairRuntimeDb* rtdb= FairRun::Instance()->GetRuntimeDb();
  STGeoPar* par=(STGeoPar*)(rtdb->getContainer("STGeoPar"));
}

  Bool_t
STDetector::ProcessHits(FairVolume* vol)
{
  if(!(gMC -> IsTrackInside()))
    return kFALSE;

  fELoss = gMC->Edep();

  if(fELoss==0) 
    return kFALSE;


  STStack* stack = (STStack*) gMC->GetStack();

  fTrackID     = stack->GetCurrentTrackNumber();
  fVolumeID    = vol->getMCid();
  fTime        = gMC->TrackTime() * 1.0e09;
  fLength      = gMC->TrackLength();
  gMC->TrackPosition(fPos);
  gMC->TrackMomentum(fMom);
  fPdg         = gMC->TrackPid();

  if(fSaveParentTrackID)
  {
    int parentID = stack -> GetCurrentParentTrackNumber();
    if(parentID > -1)
    {   
      auto it = fToParent.find(parentID); 
      for(;it != fToParent.end(); it = fToParent.find(parentID))
        parentID = it -> second;
      fToParent[fTrackID] = parentID;
      fTrackID = parentID;

      auto it2 = fParentPDG.find(fTrackID);
      if(it2 != fParentPDG.end()) fPdg = it2 -> second;
    }
    else fParentPDG[fTrackID] = fPdg;
  }
  AddHit(fTrackID, fVolumeID, 
      TVector3(fPos.X(),  fPos.Y(),  fPos.Z()),
      TVector3(fMom.Px(), fMom.Py(), fMom.Pz()), 
      fTime, fLength, fELoss, fPdg);

  if(std::strncmp(gMC -> CurrentVolName(), "kyoto", std::min(5, int(std::strlen(gMC -> CurrentVolName())))) == 0)  {
    gMC -> StopTrack();
    return kFALSE;
  }

  stack->AddPoint(kSPiRIT);
  
  Double_t stepLength = gMC->TrackStep();
  stack->AddPoint(fVolumeID,fELoss,stepLength);

 
  return kTRUE;
}

  void 
STDetector::EndOfEvent()
{
  fSTMCPointCollection->Clear();
  fParentPDG.clear();
  fToParent.clear();
}

  void 
STDetector::Register()
{

  /** This will create a branch in the output tree called
    STMCPoint, setting the last parameter to kFALSE means:
    this collection will not be written to the file, it will exist
    only during the simulation.
    */

  FairRootManager::Instance()->Register("STMCPoint", "SPiRIT",
      fSTMCPointCollection, kTRUE);
}


TClonesArray* STDetector::GetCollection(Int_t iColl) const
{
  if (iColl == 0) { return fSTMCPointCollection; }
  else { return NULL; }
}


void STDetector::Reset()
{
  fSTMCPointCollection->Clear();
}


void STDetector::ConstructGeometry()
{
  /** If you are using the standard ASCII input for the geometry
    just copy this and use it for your detector, otherwise you can
    implement here you own way of constructing the geometry. */

  TString fileName = GetGeometryFileName();
  if(fileName.EndsWith(".root")) {
    std::cout<<"STDetector::ConstructGeometry() "
      <<"  ...using ROOT geometry"<<std::endl;
    ConstructRootGeometry();
    return;
  }

  FairGeoLoader*    geoLoad = FairGeoLoader::Instance();
  FairGeoInterface* geoFace = geoLoad->getGeoInterface();
  STGeo*  Geo  = new STGeo();
  Geo->setGeomFile(GetGeometryFileName());
  geoFace->addGeoModule(Geo);

  Bool_t rc = geoFace->readSet(Geo);
  if (rc) { Geo->create(geoLoad->getGeoBuilder()); }
  TList* volList = Geo->getListOfVolumes();

  // store geo parameter
  FairRun* fRun = FairRun::Instance();
  FairRuntimeDb* rtdb= FairRun::Instance()->GetRuntimeDb();
  STGeoPar* par=(STGeoPar*)(rtdb->getContainer("STGeoPar"));
  TObjArray* fSensNodes = par->GetGeoSensitiveNodes();
  TObjArray* fPassNodes = par->GetGeoPassiveNodes();

  TListIter iter(volList);
  FairGeoNode* node   = NULL;
  FairGeoVolume* aVol=NULL;

  while( (node = (FairGeoNode*)iter.Next()) ) {
    aVol = dynamic_cast<FairGeoVolume*> ( node );
    if ( node->isSensitive()  ) {
      fSensNodes->AddLast( aVol );
    } else {
      fPassNodes->AddLast( aVol );
    }
  }
  par->setChanged();
  par->setInputVersion(fRun->GetRunId(),1);

  ProcessNodes ( volList );
}

  STMCPoint* 
STDetector::AddHit(Int_t trackID, Int_t detID,
    TVector3 pos, TVector3 mom,
    Double_t time, Double_t length,
    Double_t eLoss, Int_t pdg)
{
  TClonesArray& clref = *fSTMCPointCollection;
  Int_t size = clref.GetEntriesFast();
  return new(clref[size]) STMCPoint(trackID, detID, pos, mom,
      time, length, eLoss, pdg);
}

  Bool_t
STDetector::CheckIfSensitive(std::string name)
{
  TString nameStr(name);

  if (nameStr.EqualTo("field_cage_in"))
    return kTRUE;
  if (nameStr.BeginsWith("kyoto"))
    return kTRUE;
  if (nameStr.BeginsWith("katanaVPla"))
    return kTRUE;


  return kFALSE;
}

void STDetector::BeginEvent() 
{  
  auto eventHeader = FairRootManager::Instance()->GetObject("MCEventHeader.");
  if(auto castedHeader = dynamic_cast<STFairMCEventHeader*>(eventHeader))
    if(fSaveParentTrackID)
      castedHeader -> SetParentIDOnHits();
}


ClassImp(STDetector)
