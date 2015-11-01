#include "TEveManager.h"
#include "STEventManagerEditor.hh"
#include "STEventManager.hh"
#include "STEventDrawTask.hh"

#include "FairRootManager.h"
#include "FairRun.h"

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
  fCurrentRow(0),
  fCurrentLayer(0),
  fCurrentRiemannSet(0),
  fTempRiemannSet(0),
  fCurrentLinearSet(0),
  fTempLinearSet(0),
  fEventTime(NULL),
  fAutoUpdateFlag(kFALSE),
  fAutoUpdatePadFlag(kFALSE)
{
  fDrawTask = STEventDrawTask::Instance();
  fDrawTask -> SetEventManagerEditor(this);
  fManager -> SetEventManagerEditor(this);

  fLogger = FairLogger::GetLogger();

  fOnline = fManager -> Online();
  fOnlineEditor = fManager -> OnlineEditor();

  Init();
}

void 
STEventManagerEditor::Init()
{
  fLogger -> Info(MESSAGE_ORIGIN, "STEventManagerEditor Init().");

  if (fOnline)
    fEntries = 999999;
  else 
  {
    FairRootManager* fRootManager = FairRootManager::Instance();
    TChain* chain = fRootManager -> GetInChain();
    fEntries = chain -> GetEntriesFast();
  }

  MakeTitle("STEventManager Editor");
  fEditorTabSubFrame = CreateEditorTabSubFrame("Event");

  //Choose only one, between below two lines
  //FillFrameContent(fEditorTabSubFrame);
  fManager -> InitByEditor();

  fLogger -> Debug(MESSAGE_ORIGIN, "STEventManagerEditor End of Init().");
}

