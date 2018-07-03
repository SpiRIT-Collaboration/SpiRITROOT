// -------------------------------------------------------------------------
// -----                      STMCTrack source file                    -----
// -----                  Created 03/08/04  by V. Friese               -----
// -----               Redesigned 13/12/19  by G. Jhang                -----
// -------------------------------------------------------------------------
#include "STMCTrack.h"

#include "FairLogger.h"                 // for FairLogger, etc

#include "TDatabasePDG.h"               // for TDatabasePDG
#include "TParticle.h"                  // for TParticle
#include "TParticlePDG.h"               // for TParticlePDG

// -----   Default constructor   -------------------------------------------
STMCTrack::STMCTrack()
  : TObject(),
  fPdgCode(0),
  fMotherId(-1),
  fTrackId(-1),
  fPx(0.),
  fPy(0.),
  fPz(0.),
  fStartX(0.),
  fStartY(0.),
  fStartZ(0.),
  fStartT(0.),
  fNPoints(0),
  fNPointsMap(),
  fEdepMap(),
  fLengthMap()
{
}
// -------------------------------------------------------------------------



// -----   Standard constructor   ------------------------------------------
STMCTrack::STMCTrack(Int_t pdgCode, Int_t motherId, Int_t trackId, Double_t px,
    Double_t py, Double_t pz, Double_t x, Double_t y,
    Double_t z, Double_t t, Int_t nPoints = 0)
  : TObject(),
  fPdgCode(pdgCode),
  fMotherId(motherId),
  fTrackId(trackId),
  fPx(px),
  fPy(py),
  fPz(pz),
  fStartX(x),
  fStartY(y),
  fStartZ(z),
  fStartT(t),
  fNPoints(nPoints),
  fNPointsMap(),
  fEdepMap(),
  fLengthMap()
{
}
// -------------------------------------------------------------------------



// -----   Copy constructor   ----------------------------------------------
STMCTrack::STMCTrack(const STMCTrack& track)
  : TObject(track),
  fPdgCode(track.fPdgCode),
  fMotherId(track.fMotherId),
  fTrackId(track.fTrackId),
  fPx(track.fPx),
  fPy(track.fPy),
  fPz(track.fPz),
  fStartX(track.fStartX),
  fStartY(track.fStartY),
  fStartZ(track.fStartZ),
  fStartT(track.fStartT),
  fNPoints(track.fNPoints),
  fNPointsMap(track.fNPointsMap),
  fEdepMap(track.fEdepMap),
  fLengthMap(track.fLengthMap)
{
}
// -------------------------------------------------------------------------



