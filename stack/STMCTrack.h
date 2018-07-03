// -------------------------------------------------------------------------
// -----                      STMCTrack header file                    -----
// -----                  Created 03/08/04  by V. Friese               -----
// -----               Redesigned 13/12/19  by G. Jhang                -----
// -------------------------------------------------------------------------


/** STMCTrack.h
 *@author V.Friese <v.friese@gsi.de>
 **
 ** Data class for storing Monte Carlo tracks processed by the FairStack.
 ** A MCTrack can be a primary track put into the simulation or a
 ** secondary one produced by the transport through decay or interaction.
 **
 ** Redesign 13/06/07 by V. Friese
 ** Redesign 13/12/19 by G. Jhang
 **/


#ifndef STMCTRACK_H
#define STMCTRACK_H 1

#include "TObject.h"                    // for TObject

#include "STDetectorList.h"             // for DetectorId

#include "Rtypes.h"                     // for Double_t, Int_t, Double32_t, etc
#include "TLorentzVector.h"             // for TLorentzVector
#include "TMath.h"                      // for Sqrt
#include "TVector3.h"                   // for TVector3

#include <map>

class TParticle;

class STMCTrack : public TObject
{

  public:


    /**  Default constructor  **/
    STMCTrack();


    /**  Standard constructor  **/
    STMCTrack(Int_t pdgCode, Int_t motherID, Int_t trackID,
                Double_t px, Double_t py, Double_t pz,
		Double_t x, Double_t y, Double_t z,
                Double_t t, Int_t nPoints);

    /**  Copy constructor  **/
    STMCTrack(const STMCTrack& track);


    /**  Constructor from TParticle  **/
    STMCTrack(TParticle* particle);


    /**  Destructor  **/
    virtual ~STMCTrack();


    /**  Output to screen  **/
    void Print(Int_t iTrack=0) const;


    /**  Accessors  **/
    Int_t    GetPdgCode()  const { return fPdgCode; }
    Int_t    GetMotherId() const { return fMotherId; }
    Int_t    GetTrackId()  const { return fTrackId; }
    Double_t GetPx()       const { return fPx; }
    Double_t GetPy()       const { return fPy; }
    Double_t GetPz()       const { return fPz; }
    Double_t GetStartX()   const { return fStartX; }
    Double_t GetStartY()   const { return fStartY; }
    Double_t GetStartZ()   const { return fStartZ; }
    Double_t GetStartT()   const { return fStartT; }
    Double_t GetMass()     const;
    Double_t GetEnergy()   const;
    Double_t GetPt()       const { return TMath::Sqrt(fPx*fPx+fPy*fPy); }
    Double_t GetP() const { return TMath::Sqrt(fPx*fPx+fPy*fPy+fPz*fPz); }
    Double_t GetRapidity() const;
    void GetMomentum(TVector3& momentum);
    void Get4Momentum(TLorentzVector& momentum);
    void GetStartVertex(TVector3& vertex);


    /** Accessors to the number of MCPoints in the detectors **/
    Int_t GetNPoints(DetectorId detId)  const;


    /** Accessors to atomic information if the track is nucleus. **/
    Int_t GetZ() const;
    Int_t GetN() const;
    Int_t GetA() const { return GetZ()+GetN(); }

    Int_t GetNPointsFromMap(Int_t detID);
    Double_t GetEdepFromMap(Int_t detID);
    Double_t GetLengthFromMap(Int_t detID);


    /**  Modifiers  **/
    void SetMotherId(Int_t id) { fMotherId = id; }
    void SetNPoints(Int_t iDet, Int_t np);

    void SetTrackId(Int_t id)      { fTrackId = id; }
    void SetMomentum(TVector3 mom) { fPx = mom.X(); fPy = mom.Y(); fPz = mom.Z(); }

    void SetPointMap(Int_t detID, Int_t np)         { fNPointsMap[detID] = np; }
    void SetEdepMap(Int_t detID, Double_t edep)     { fEdepMap[detID] = edep; }
    void SetLengthMap(Int_t detID, Double_t length) { fLengthMap[detID] = length; }


  private:

    /**  PDG particle code  **/
    Int_t  fPdgCode;

    /**  Index of mother track. -1 for primary particles.  **/
    Int_t  fMotherId;
    
    /**  Track index in VMC. **/
    Int_t  fTrackId;

    /** Momentum components at start vertex [GeV]  **/
    Double32_t fPx, fPy, fPz;

    /** Coordinates of start vertex [cm, ns]  **/
    Double32_t fStartX, fStartY, fStartZ, fStartT;

    /**  Bitvector representing the number of MCPoints for this track in
     **  each subdetector. The detectors are represented by
     **  REF:         Bit  0      (1 bit,  max. value  1)
     **  MVD:         Bit  1 -  3 (3 bits, max. value  7)
     **  STS:         Bit  4 -  8 (5 bits, max. value 31)
     **  RICH:        Bit  9      (1 bit,  max. value  1)
     **  MUCH:        Bit 10 - 14 (5 bits, max. value 31)
     **  TRD:         Bit 15 - 19 (5 bits, max. value 31)
     **  TOF:         Bit 20 - 23 (4 bits, max. value 15)
     **  ECAL:        Bit 24      (1 bit,  max. value  1)
     **  ZDC:         Bit 25      (1 bit,  max. value  1)
     **  The respective point numbers can be accessed and modified
     **  with the inline functions.
     **  Bits 26-31 are spare for potential additional detectors.
     **/
    Int_t fNPoints;

    std::map<Int_t, Int_t>    fNPointsMap;  // array of pair< detectorID, # of steps >
    std::map<Int_t, Double_t> fEdepMap;     // array of pair< detectorID, sum of dE >
    std::map<Int_t, Double_t> fLengthMap;   // array of pair< detectorID, sum of length >


    ClassDef(STMCTrack,3);

};



// ==========   Inline functions   ========================================

inline Double_t STMCTrack::GetEnergy() const
{
  Double_t mass = GetMass();
  return TMath::Sqrt(mass*mass + fPx*fPx + fPy*fPy + fPz*fPz );
}


inline void STMCTrack::GetMomentum(TVector3& momentum)
{
  momentum.SetXYZ(fPx,fPy,fPz);
}


inline void STMCTrack::Get4Momentum(TLorentzVector& momentum)
{
  momentum.SetXYZT(fPx,fPy,fPz,GetEnergy());
}


inline void STMCTrack::GetStartVertex(TVector3& vertex)
{
  vertex.SetXYZ(fStartX,fStartY,fStartZ);
}

#endif