void 
STEventManagerEditor::FillFrameContent(TGCompositeFrame* frame)
{
  TGCompositeFrame* eventFrame = new TGCompositeFrame(frame, 230, 10,
      kVerticalFrame | kLHintsExpandX |
      kFixedWidth    | kOwnBackground);

  /********************************************************************/

  fLogger -> Info(MESSAGE_ORIGIN, "Making Info frame.");

  TGGroupFrame* frameInfo = new TGGroupFrame(eventFrame,"Info",kVerticalFrame);
  frameInfo -> SetTitlePos(TGGroupFrame::kLeft);

  TString name;

  if (fManager -> Online())
    name = ((STSource*) FairRunOnline::Instance() -> GetSource()) -> GetDataFileName();
  else {
    TFile* file = FairRootManager::Instance() -> GetInChain() -> GetFile();
    //UInt_t RunId = FairRunAna::Instance() -> GetRunId();
    name = file -> GetName();
  }

  TObjString *last = (TObjString*) name.Tokenize("/") -> Last();
  name = last -> GetString();

  TGLabel* labelFileName = new TGLabel(frameInfo, name);
  //TGLabel* labelRunID    = new TGLabel(frameInfo, TString("Run Id : ") + Form("%d",RunId));
  TGLabel* labelEventID  = new TGLabel(frameInfo, TString("No. of events : ") + Form("%d", fEntries));

  frameInfo -> AddFrame(labelFileName,new TGLayoutHints (kLHintsLeft));
  //frameInfo -> AddFrame(labelRunID ,new TGLayoutHints (kLHintsLeft));
  frameInfo -> AddFrame(labelEventID);

  eventFrame -> AddFrame(frameInfo, new TGLayoutHints(kLHintsRight | kLHintsExpandX));

  /********************************************************************/

  fLogger -> Info(MESSAGE_ORIGIN, "Making event update frame.");

  TGGroupFrame* frameEvent = new TGGroupFrame(eventFrame,"Event",kVerticalFrame);
  frameEvent -> SetTitlePos(TGGroupFrame::kLeft);

  TGCheckButton* checkAutoUpdate;
  if (fOnline == kFALSE)
  {
    checkAutoUpdate = new TGCheckButton(frameEvent, "Auto Update");
    checkAutoUpdate -> Connect("Toggled(Bool_t)", "STEventManagerEditor", this, "ToggleAutoUpdate(Bool_t)");
    checkAutoUpdate -> Toggle(kTRUE);
  }

  TGHorizontalFrame* frameEvent1 = new TGHorizontalFrame(frameEvent);
  TGLabel* labelEvent = new TGLabel(frameEvent1, "Current Event : ");
  Int_t eventMin = 0;
  if (fOnline == kTRUE)
    eventMin = -1;

  fCurrentEvent = new TGNumberEntry(frameEvent1, 0., 6, -1,
                                    TGNumberFormat::kNESInteger, TGNumberFormat::kNEAAnyNumber,
                                    TGNumberFormat::kNELLimitMinMax, eventMin, fEntries - 1);
  fCurrentEvent -> Connect("ValueSet(Long_t)","STEventManagerEditor", this, "SelectEventIf()");
  frameEvent1 -> AddFrame(labelEvent, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 1, 2, 1, 1));
  frameEvent1 -> AddFrame(fCurrentEvent, new TGLayoutHints(kLHintsLeft, 1, 1, 1, 1));

  TGTextButton* buttonNextEvent = new TGTextButton(frameEvent, "Next");
  buttonNextEvent -> Connect("Clicked()", "STEventManagerEditor", this, "NextEvent()");

  TGTextButton* buttonBeforeEvent;
  if (fOnline == kFALSE)
  {
    buttonBeforeEvent = new TGTextButton(frameEvent, "Before");
    buttonBeforeEvent -> Connect("Clicked()", "STEventManagerEditor", this, "BeforeEvent()");
  }

  TGTextButton* buttonUpdate = new TGTextButton(frameEvent, "Update");
  buttonUpdate -> Connect("Clicked()", "STEventManagerEditor", this, "SelectEvent()");

  if (fOnline == kFALSE)
    frameEvent -> AddFrame(checkAutoUpdate, new TGLayoutHints(kLHintsLeft, 1,1,5,3));
  frameEvent -> AddFrame(frameEvent1, new TGLayoutHints(kLHintsLeft, 1,1,3,3));
  frameEvent -> AddFrame(buttonNextEvent, new TGLayoutHints(kLHintsRight | kLHintsExpandX, 5,5,5,1));
  if (fOnline == kFALSE)
    frameEvent -> AddFrame(buttonBeforeEvent, new TGLayoutHints(kLHintsRight | kLHintsExpandX, 5,5,1,3));
  frameEvent -> AddFrame(buttonUpdate, new TGLayoutHints(kLHintsRight | kLHintsExpandX, 5,5,5,3));

  eventFrame -> AddFrame(frameEvent, new TGLayoutHints(kLHintsRight | kLHintsExpandX));

  /********************************************************************/

  fLogger -> Info(MESSAGE_ORIGIN, "Making riemann control frame.");

  TGGroupFrame* frameRiemann = new TGGroupFrame(eventFrame,"Riemann Tracklet");
  frameRiemann -> SetTitlePos(TGGroupFrame::kLeft);

  TGHorizontalFrame* frameRiemann1 = new TGHorizontalFrame(frameRiemann);
  TGLabel* labelTracklet = new TGLabel(frameRiemann1, "Select Tracklet : ");
  fCurrentRiemannSet = new TGNumberEntry(frameRiemann1, 0., 6, -1,
                                         TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative,
                                         TGNumberFormat::kNELLimitMinMax, 0, GetNRiemannSet()-1);
  fCurrentRiemannSet -> Connect("ValueSet(Long_t)", "STEventManagerEditor", this, "SelectRiemannSet()");
  frameRiemann1 -> AddFrame(labelTracklet, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 1, 2, 1, 1));
  frameRiemann1 -> AddFrame(fCurrentRiemannSet, new TGLayoutHints(kLHintsLeft, 1, 1, 1, 1));

  TGTextButton* buttonVisAll = new TGTextButton(frameRiemann, "Visualize All");
  buttonVisAll -> Connect("Clicked()","STEventManagerEditor", this, "VisAllRiemannSet()");
  TGTextButton* buttonVisOff = new TGTextButton(frameRiemann, "Unvisualize All");
  buttonVisOff -> Connect("Clicked()","STEventManagerEditor", this, "VisOffRiemannSet()");

  frameRiemann -> AddFrame(frameRiemann1, new TGLayoutHints(kLHintsLeft, 1,1,5,3));
  frameRiemann -> AddFrame(buttonVisAll, new TGLayoutHints(kLHintsRight | kLHintsExpandX, 5,5,5,2));
  frameRiemann -> AddFrame(buttonVisOff, new TGLayoutHints(kLHintsRight | kLHintsExpandX, 5,5,2,3));

  if (fOnlineEditor == kFALSE)
    eventFrame -> AddFrame(frameRiemann, new TGLayoutHints(kLHintsRight | kLHintsExpandX));

  /********************************************************************/

  fLogger -> Info(MESSAGE_ORIGIN, "Making linear control frame.");

  TGGroupFrame* frameLinear = new TGGroupFrame(eventFrame,"Linear Tracklet");
  frameLinear -> SetTitlePos(TGGroupFrame::kLeft);

  TGHorizontalFrame* frameLinear1 = new TGHorizontalFrame(frameLinear);
  TGLabel* labelLinearTracklet = new TGLabel(frameLinear1, "Select Tracklet : ");
  fCurrentLinearSet = new TGNumberEntry(frameLinear1, 0., 6, -1,
                                         TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative,
                                         TGNumberFormat::kNELLimitMinMax, 0, fDrawTask -> GetNLinearSet() - 1);
  fCurrentLinearSet -> Connect("ValueSet(Long_t)", "STEventManagerEditor", this, "SelectLinearSet()");
  frameLinear1 -> AddFrame(labelLinearTracklet, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 1, 2, 1, 1));
  frameLinear1 -> AddFrame(fCurrentLinearSet, new TGLayoutHints(kLHintsLeft, 1, 1, 1, 1));

  TGTextButton* buttonVisAllLinear = new TGTextButton(frameLinear, "Visualize All");
  buttonVisAllLinear -> Connect("Clicked()","STEventManagerEditor", this, "VisAllLinearSet()");
  TGTextButton* buttonVisOffLinear = new TGTextButton(frameLinear, "Unvisualize All");
  buttonVisOffLinear -> Connect("Clicked()","STEventManagerEditor", this, "VisOffLinearSet()");

  frameLinear -> AddFrame(frameLinear1, new TGLayoutHints(kLHintsLeft, 1,1,5,3));
  frameLinear -> AddFrame(buttonVisAllLinear, new TGLayoutHints(kLHintsRight | kLHintsExpandX, 5,5,5,2));
  frameLinear -> AddFrame(buttonVisOffLinear, new TGLayoutHints(kLHintsRight | kLHintsExpandX, 5,5,2,3));

  eventFrame -> AddFrame(frameLinear, new TGLayoutHints(kLHintsRight | kLHintsExpandX));

  /********************************************************************/

  fLogger -> Info(MESSAGE_ORIGIN, "Making pad control frame.");

  TGGroupFrame* framePad = new TGGroupFrame(eventFrame,"Pad");
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

  eventFrame -> AddFrame(framePad, new TGLayoutHints(kLHintsRight | kLHintsExpandX));

  /********************************************************************/

  fLogger -> Info(MESSAGE_ORIGIN, "Making time bucket window control frame.");

  TGGroupFrame* frameWindowTb = new TGGroupFrame(eventFrame,"Time Bucket Window");
  frameWindowTb -> SetTitlePos(TGGroupFrame::kLeft);

  TGHorizontalFrame* frameWindowTbStart = new TGHorizontalFrame(frameWindowTb);
  TGLabel* labelWindowTbStart = new TGLabel(frameWindowTbStart, "start :");
  fWindowTbStartDefault = fDrawTask -> GetWindowTbStart();
  fCurrentWindowTbStart = new TGNumberEntry(frameWindowTbStart, fWindowTbStartDefault, 6, -1,
                                  TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative,
                                  TGNumberFormat::kNELLimitMinMax, 0, 512);
  frameWindowTbStart -> AddFrame(labelWindowTbStart, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 1, 2, 1, 1));
  frameWindowTbStart -> AddFrame(fCurrentWindowTbStart, new TGLayoutHints(kLHintsLeft, 1, 1, 1, 1));

  TGHorizontalFrame* frameWindowTbEnd = new TGHorizontalFrame(frameWindowTb);
  TGLabel* labelWindowTbEnd = new TGLabel(frameWindowTbEnd, "end  :");
  fWindowTbEndDefault = fDrawTask -> GetWindowTbEnd();
  fCurrentWindowTbEnd = new TGNumberEntry(frameWindowTbEnd, fWindowTbEndDefault, 6, -1,
                                  TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative,
                                  TGNumberFormat::kNELLimitMinMax, 1, 512);
  frameWindowTbEnd -> AddFrame(labelWindowTbEnd, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 1, 2, 1, 1));
  frameWindowTbEnd -> AddFrame(fCurrentWindowTbEnd, new TGLayoutHints(kLHintsLeft, 1, 1, 1, 1));

  TGTextButton* buttonUpdateWindowTb = new TGTextButton(frameWindowTb, "Update");
  buttonUpdateWindowTb -> Connect("Clicked()", "STEventManagerEditor", this, "UpdateWindowTb()");

  TGTextButton* buttonDefaultWindowTb = new TGTextButton(frameWindowTb, "Reset to default");
  buttonDefaultWindowTb -> Connect("Clicked()", "STEventManagerEditor", this, "DefaultWindowTb()");

  TGTextButton* buttonResetWindowTb = new TGTextButton(frameWindowTb, "Reset (0,512)");
  buttonResetWindowTb -> Connect("Clicked()", "STEventManagerEditor", this, "ResetWindowTb()");

  frameWindowTb -> AddFrame(frameWindowTbStart, new TGLayoutHints(kLHintsLeft, 1,1,5,3));
  frameWindowTb -> AddFrame(frameWindowTbEnd, new TGLayoutHints(kLHintsLeft, 1,1,3,3));
  frameWindowTb -> AddFrame(buttonUpdateWindowTb, new TGLayoutHints(kLHintsRight | kLHintsExpandX, 5,5,5,3));
  frameWindowTb -> AddFrame(buttonDefaultWindowTb, new TGLayoutHints(kLHintsRight | kLHintsExpandX, 5,5,3,1));
  frameWindowTb -> AddFrame(buttonResetWindowTb, new TGLayoutHints(kLHintsRight | kLHintsExpandX, 5,5,1,3));

  eventFrame -> AddFrame(frameWindowTb, new TGLayoutHints(kLHintsRight | kLHintsExpandX));

  /********************************************************************/

  frame -> AddFrame(eventFrame, new TGLayoutHints(kLHintsTop, 0, 0, 2, 0));
}

