#ifndef _STMCPOINT_H_
#define _STMCPOINT_H_ 1


#include "FairMCPoint.h"

#include "TObject.h"
#include "TVector3.h"

class STMCPoint : public FairMCPoint
{
   public:

      /** Default constructor **/
      STMCPoint();

      /** Copy constructor **/
      STMCPoint(const STMCPoint& point);
      STMCPoint& operator=(const STMCPoint& point);

      /** Constructor with arguments
       *@param trackID  Index of MCTrack
       *@param detID    Detector ID
       *@param pos      Ccoordinates at entrance to active volume [cm]
       *@param mom      Momentum of track at entrance [GeV]
       *@param tof      Time since event start [ns]
       *@param length   Track length since creation [cm]
       *@param eLoss    Energy deposit [GeV]
       **/
      STMCPoint(Int_t trackID, Int_t detID, TVector3 pos, TVector3 mom,
	    Double_t tof, Double_t length, Double_t eLoss, Int_t pdg);


      /** Destructor **/
      virtual ~STMCPoint();

      /** Output to screen **/
      virtual void Print(const Option_t* opt) const;

      Int_t GetPDG()	{ return fPdg; }

   private:
      Int_t fPdg;

      ClassDef(STMCPoint,1)

};

#endif
