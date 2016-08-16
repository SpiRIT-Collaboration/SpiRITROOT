#ifndef STMCRECOMATCHING
#define STMCRECOMATCHING

#include "TObject.h"
#include "TVector3.h"

class STMCRecoMatching : public TObject
{
  public:
    STMCRecoMatching();
    virtual ~STMCRecoMatching() {}

    enum STMatchingStatus { kNotSet, kMatch, kNotFound, kFake };

    void Init(Int_t mcid, Int_t recoid, Int_t mcpdg, TVector3 mcp, TVector3 recop);

    void SetMCID(Int_t val);
    void SetRecoID(Int_t val);
    void SetMCPDGCode(Int_t val);
    void SetMCMomentum(TVector3 p);
    void SetRecoMomentum(TVector3 p);

    void SetIsMatched();
    void SetIsNotFound();
    void SetIsFake();
    void SetStatus(STMatchingStatus val);

    Int_t GetMCID();
    Int_t GetRecoID();
    Int_t GetMCPDGCode();
    TVector3 GetMCMomentum();
    TVector3 GetRecoMomentum();

    bool IsMatched();  ///< MC and Reco tracks are matched
    bool IsNotFound(); ///< Only MCID is set. Not found in reconstruction
    bool IsFake();     ///< Only RecoID is set. Fake track
    STMatchingStatus GetStatus();

  private:
    STMatchingStatus fStatus; ///< Matching status

    Int_t fMCID;             ///< position in array of STMCTrack 
    Int_t fRecoID;           ///< position in array of STTrack 
    Int_t fMCPDGCode;        ///< PDG code in STMCTrack
    TVector3 fMCMomentum;    ///< Original momentum
    TVector3 fRecoMomentum;  ///< Reconstructed momentum


  ClassDef(STMCRecoMatching, 1)
};

#endif