void
STEventManagerEditor::ToggleAutoUpdatePad(Bool_t onoff)
{
  fAutoUpdatePadFlag = onoff;
}

void 
STEventManagerEditor::UpdateWindowTb()
{
  fDrawTask -> SetWindowRange(fCurrentWindowTbStart -> GetIntNumber(), fCurrentWindowTbEnd -> GetIntNumber());
  fManager -> UpdateEvent();
}

void 
STEventManagerEditor::ResetWindowTb()
{
  fDrawTask -> SetWindow();
  fCurrentWindowTbStart -> SetNumber(fDrawTask -> GetWindowTbStart());
  fCurrentWindowTbEnd -> SetNumber(fDrawTask -> GetWindowTbEnd());
  fManager -> UpdateEvent();
}

void 
STEventManagerEditor::DefaultWindowTb()
{
  fCurrentWindowTbStart -> SetNumber(fWindowTbStartDefault);
  fCurrentWindowTbEnd -> SetNumber(fWindowTbEndDefault);
  fDrawTask -> SetWindowRange(fCurrentWindowTbStart -> GetIntNumber(), fCurrentWindowTbEnd -> GetIntNumber());
  fManager -> UpdateEvent();
}

void 
STEventManagerEditor::SelectPadIf()
{
  if(fAutoUpdatePadFlag) SelectPad();
}