// -----   Constructor from TParticle   ------------------------------------
STMCTrack::STMCTrack(TParticle* part)
  : TObject(),
  fPdgCode(part->GetPdgCode()),
  fMotherId(part->GetMother(0)),
  fTrackId(-1),
  fPx(part->Px()),
  fPy(part->Py()),
  fPz(part->Pz()),
  fStartX(part->Vx()),
  fStartY(part->Vy()),
  fStartZ(part->Vz()),
  fStartT(part->T()*1e09),
  fNPoints(0),
  fNPointsMap(),
  fEdepMap(),
  fLengthMap()
{
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
STMCTrack::~STMCTrack() { }
// -------------------------------------------------------------------------



// -----   Public method Print   -------------------------------------------
void STMCTrack::Print(Int_t trackId) const
{
  LOG(DEBUG) << "Track " << trackId << ", mother : " << fMotherId << ", Type "
    << fPdgCode << ", momentum (" << fPx << ", " << fPy << ", "
    << fPz << ") GeV" << FairLogger::endl;
  LOG(DEBUG2) << "       Ref " << GetNPoints(kREF)
    << ", TutDet " << GetNPoints(kTutDet)
    << ", Rutherford " << GetNPoints(kSPiRIT)
    << FairLogger::endl;
}
// -------------------------------------------------------------------------



// -----   Public method GetMass   -----------------------------------------
Double_t STMCTrack::GetMass() const
{
  if ( TDatabasePDG::Instance() ) {
    TParticlePDG* particle = TDatabasePDG::Instance()->GetParticle(fPdgCode);
    if ( particle ) { return particle->Mass(); }
    else if( GetZ()==1 && GetN()==1 ) { return 1.875613; }
    else if( GetZ()==1 && GetN()==2 ) { return 2.808921; }
    else if( GetZ()==2 && GetN()==1 ) { return 2.808391; }
    else if( GetZ()==2 && GetN()==2 ) { return 3.727379; }
    else if( GetA()>4 ) { return 0.931494*(double)GetA(); }
    else { return 0.; }
  }
  return 0.;
}
// -------------------------------------------------------------------------




// -----   Public method GetRapidity   -------------------------------------
Double_t STMCTrack::GetRapidity() const
{
  Double_t e = GetEnergy();
  Double_t y = 0.5 * TMath::Log( (e+fPz) / (e-fPz) );
  return y;
}
// -------------------------------------------------------------------------




// -----   Public method GetNPoints   --------------------------------------
Int_t STMCTrack::GetNPoints(DetectorId detId) const
{
  // TODO: Where does this come from
  if      ( detId == kREF  ) { return (  fNPoints &   1); }
  else if ( detId == kTutDet  ) { return ( (fNPoints & ( 7 <<  1) ) >>  1); }
  else if ( detId == kSPiRIT ) { return ( (fNPoints & (31 <<  4) ) >>  4); }
  else {
    LOG(ERROR) << "Unknown detector ID "
      << detId << FairLogger::endl;
    return 0;
  }
}
// -------------------------------------------------------------------------


Int_t STMCTrack::GetZ() const
{
  if( fPdgCode==2212 ) return 1;
  else if( fPdgCode>3000 ) return (fPdgCode%10000000)/10000;
  else return 0;
}

Int_t STMCTrack::GetN() const
{
  if( fPdgCode==2112 ) return 1;
  else if( fPdgCode>3000 ) return (fPdgCode%10000)/10 - GetZ();
  else return 0;
}

Int_t STMCTrack::GetNPointsFromMap(Int_t detID)
{
  if( fNPointsMap.find(detID) != end(fNPointsMap) ) return fNPointsMap[detID];
  else return 0;
}

Double_t STMCTrack::GetEdepFromMap(Int_t detID)
{
  if( fEdepMap.find(detID) != end(fEdepMap) ) return fEdepMap[detID];
  else return 0.;
}

Double_t STMCTrack::GetLengthFromMap(Int_t detID)
{
  if( fLengthMap.find(detID) != end(fLengthMap) ) return fLengthMap[detID];
  else return 0.;
}



// -----   Public method SetNPoints   --------------------------------------
void STMCTrack::SetNPoints(Int_t iDet, Int_t nPoints)
{

  if ( iDet == kREF ) {
    if      ( nPoints < 0 ) { nPoints = 0; }
    else if ( nPoints > 1 ) { nPoints = 1; }
    fNPoints = ( fNPoints & ( ~ 1 ) )  |  nPoints;
  }

  else if ( iDet == kTutDet ) {
    if      ( nPoints < 0 ) { nPoints = 0; }
    else if ( nPoints > 7 ) { nPoints = 7; }
    fNPoints = ( fNPoints & ( ~ (  7 <<  1 ) ) )  |  ( nPoints <<  1 );
  }

  else if ( iDet == kSPiRIT ) {
    if      ( nPoints <  0 ) { nPoints =  0; }
    else if ( nPoints > 31 ) { nPoints = 31; }
    fNPoints = ( fNPoints & ( ~ ( 31 <<  4 ) ) )  |  ( nPoints <<  4 );
  }

  else LOG(ERROR) << "Unknown detector ID "
    << iDet << FairLogger::endl;

}
// -------------------------------------------------------------------------

ClassImp(STMCTrack)
