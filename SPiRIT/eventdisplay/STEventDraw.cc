// =================================================
//  STEventDraw Class
//
//  Description:
//    Drawing hits and tracks in STEvent container
//
//  Genie Jhang ( geniejhang@majimak.com )
//  2014. 07. 07
// ================================================= 

// SpiRITROOT classes
#include "STEventDraw.hh"
#include "STEvent.hh"

// FairRoot classes
#include "FairRootManager.h"

// ROOT classes
#include "TEveManager.h"
#include "TEveTreeTools.h"
#include "TNamed.h"
#include "TString.h"

ClassImp(STEventDraw);

STEventDraw::STEventDraw()
:FairTask("STEventDraw", 0)
{
  fVerbose = 0;
  fEventArray = NULL;
  fEventManager = NULL;
  fPointSet = NULL;
  fColor = 0;
  fStyle = 0;
  fLogger = FairLogger::GetLogger();
}

STEventDraw::STEventDraw(const Char_t *name, Color_t color, Style_t style, Int_t verbose)
:FairTask(name, verbose)
{
  fVerbose = verbose;
  fEventArray = NULL;
  fEventManager = NULL;
  fPointSet = NULL;
  fColor = color;
  fStyle = style;
  fLogger = FairLogger::GetLogger();
}

STEventDraw::~STEventDraw()
{
}

void
STEventDraw::SetVerbose(Int_t verbose)
{
  fVerbose = verbose;
}

InitStatus STEventDraw::Init()
{
  if (fVerbose > 1) 
    fLogger -> Info(MESSAGE_ORIGIN, "");

  FairRootManager *ioMan = FairRootManager::Instance();
  fEventArray = (TClonesArray *) ioMan -> GetObject(GetName());

  if (fEventArray == 0) {
    fLogger -> Info(MESSAGE_ORIGIN, Form("Branch %s not found! Task will be deactivated!", GetName()));

    SetActive(kFALSE);
  }

  if (fVerbose > 2)
    fLogger -> Info(MESSAGE_ORIGIN, Form("Get track list ", fEventArray));

  fEventManager = FairEventManager::Instance();
  if (fVerbose > 2)
    fLogger -> Info(MESSAGE_ORIGIN, Form("Get instance of FairEventManager!"));

  fPointSet = 0;

  // gEve->AddElement(fPointSet, fEventManager );
  return kSUCCESS;
}

void STEventDraw::Exec(Option_t* option)
{
  if (IsActive()) {
    Reset();

    STEvent *aEvent = (STEvent *) fEventArray -> At(0);
    Int_t numPoints = aEvent -> GetNumHits();
    TEvePointSet* pointSet = new TEvePointSet(GetName(), numPoints, TEvePointSelectorConsumer::kTVT_XYZ);
    pointSet -> SetOwnIds(kTRUE);
    pointSet -> SetMarkerColor(fColor);
    pointSet -> SetMarkerSize(1.5);
    pointSet -> SetMarkerStyle(fStyle);

    std::vector<STHit> *pointArray = aEvent -> GetHitArray();
    for (Int_t iPoint = 0; iPoint < numPoints; iPoint++) {
      STHit aPoint = pointArray -> at(iPoint);
      TVector3 vec(GetVector(aPoint));
      pointSet -> SetNextPoint(vec.X()/10., vec.Y()/10., vec.Z()/10.); // mm -> cm
      pointSet -> SetPointId(GetValue(aPoint, iPoint));
    }

    gEve -> AddElement(pointSet);
    gEve -> Redraw3D(kFALSE);

    fPointSet = pointSet;
  }
}

TObject* STEventDraw::GetValue(STHit &hit, Int_t iHit)
{
  return new TNamed(Form("Hit %d", iHit), "");
}


void
STEventDraw::Reset()
{
  if(fPointSet != 0) {
    fPointSet -> Reset();
    gEve -> RemoveElement(fPointSet, fEventManager);
  }
}