void 
STEventManagerEditor::SelectPad()
{
  fDrawTask -> DrawPad(fCurrentRow -> GetIntNumber(), fCurrentLayer -> GetIntNumber());
}

void 
STEventManagerEditor::SelectEventIf()
{
  if(fAutoUpdateFlag) SelectEvent();
}

void 
STEventManagerEditor::SelectEvent()
{
  fManager -> RunEvent(fCurrentEvent -> GetIntNumber());

  if (fOnline == kTRUE)
    fCurrentEvent -> SetLimitValues(fCurrentEvent -> GetIntNumber(), fEntries - 1);

  if(fCurrentRiemannSet)
    fCurrentRiemannSet -> SetLimitValues(0,GetNRiemannSet()-1);
  if(fTempRiemannSet)
    fTempRiemannSet -> SetLimitValues(0,GetNRiemannSet()-1);

  if(fCurrentLinearSet)
    fCurrentLinearSet -> SetLimitValues(0, fDrawTask -> GetNLinearSet() - 1);
  if(fTempLinearSet)
    fTempLinearSet -> SetLimitValues(0, fDrawTask -> GetNLinearSet() - 1);

  fDrawTask -> UpdatePadRange();

  Update();
}

void 
STEventManagerEditor::NextEvent()
{
  Int_t eventID = fCurrentEvent -> GetIntNumber();

  if (fOnline == kTRUE)
  {
    fCurrentEvent -> SetNumber(-1);
    SelectEvent();
    fCurrentEvent -> SetNumber(((STSource*) FairRunOnline::Instance() -> GetSource()) -> GetEventID());
  }
  else if (eventID != fEntries - 1)
  {
    Int_t updatedEventID = eventID + 1;
    fCurrentEvent -> SetNumber(updatedEventID);

    SelectEvent();
  }
}

