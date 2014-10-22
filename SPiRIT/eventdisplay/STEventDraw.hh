// =================================================
//  STEventDraw Class
//
//  Description:
//    Drawing hits and tracks in STEvent container
//
//  Genie Jhang ( geniejhang@majimak.com )
//  2014. 07. 07
// ================================================= 

#ifndef STEVENTDRAW_H
#define STEVENTDRAW_H

// SpiRITROOT classes
#include "STHit.hh"
#include "STHitCluster.hh"

// FairRoot classes
#include "FairTask.h"
#include "FairEventManager.h"
#include "FairLogger.h"

// ROOT classes
#include "TEvePointSet.h"
#include "TClonesArray.h"
#include "TVector3.h"
#include "Rtypes.h"
#include "TH2D.h"

class TObject;

class STEventDraw : public FairTask
{
  public:
    //! Constructor
    STEventDraw();

    STEventDraw(const Char_t *name, Color_t color, Style_t style, Int_t verbose);

    //! Destructor
    virtual ~STEventDraw();

    //! Set verbosity level. For this task and all of the subtasks.
    void SetVerbose(Int_t verbose);
    //! 
    void Set2DPlot(Bool_t value = kTRUE);
    void Set2DPlotExternal(Bool_t value = kTRUE);
    //!
    void Set2DPlotRange(Int_t uaIdx);

    ///
    void SetThreshold(Int_t value);

    virtual InitStatus Init();
    virtual void Exec(Option_t* option);

    void Reset();

  protected:
    virtual TVector3 GetVector(STHit &hit) = 0;
    virtual TVector3 GetVector(STHitCluster &cluster) = 0;
    TObject *GetValue(STHit &hit, Int_t iHit);
    TObject *GetValue(STHitCluster &cluster, Int_t iCluster);

    FairLogger *fLogger;              //!<
    Int_t fVerbose;                   //!<  Verbosity level
    TClonesArray* fEventArray;        //!<
    FairEventManager* fEventManager;  //!<
    TEvePointSet* fPointSet;          //!<
    Color_t fColor;                   //!<
    Style_t fStyle;                   //!<

    Bool_t fIs2DPlot;                 //!<
    Bool_t fIs2DPlotExternal;         //!<
    Bool_t fIs2DPlotRange;            //!<
    TCanvas *fPadPlaneCvs;            //!<
    TH2D *fPadPlane;                  //!<
    Int_t fMinZ;                      //!<
    Int_t fMaxZ;                      //!<
    Int_t fMinX;                      //!<
    Int_t fMaxX;                      //!<
    Int_t fThreshold;                 ///

  private:
    STEventDraw(const STEventDraw&);
    STEventDraw& operator=(const STEventDraw&);

    void DrawPadPlane();

    ClassDef(STEventDraw,1);
};

#endif
