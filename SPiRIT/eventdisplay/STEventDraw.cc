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
#include "TGraph.h"
#include "TCanvas.h"
#include "TPaletteAxis.h"
#include "TStyle.h"
#include "TVirtualX.h"
#include "TGWindow.h"

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

  fIs2DPlot = kFALSE;
  fIs2DPlotRange = kFALSE;
  fIs2DPlotExternal = kFALSE;
  fPadPlaneCvs = NULL;
  fPadPlane = NULL;
  fMinZ = 0;
  fMaxZ = 1344;
  fMinX = 432;
  fMaxX = -432;

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

  fIs2DPlot = kFALSE;
  fIs2DPlotRange = kFALSE;
  fIs2DPlotExternal = kFALSE;
  fPadPlaneCvs = NULL;
  fPadPlane = NULL;
  fMinZ = 0;
  fMaxZ = 1344;
  fMinX = 432;
  fMaxX = -432;

  fLogger = FairLogger::GetLogger();
}

STEventDraw::~STEventDraw()
{
}

void STEventDraw::SetVerbose(Int_t verbose)       { fVerbose = verbose; }
void STEventDraw::Set2DPlot(Bool_t value)         { fIs2DPlot = value; }
void STEventDraw::Set2DPlotExternal(Bool_t value) { fIs2DPlotExternal = value; }

void
STEventDraw::Set2DPlotRange(Int_t uaIdx)
{
  if (uaIdx < 0 || uaIdx > 47) {
    fLogger -> Error(MESSAGE_ORIGIN, "2DPlotRange should be [0, 47]!");

    return;
  }

  fMinZ = (uaIdx%4)*12*7*4;
  fMaxZ = (uaIdx%4 + 1)*12*7*4;
  fMinX = (uaIdx/4)*8*9 - 432;
  fMaxX = (uaIdx/4 + 1)*8*9 - 432;

  fIs2DPlotRange = kTRUE;
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

  if (fIs2DPlot) {
    gStyle -> SetPalette(55);
    if (fIs2DPlotExternal) {
      UInt_t width = 0, height = 0;
      Int_t dummy = 0;
      gVirtualX -> GetWindowSize(gClient -> GetRoot() -> GetId(), dummy, dummy, width, height);

      if (fIs2DPlotRange) {
        Double_t ratio = 270./1300.;
        if (width < 1300) {
          width = 0.9*width;
          height = ratio*width;
        } else {
          width = 1300;
          height = 270;
        }

        fPadPlaneCvs = new TCanvas("padplane", "Pad Plane View", width, height);
        fPadPlaneCvs -> SetLeftMargin(0.06);
        fPadPlaneCvs -> SetRightMargin(0.065);
      } else {
        Double_t ratio = 800./1300.;
      
        if (width < 1300) {
          width = 0.9*width;
          height = width*ratio;
        } else if (height < 800) {
          height = 0.8*height;
          width = height/ratio;
        } else {
          width = 1300;
          height = 800;
        }

        fPadPlaneCvs = new TCanvas("padplane", "Pad Plane View", width, height);
      }
      fPadPlaneCvs -> SetFixedAspectRatio();
    } else
      fPadPlaneCvs = gEve -> AddCanvasTab("Pad Plane View");
    DrawPadPlane();
  }

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

      if (fIs2DPlot)
        fPadPlane -> Fill(vec.Z(), vec.X(), aPoint.GetCharge());
    }

    gEve -> AddElement(pointSet);
    gEve -> Redraw3D(kFALSE);

    if (fIs2DPlot) {
      fPadPlaneCvs -> Modified();
      fPadPlaneCvs -> Update();

      TPaletteAxis *paxis = (TPaletteAxis *) fPadPlane -> GetListOfFunctions() -> FindObject("palette");
      if (paxis) {
        if (fIs2DPlotExternal && fIs2DPlotRange) {
          paxis -> SetX1NDC(0.940);
          paxis -> SetX2NDC(0.955);
          paxis -> SetLabelSize(0.08);
          paxis -> GetAxis() -> SetTickSize(0.01);
        } else {
          paxis -> SetX1NDC(0.905);
          paxis -> SetX2NDC(0.94);
        }

        fPadPlaneCvs -> Modified();
        fPadPlaneCvs -> Update();
      }
    }

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

  fPadPlane -> Reset();
}

void
STEventDraw::DrawPadPlane()
{
  if (fPadPlane != NULL) {
    fPadPlane -> Reset();

    return;
  }

  fPadPlaneCvs -> cd();
  fPadPlane = new TH2D("padplane", "SpiRIT Pad Plane", 112, 0, 1344, 108, -432, 432);
  fPadPlane -> GetXaxis() -> SetTickLength(0.01);
  fPadPlane -> GetXaxis() -> SetTitle("z (mm)");
  fPadPlane -> GetXaxis() -> CenterTitle();
  fPadPlane -> GetYaxis() -> SetTickLength(0.01);
  fPadPlane -> GetYaxis() -> SetTitle("x (mm)");
  fPadPlane -> GetYaxis() -> CenterTitle();
  fPadPlane -> SetMinimum(0);
  fPadPlane -> SetMaximum(4095);
  if (fIs2DPlotExternal && fIs2DPlotRange) {
    fPadPlane -> GetYaxis() -> SetTitleOffset(0.35);
    fPadPlane -> GetYaxis() -> SetTitleSize(0.08);
    fPadPlane -> GetXaxis() -> SetTitleOffset(0.40);
    fPadPlane -> GetXaxis() -> SetTitleSize(0.08);
    fPadPlane -> SetLabelSize(0.08, "X");
    fPadPlane -> SetLabelSize(0.08, "Y");
    fPadPlane -> GetXaxis() -> SetRangeUser(fMinZ, fMaxZ);
    fPadPlane -> GetYaxis() -> SetRangeUser(fMinX, fMaxX);
  }
  fPadPlane -> SetStats(0);
  fPadPlane -> Draw("colz");

  for (Int_t i = 0; i < 107; i++) {
    Int_t x[2] = {-432 + (i + 1)*8, -432 + (i + 1)*8};
    Int_t z[2] = {0, 1344};
    TGraph *line = new TGraph(2, z, x);
    line -> SetMarkerStyle(1);
    if ((i + 1)%9 == 0)
        line -> SetLineStyle(1);
    else
        line -> SetLineStyle(3);
    line -> Draw("same");
  }

  for (Int_t i = 0; i < 111; i++) {
    Int_t x[2] = {-432, 432};
    Int_t z[2] = {(i + 1)*12, (i + 1)*12};
    TGraph *line = new TGraph(2, z, x);
    line -> SetMarkerStyle(1);
    if ((i + 1)%7 == 0)
        line -> SetLineStyle(1);
    else
        line -> SetLineStyle(3);
    line -> Draw("same");
  }
}
