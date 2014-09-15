//---------------------------------------------------------------------
// Description:
//      Primary Cluster data class header
//
//      Data class for primary clusters for STClusterizerTask.
//      Primary cluster is cluster of electrons produced from 
//      STMCPoint.
//
//      Created by : STClusterizerTask
//      Used    by : STDriftTask
//
// Author List:
//      JungWoo Lee     Korea Univ.       (original author)
//
//----------------------------------------------------------------------

#ifndef _STPRIMARYCLUSTER_H_
#define _STPRIMARYCLUSTER_H_

// ROOT class headers
#include "TObject.h"
#include "TVector3.h"

class STPrimaryCluster : public TObject
{
  public :

    /** Default constructor **/
    STPrimaryCluster();

    STPrimaryCluster(UInt_t   charge,
                     TVector3 position,
                     Double_t time,
                     UInt_t   trackID,
                     Int_t    hitID);

    /** Default destructor **/
    ~STPrimaryCluster();

    //Getters
    UInt_t   GetCharge();
    TVector3 GetPosition();
    Double_t GetPositionX();
    Double_t GetPositionY();
    Double_t GetPositionZ();
    Double_t GetTime();
    UInt_t   GetTrackID();
    Int_t    GetHitID();

    //Setters
    void SetIndex(Int_t index);
    
  private :
    UInt_t   fCharge;    ///  charge [eV]
    TVector3 fPosition;  ///  position [cm]
    Double_t fTime;      ///  time from MC [ns]
    UInt_t   fTrackID;   /// original MC trackID
    Int_t    fHitID;     ///  original MC hitID

    Int_t    fIndex;     ///  position of STPrimaryCluster in "TClonesArray"

    ClassDef(STPrimaryCluster,1)
};

#endif
