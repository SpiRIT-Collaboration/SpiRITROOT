#ifndef STEXTRAPOLATEDTRACK_HH
#define STEXTRAPOLATEDTRACK_HH

#include "TVector3.h"
#include "TObject.h"
#include <vector>

using namespace std;

class STExtrapolatedTrack : public TObject
{
  public:
    STExtrapolatedTrack();
    virtual ~STExtrapolatedTrack() {};

    virtual void Clear(Option_t *option = "");
    virtual void Print(Option_t *option="") const;

    void SetParentID(Int_t id);
    void SetTrackID(Int_t id);
    void AddPoint(TVector3 point, Double_t length = -1);

    Int_t GetParentID() const;
    Int_t GetTrackID() const;
    vector<TVector3> *GetPoints();
    vector<Double_t> *GetLengths(); 

    Int_t GetNumPoint() const;

    TVector3 ExtrapolateByLength(Double_t length) const;

  private:
    Int_t fParentID;
    Int_t fTrackID;

    vector<TVector3> fPoints;
    vector<Double_t> fLengths;

  ClassDef(STExtrapolatedTrack, 1)
};

#endif
