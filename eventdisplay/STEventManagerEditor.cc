#include "TEveManager.h"
#include "STEventManagerEditor.hh"
#include "STEventManager.hh"
#include "STEventDrawTask.hh"

#include "FairRootManager.h"
#include "FairRunAna.h"

#include "TChain.h"
#include "TEveGValuators.h"
#include "TFile.h"
#include "TGButton.h"
#include "TGLabel.h"
#include "TGLayout.h"
#include "TGNumberEntry.h"
#include "TGeoManager.h"
#include "TString.h"

#include <iostream>
#include <stddef.h>
#include <iostream>

class TGWindow;
class TObject;

using namespace std;

ClassImp(STEventManagerEditor)

STEventManagerEditor::STEventManagerEditor
(const TGWindow* p, Int_t width, Int_t height, UInt_t options, Pixel_t back)
: TGedFrame(p, width, height, options | kVerticalFrame, back),
  fObject(0),
  fManager(STEventManager::Instance()),
  fDrawTask(0),
  fCurrentEvent(0),
  fCurrentRiemannSet(0),
  fCurrentRow(0),
  fCurrentLayer(0),
  fTempRiemannSet(0),
  fEventTime(NULL),
  fAutoUpdateFlag(kFALSE),
  fAutoUpdatePadFlag(kFALSE)
{
  fDrawTask = STEventDrawTask::Instance();
  fDrawTask -> SetEventManagerEditor(this);
  Init();
}

