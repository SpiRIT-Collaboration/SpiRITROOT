#ifndef SPIRITPOINT_H
#define SPIRITPOINT_H 1


#include "FairMCPoint.h"

#include "TObject.h"
#include "TVector3.h"

class SPiRITPoint : public FairMCPoint
{

  public:

    /** Default constructor **/
    SPiRITPoint();


    /** Constructor with arguments
     *@param trackID  Index of MCTrack
     *@param detID    Detector ID
     *@param pos      Ccoordinates at entrance to active volume [cm]
     *@param mom      Momentum of track at entrance [GeV]
     *@param tof      Time since event start [ns]
     *@param length   Track length since creation [cm]
     *@param eLoss    Energy deposit [GeV]
     **/
    SPiRITPoint(Int_t trackID, Int_t detID, TVector3 pos, TVector3 mom,
                     Double_t tof, Double_t length, Double_t eLoss);




    /** Destructor **/
    virtual ~SPiRITPoint();

    /** Output to screen **/
    virtual void Print(const Option_t* opt) const;

  private:
    /** Copy constructor **/
    SPiRITPoint(const SPiRITPoint& point);
    SPiRITPoint operator=(const SPiRITPoint& point);

    ClassDef(SPiRITPoint,1)

};

#endif
