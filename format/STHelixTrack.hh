#ifndef STHELIXTRACK
#define STHELIXTRACK

#include "STHit.hh"
#include <vector>

class STHelixTrack : public TObject
{
  public:
    STHelixTrack();
    STHelixTrack(Int_t id);

    enum STFitStatus { kNon, kHelix, kLine };

    void Clear(Option_t *option = "");
    virtual void Print(Option_t *option="") const;

    void AddHit(STHit *hit);
    void DeleteHits();

    void SetFitStatus(STFitStatus value);
    void SetStatusToHelix();
    void SetStatusToLine();

    void SetHelixCenter(Double_t x, Double_t z);
    void SetHelixRadius(Double_t r);
    void SetDipAngle(Double_t dip);
    void SetYInitial(Double_t y);

    void SetLineDirection(Double_t x, Double_t y, Double_t z);

    STFitStatus GetFitStatus();

    Double_t GetHelixCenterX();
    Double_t GetHelixCenterZ();
    Double_t GetHelixRadius();
    Double_t GetDipAngle();
    Double_t GetYInitial();

    void GetHelixParameters(Double_t &xCenter, 
                            Double_t &zCenter, 
                            Double_t &radius, 
                            Double_t &dipAngle,
                            Double_t &yInitial);

    /*
    void GetLineParameters(Double_t &xDirection, 
                           Double_t &yDirection, 
                           Double_t &zDirection,
                           Double_t &xVertex,
                           Double_t &yVertex,
                           Double_t &zVertex);
    */

    Double_t GetChargeSum();
    Double_t GetXMean();
    Double_t GetYMean();
    Double_t GetZMean();
    Double_t GetXCov();
    Double_t GetZCov();

    Int_t GetNumHits() const;
    std::vector<STHit *> *GetHitArray();

  private:
    Int_t fTrackID;
    Int_t fParentID;

    STFitStatus fFitStatus;

    Double_t fXHelixCenter;  ///< x-component of the helix center (mm)
    Double_t fZHelixCenter;  ///< z-component of the helix center (mm)
    Double_t fHelixRadius;   ///< Radius of the helix (mm)
    Double_t fDipAngle;      ///< Dip angle (radian), angle becomes 0 on xz plane
    Double_t fYInitial;      ///< y-position at angle phi = 0

    Double_t fChargeSum;
    Double_t fXMean;
    Double_t fYMean;
    Double_t fZMean;
    Double_t fXCov;
    Double_t fZCov;

    std::vector<STHit *> fHitArray;  ///< Hit array

  ClassDef(STHelixTrack, 1)
};

#endif