void 
STEventManagerEditor::Init()
{
  FairRootManager* fRootManager=FairRootManager::Instance();
  TChain* chain = fRootManager -> GetInChain();
  Int_t Entries= chain -> GetEntriesFast();

  MakeTitle("STEventManager Editor");
  TGVerticalFrame* fEventFrame = CreateEditorTabSubFrame("Event");
  TGCompositeFrame* title = new TGCompositeFrame(fEventFrame, 230, 10,
      kVerticalFrame | kLHintsExpandX |
      kFixedWidth    | kOwnBackground);

  /********************************************************************/

  TGGroupFrame* frameInfo = new TGGroupFrame(title,"Info",kVerticalFrame);
  frameInfo -> SetTitlePos(TGGroupFrame::kCenter);

  TFile* file = FairRootManager::Instance() -> GetInChain() -> GetFile();
  UInt_t RunId= FairRunAna::Instance() -> getRunId();

  TGLabel* labelFileName = new TGLabel(frameInfo, TString("Input file : ")+file->GetName());
  TGLabel* labelRunID    = new TGLabel(frameInfo, TString("Run Id : ")+Form("%d",RunId));
  TGLabel* labelEventID  = new TGLabel(frameInfo, TString("No. of events : ")+Form("%d",Entries));

  frameInfo -> AddFrame(labelFileName,new TGLayoutHints (kLHintsLeft));
  frameInfo -> AddFrame(labelRunID ,new TGLayoutHints (kLHintsLeft));
  frameInfo -> AddFrame(labelEventID);

  title -> AddFrame(frameInfo, new TGLayoutHints(kLHintsRight | kLHintsExpandX));

  /********************************************************************/

  TGGroupFrame* frameEvent = new TGGroupFrame(title,"Event",kVerticalFrame);
  frameEvent -> SetTitlePos(TGGroupFrame::kLeft);

  TGCheckButton* checkAutoUpdate = new TGCheckButton(frameEvent, "Auto Update");
  checkAutoUpdate -> Connect("Toggled(Bool_t)", "STEventManagerEditor", this, "ToggleAutoUpdate(Bool_t)");
  checkAutoUpdate -> Toggle(kTRUE);

  TGHorizontalFrame* frameEvent1 = new TGHorizontalFrame(frameEvent);
  TGLabel* labelEvent = new TGLabel(frameEvent1, "Current Event : ");
  fCurrentEvent = new TGNumberEntry(frameEvent1, 0., 6, -1,
                                    TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative,
                                    TGNumberFormat::kNELLimitMinMax, 0, Entries);
  fCurrentEvent -> Connect("ValueSet(Long_t)","STEventManagerEditor", this, "SelectEventIf()");
  frameEvent1 -> AddFrame(labelEvent, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 1, 2, 1, 1));
  frameEvent1 -> AddFrame(fCurrentEvent, new TGLayoutHints(kLHintsLeft, 1, 1, 1, 1));

  TGTextButton* buttonUpdate = new TGTextButton(frameEvent, "Update");
  buttonUpdate -> Connect("Clicked()", "STEventManagerEditor", this, "SelectEvent()");

  frameEvent -> AddFrame(checkAutoUpdate, new TGLayoutHints(kLHintsLeft, 1,1,5,3));
  frameEvent -> AddFrame(frameEvent1, new TGLayoutHints(kLHintsLeft, 1,1,3,3));
  frameEvent -> AddFrame(buttonUpdate, new TGLayoutHints(kLHintsRight | kLHintsExpandX, 5,5,5,3));

  title -> AddFrame(frameEvent, new TGLayoutHints(kLHintsRight | kLHintsExpandX));

  /********************************************************************/

  TGGroupFrame* frameRiemann = new TGGroupFrame(title,"Riemann Tracklet");
  frameRiemann -> SetTitlePos(TGGroupFrame::kLeft);

  TGHorizontalFrame* frameRiemann1 = new TGHorizontalFrame(frameRiemann);
  TGLabel* labelTracklet = new TGLabel(frameRiemann1, "Select Tracklet : ");
  fCurrentRiemannSet = new TGNumberEntry(frameRiemann1, 0., 6, -1,
                                         TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative,
                                         TGNumberFormat::kNELLimitMinMax, 0, GetNRiemannSet()-1);
  fCurrentRiemannSet -> Connect("ValueSet(Long_t)", "STEventManagerEditor", this, "SelectRiemannSet()");
  frameRiemann1 -> AddFrame(labelTracklet, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 1, 2, 1, 1));
  frameRiemann1 -> AddFrame(fCurrentRiemannSet, new TGLayoutHints(kLHintsLeft, 1, 1, 1, 1));

  /*
  TGHorizontalFrame* frameRiemann2 = new TGHorizontalFrame(frameRiemann);
  TGLabel* labelAddTracklet = new TGLabel(frameRiemann2, "Add : ");
  fTempRiemannSet = new TGNumberEntry(frameRiemann2, 0., 6, -1,
                                      TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative,
                                      TGNumberFormat::kNELLimitMinMax, 0, GetNRiemannSet()-1);
  fTempRiemannSet -> Connect("ValueSet(Long_t)", "STEventManagerEditor", this, "");
  TGTextButton* buttonAddRiemannSet = new TGTextButton(frameRiemann2, "Add");
  buttonAddRiemannSet -> Connect("Clicked()", "STEventManagerEditor", this, "AddRiemannSet()");
  frameRiemann2 -> AddFrame(labelAddTracklet, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 1, 2, 1, 1));
  frameRiemann2 -> AddFrame(fTempRiemannSet, new TGLayoutHints(kLHintsLeft, 1, 1, 1, 1));
  frameRiemann2 -> AddFrame(buttonAddRiemannSet, new TGLayoutHints(kLHintsLeft, 1, 1, 1, 1));
  */

  TGTextButton* buttonVisAll = new TGTextButton(frameRiemann, "Visualize All");
  buttonVisAll -> Connect("Clicked()","STEventManagerEditor", this, "VisAllRiemannSet()");
  TGTextButton* buttonVisOff = new TGTextButton(frameRiemann, "Unvisualize All");
  buttonVisOff -> Connect("Clicked()","STEventManagerEditor", this, "VisOffRiemannSet()");

  frameRiemann -> AddFrame(frameRiemann1, new TGLayoutHints(kLHintsLeft, 1,1,5,3));
  //frameRiemann -> AddFrame(frameRiemann2, new TGLayoutHints(kLHintsLeft, 1,1,3,3));
  frameRiemann -> AddFrame(buttonVisAll, new TGLayoutHints(kLHintsRight | kLHintsExpandX, 5,5,5,2));
  frameRiemann -> AddFrame(buttonVisOff, new TGLayoutHints(kLHintsRight | kLHintsExpandX, 5,5,2,3));

  title -> AddFrame(frameRiemann, new TGLayoutHints(kLHintsRight | kLHintsExpandX));

  /********************************************************************/

  TGGroupFrame* framePad = new TGGroupFrame(title,"Pad");
  framePad -> SetTitlePos(TGGroupFrame::kLeft);

  TGCheckButton* checkAutoUpdatePad = new TGCheckButton(framePad, "Auto Update");
  checkAutoUpdatePad -> Connect("Toggled(Bool_t)", "STEventManagerEditor", this, "ToggleAutoUpdatePad(Bool_t)");
  checkAutoUpdatePad -> Toggle(kFALSE);

  TGHorizontalFrame* framePadRow = new TGHorizontalFrame(framePad);
  TGLabel* labelRow   = new TGLabel(framePadRow, "Row   :");
  fCurrentRow = new TGNumberEntry(framePadRow, 0., 6, -1,
                                  TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative,
                                  TGNumberFormat::kNELLimitMinMax, 0, 107);
  fCurrentRow -> Connect("ValueSet(Long_t)", "STEventManagerEditor", this, "SelectPadIf()");
  framePadRow -> AddFrame(labelRow, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 1, 2, 1, 1));
  framePadRow -> AddFrame(fCurrentRow, new TGLayoutHints(kLHintsLeft, 1, 1, 1, 1));

  TGHorizontalFrame* framePadLayer = new TGHorizontalFrame(framePad);
  TGLabel* labelLayer = new TGLabel(framePadLayer, "Layer :");
  fCurrentLayer = new TGNumberEntry(framePadLayer, 0., 6, -1,
                                    TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative,
                                    TGNumberFormat::kNELLimitMinMax, 0, 111);
  fCurrentLayer -> Connect("ValueSet(Long_t)", "STEventManagerEditor", this, "SelectPadIf()");
  framePadLayer -> AddFrame(labelLayer, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 1, 2, 1, 1));
  framePadLayer -> AddFrame(fCurrentLayer, new TGLayoutHints(kLHintsLeft, 1, 1, 1, 1));

  TGTextButton* buttonUpdatePad = new TGTextButton(framePad, "Update");
  buttonUpdatePad -> Connect("Clicked()", "STEventManagerEditor", this, "SelectPad()");

  framePad -> AddFrame(checkAutoUpdatePad, new TGLayoutHints(kLHintsLeft, 1,1,5,3));
  framePad -> AddFrame(framePadRow, new TGLayoutHints(kLHintsLeft, 1,1,3,3));
  framePad -> AddFrame(framePadLayer, new TGLayoutHints(kLHintsLeft, 1,1,3,3));
  framePad -> AddFrame(buttonUpdatePad, new TGLayoutHints(kLHintsRight | kLHintsExpandX, 5,5,5,3));

  title -> AddFrame(framePad, new TGLayoutHints(kLHintsRight | kLHintsExpandX));

  /********************************************************************/

  fEventFrame -> AddFrame(title, new TGLayoutHints(kLHintsTop, 0, 0, 2, 0));
}

