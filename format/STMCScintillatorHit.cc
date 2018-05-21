#include "STMCScintillatorHit.hh"

STMCScintillatorHit::STMCScintillatorHit()
: TObject()
{
}

STMCScintillatorHit::~STMCScintillatorHit()
{
}

STMCScintillatorHit::STMCScintillatorHit(const STMCScintillatorHit& hit)
: TObject(hit),
  fDetectorID(hit.fDetectorID),
  fTotalEdep(hit.fTotalEdep),
  fMaxZ(hit.fMaxZ),
  fMCPointArray(hit.fMCPointArray)
{
}

void STMCScintillatorHit::AddStep(STMCPoint* point)
{
   if(point->GetEnergyLoss()<=0.) return;

   fTotalEdep += point->GetEnergyLoss()*1000.;
   fMCPointArray.push_back(*point);

}

ClassImp(STMCScintillatorHit);
