#include "SPiRIT.h"

#include "SPiRITPoint.h"
#include "SPiRITGeo.h"
#include "SPiRITGeoPar.h"

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
using std::cout;
using std::endl;

SPiRIT::SPiRIT()
  : FairDetector("SPiRIT", kTRUE, kSPiRIT),
    fTrackID(-1),
    fVolumeID(-1),
    fPos(),
    fMom(),
    fTime(-1.),
    fLength(-1.),
    fELoss(-1),
    fSPiRITPointCollection(new TClonesArray("SPiRITPoint"))
{
}

SPiRIT::SPiRIT(const char* name, Bool_t active)
  : FairDetector(name, active, kSPiRIT),
    fTrackID(-1),
    fVolumeID(-1),
    fPos(),
    fMom(),
    fTime(-1.),
    fLength(-1.),
    fELoss(-1),
    fSPiRITPointCollection(new TClonesArray("SPiRITPoint"))
{
}

SPiRIT::~SPiRIT()
{
  if (fSPiRITPointCollection) {
    fSPiRITPointCollection->Delete();
    delete fSPiRITPointCollection;
  }
}

void SPiRIT::Initialize()
{
  FairDetector::Initialize();
  FairRuntimeDb* rtdb= FairRun::Instance()->GetRuntimeDb();
  SPiRITGeoPar* par=(SPiRITGeoPar*)(rtdb->getContainer("SPiRITGeoPar"));
}

Bool_t  SPiRIT::ProcessHits(FairVolume* vol)
{
  /** This method is called from the MC stepping */

  //Set parameters at entrance of volume. Reset ELoss.
  if ( gMC->IsTrackEntering() ) {
    fELoss  = 0.;
    fTime   = gMC->TrackTime() * 1.0e09;
    fLength = gMC->TrackLength();
    gMC->TrackPosition(fPos);
    gMC->TrackMomentum(fMom);
  }

  // Sum energy loss for all steps in the active volume
  fELoss += gMC->Edep();

  // Create SPiRITPoint at exit of active volume
  if ( gMC->IsTrackExiting()    ||
       gMC->IsTrackStop()       ||
       gMC->IsTrackDisappeared()   ) {
    fTrackID  = gMC->GetStack()->GetCurrentTrackNumber();
    fVolumeID = vol->getMCid();
    if (fELoss == 0. ) { return kFALSE; }
    AddHit(fTrackID, fVolumeID, TVector3(fPos.X(),  fPos.Y(),  fPos.Z()),
           TVector3(fMom.Px(), fMom.Py(), fMom.Pz()), fTime, fLength,
           fELoss);

    // Increment number of SPiRIT det points in TParticle
    STStack* stack = (STStack*) gMC->GetStack();
    stack->AddPoint(kSPiRIT);
  }

  return kTRUE;
}

void SPiRIT::EndOfEvent()
{

  fSPiRITPointCollection->Clear();

}



void SPiRIT::Register()
{

  /** This will create a branch in the output tree called
      SPiRITPoint, setting the last parameter to kFALSE means:
      this collection will not be written to the file, it will exist
      only during the simulation.
  */

  FairRootManager::Instance()->Register("SPiRITPoint", "SPiRIT",
                                        fSPiRITPointCollection, kTRUE);

}


TClonesArray* SPiRIT::GetCollection(Int_t iColl) const
{
  if (iColl == 0) { return fSPiRITPointCollection; }
  else { return NULL; }
}

void SPiRIT::Reset()
{
  fSPiRITPointCollection->Clear();
}

void SPiRIT::ConstructGeometry()
{
  /** If you are using the standard ASCII input for the geometry
      just copy this and use it for your detector, otherwise you can
      implement here you own way of constructing the geometry. */

  TString fileName = GetGeometryFileName();
  if(fileName.EndsWith(".root")) {
    std::cout<<"SPiRIT::ConstructGeometry() "
	     <<"  ...using ROOT geometry"<<std::endl;
    ConstructRootGeometry();
    return;
  }

  FairGeoLoader*    geoLoad = FairGeoLoader::Instance();
  FairGeoInterface* geoFace = geoLoad->getGeoInterface();
  SPiRITGeo*  Geo  = new SPiRITGeo();
  Geo->setGeomFile(GetGeometryFileName());
  geoFace->addGeoModule(Geo);

  Bool_t rc = geoFace->readSet(Geo);
  if (rc) { Geo->create(geoLoad->getGeoBuilder()); }
  TList* volList = Geo->getListOfVolumes();

  // store geo parameter
  FairRun* fRun = FairRun::Instance();
  FairRuntimeDb* rtdb= FairRun::Instance()->GetRuntimeDb();
  SPiRITGeoPar* par=(SPiRITGeoPar*)(rtdb->getContainer("SPiRITGeoPar"));
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

SPiRITPoint* SPiRIT::AddHit(Int_t trackID, Int_t detID,
                                      TVector3 pos, TVector3 mom,
                                      Double_t time, Double_t length,
                                      Double_t eLoss)
{
  TClonesArray& clref = *fSPiRITPointCollection;
  Int_t size = clref.GetEntriesFast();
  return new(clref[size]) SPiRITPoint(trackID, detID, pos, mom,
         time, length, eLoss);
}

ClassImp(SPiRIT)