void 
STEventManagerEditor::BeforeEvent()
{
  Int_t eventID = fCurrentEvent -> GetIntNumber();
  if (eventID != 0)
  {
    Int_t updatedEventID = eventID - 1;
    fCurrentEvent -> SetNumber(updatedEventID);

    SelectEvent();
  }
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
STEventManagerEditor::SelectLinearSet()
{
  fDrawTask -> SetSelfLinearSet(fCurrentLinearSet->GetIntNumber(),kTRUE);
  gEve -> Redraw3D();
}

void
STEventManagerEditor::AddLinearSet()
{
  fDrawTask -> SetSelfLinearSet(fTempLinearSet->GetIntNumber(),kFALSE);
  gEve -> Redraw3D();
}

void
STEventManagerEditor::VisAllLinearSet()
{
  fDrawTask -> SetSelfLinearSet(-1,kFALSE);
  gEve -> Redraw3D();
}

void
STEventManagerEditor::VisOffLinearSet()
{
  fDrawTask -> SetSelfLinearSet(-1,kTRUE);
  gEve -> Redraw3D();
}

void
STEventManagerEditor::SetRowLayer(Int_t row, Int_t layer)
{
  fCurrentRow -> SetNumber(row);
  fCurrentLayer -> SetNumber(layer);
}

TGVerticalFrame* STEventManagerEditor::GetEditorTabSubFrame() { return fEditorTabSubFrame; }
