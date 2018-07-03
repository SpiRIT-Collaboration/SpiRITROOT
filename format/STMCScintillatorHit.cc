#include "STMCScintillatorHit.hh"
#include "STMCPoint.hh"
#include "TDatabasePDG.h"

STMCScintillatorHit::STMCScintillatorHit()
:TObject(),
  fDetectorID(-1),
  fTotalEdep(0.),
  fMaxZ(-1)
{
  fMCPointArray.clear();
}

STMCScintillatorHit::~STMCScintillatorHit()
{
}

STMCScintillatorHit::STMCScintillatorHit(const STMCScintillatorHit& hit)
:TObject(hit),
  fDetectorID(hit.fDetectorID),
  fTotalEdep(hit.fTotalEdep),
  fMaxZ(hit.fMaxZ),
  fMCPointArray(hit.fMCPointArray)
{
}

void STMCScintillatorHit::AddStep(STMCPoint *point)
{
  if(point->GetEnergyLoss()<=0.) return;

  fTotalEdep += point->GetEnergyLoss();
  fMCPointArray.push_back(*point);

  Int_t pdg = point->GetPDG();
  Int_t z;
  if(pdg==2212) z = 1;
  else if(pdg>10000) z = (pdg%10000)/10;
  else if(TDatabasePDG::Instance()->GetParticle(pdg)) z = TDatabasePDG::Instance()->GetParticle(pdg)->Charge()/3.;  // |e|/3 unit. 
  else z = 0;
  z = z>=GetMaxZ() ? z : GetMaxZ();
  SetMaxZ(z);

}

void STMCScintillatorHit::AddStep(Double_t edep, STMCPoint *point)
{
  if(point->GetEnergyLoss()<=0.) return;

  fTotalEdep += edep;

  fMCPointArray.push_back(*point);


  Int_t pdg = point->GetPDG();
  Int_t z;
  if(pdg==2212) z = 1;
  else if(pdg>10000) z = (pdg%10000)/10;
  else if(TDatabasePDG::Instance()->GetParticle(pdg)) z = TDatabasePDG::Instance()->GetParticle(pdg)->Charge()/3.;  // |e|/3 unit. 
  else z = 0;
  z = z>=GetMaxZ() ? z : GetMaxZ();
  SetMaxZ(z);

}

ClassImp(STMCScintillatorHit);