void
STEventManagerEditor::ToggleAutoUpdatePad(Bool_t onoff)
{
  fAutoUpdatePadFlag = onoff;
}

void 
STEventManagerEditor::SelectPadIf()
{
  if(fAutoUpdatePadFlag) SelectPad();
}

void 
STEventManagerEditor::SelectPad()
{
  fDrawTask -> DrawPad(fCurrentRow->GetIntNumber(), fCurrentLayer->GetIntNumber());
}

void 
STEventManagerEditor::SelectEventIf()
{
  if(fAutoUpdateFlag) SelectEvent();
}

void 
STEventManagerEditor::SelectEvent()
{
  fManager -> GotoEvent(fCurrentEvent->GetIntNumber());
  if(fCurrentRiemannSet)
    fCurrentRiemannSet -> SetLimitValues(0,GetNRiemannSet()-1);
  if(fTempRiemannSet)
    fTempRiemannSet -> SetLimitValues(0,GetNRiemannSet()-1);

  Update();
}

void 
STEventManagerEditor::SetModel(TObject* obj)
{
  fObject = obj;
}

void
STEventManagerEditor::ToggleAutoUpdate(Bool_t onoff)
{
  fAutoUpdateFlag = onoff;
}

Int_t
STEventManagerEditor::GetNRiemannSet()
{
  return fDrawTask -> GetNRiemannSet();
}

void 
STEventManagerEditor::SelectRiemannSet()
{
  fDrawTask -> SetSelfRiemannSet(fCurrentRiemannSet->GetIntNumber(),kTRUE);
  gEve -> Redraw3D();
}

void
STEventManagerEditor::AddRiemannSet()
{
  fDrawTask -> SetSelfRiemannSet(fTempRiemannSet->GetIntNumber(),kFALSE);
  gEve -> Redraw3D();
}

void
STEventManagerEditor::VisAllRiemannSet()
{
  fDrawTask -> SetSelfRiemannSet(-1,kFALSE);
  gEve -> Redraw3D();
}

void
STEventManagerEditor::VisOffRiemannSet()
{
  fDrawTask -> SetSelfRiemannSet(-1,kTRUE);
  gEve -> Redraw3D();
}

void
STEventManagerEditor::SetRowLayer(Int_t row, Int_t layer)
{
  fCurrentRow -> SetNumber(row);
  fCurrentLayer -> SetNumber(layer);
}
