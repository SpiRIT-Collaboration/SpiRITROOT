/*
 * @breif SPiRIT Event display manager
 * @author JungWoo Lee (Korea Univ.)
 */

#include "STEveManager.hh"

#include "FairParAsciiFileIo.h"

#include "TEveGeoNode.h"
#include "TEveManager.h"
#include "TEveProjectionManager.h"
#include "TEveWindowManager.h"
#include "TEveScene.h"
#include "TEveViewer.h"
#include "TEveWindow.h"
#include "TEveBrowser.h"
#include "TEveGedEditor.h"
#include "TRootEmbeddedCanvas.h"
#include "TObjArray.h"
#include "TGString.h"
#include "TGedEditor.h"
#include "TGTab.h"
#include "TGLViewer.h"
#include "TGeoManager.h"
#include "TVirtualX.h"
#include "TGWindow.h"

#include <iostream>

class TGeoNode;

using namespace std;

ClassImp(STEveManager);

STEveManager* STEveManager::fInstance = 0;
STEveManager* STEveManager::Instance() { return fInstance; }
STEveManager::STEveManager()
: fMainTask(new FairTask("MainTask")), fEveTask(new STEveTask("EveTask"))
{
  fLogger = FairLogger::GetLogger();

  fEveMode = kOverview;

  fCurrentEventEntry = -1;

  fCvsPadPlane = NULL;
  fCvsPadADC   = NULL;

  fCurrentEventNumberEntry = NULL;
  fCurrentWindowTbStart    = NULL;
  fCurrentWindowTbEnd      = NULL;
  fCurrentRiemannSet       = NULL;
  fTempRiemannSet          = NULL;
  fCurrentLinearSet        = NULL;
  fTempLinearSet           = NULL;
  fCurrentRow              = NULL;
  fCurrentLayer            = NULL;
  fEventTime               = NULL;

  fGeomFileName = "";
  fTransparency = 80;
  fClearColor   = kWhite;
  fHRotate      = -0.7;
  fVRotate      = 1.1;

  fInstance = this;

  fRun = FairRunAna::Instance();
  if (fRun == NULL)
    fRun = new FairRunAna();
}

void STEveManager::SetInputFile(TString name)    { fRun -> SetInputFile(name); }
void STEveManager::AddFriend(TString name)       { fRun -> AddFriend(name); }
void STEveManager::SetOutputFile(TString name)   { fRun -> SetOutputFile(name); }
void STEveManager::SetGeomFile(TString name)     { fRun -> SetGeomFile(name); }
void STEveManager::SetParInputFile(TString name) 
{
  FairParAsciiFileIo* digiPar = new FairParAsciiFileIo();
  digiPar -> open(name);
  FairRuntimeDb* fDb = fRun -> GetRuntimeDb();
  fDb -> setSecondInput(digiPar);
}

void STEveManager::AddTask(FairTask* task)    { fMainTask -> Add(task); }
void STEveManager::AddEveTask(FairTask* task) 
{ 
  fMainTask -> Add(task); 
  fEveTask  -> Add(task); 
}

void STEveManager::UpdateEvent() { fEveTask -> ExecuteTask(""); }
void STEveManager::NextEvent()   { RunEvent(fCurrentEventEntry + 1); }
void STEveManager::PrevEvent()   { RunEvent(fCurrentEventEntry - 1); }
void STEveManager::RunEvent(Long64_t entry) 
{
  fRun -> Run(entry);
  fCurrentEventEntry = entry;
}

void STEveManager::SetEveMode(STEveMode mode)        { fEveMode      = mode; }
void STEveManager::SetGeomFileManual(TString name)   { fGeomFileName = name; }
void STEveManager::SetVolumeTransparency(Int_t val)  { fTransparency = val; }
void STEveManager::SetBackgroundColor(Color_t color) { fClearColor   = color; }
void STEveManager::SetViewerPoint(Double_t hRotate, Double_t vRotate) 
{
  fHRotate = hRotate;
  fVRotate = vRotate;
}
void STEveManager::SetNumRiemannSet(Int_t num)   { fNumRiemannSet = num; }
void STEveManager::SetNumLinearSet(Int_t num)    { fNumLinearSet  = num; }
void STEveManager::SetRowLayer(Int_t row, Int_t layer)
{
  fCurrentRow -> SetNumber(row);
  fCurrentLayer -> SetNumber(layer);
}


STEveManager::STEveMode STEveManager::GetEveMode() { return fEveMode; }
TCanvas*  STEveManager::GetCvsPadPlane()           { return fCvsPadPlane; }
TCanvas*  STEveManager::GetCvsPad()                { return fCvsPadADC; }
TCanvas*  STEveManager::GetCvsPadADC()             { return fCvsPadADC; }
Long64_t  STEveManager::GetCurrentEventEntry()     { return fCurrentEventEntry; }

void 
STEveManager::Init(Int_t option, Int_t level, Int_t nNodes)
{
  fLogger -> Debug(MESSAGE_ORIGIN, "Initializing eve.");

  fRun -> SetTask(fMainTask);

  BuildFrame();

  fRun -> Init(); // Initialize after the canvases are created!

  FairRuntimeDb* rtdb = fRun -> GetRuntimeDb();
  STDigiPar* par = (STDigiPar*) rtdb -> getContainer("STDigiPar");

  fWindowTbStartDefault = par -> GetWindowStartTb();
  fWindowTbEndDefault   = par -> GetWindowNumTbs() + fWindowTbStartDefault;
  fWindowTbStart = fWindowTbStartDefault;
  fWindowTbEnd   = fWindowTbEndDefault;

  if (fCurrentWindowTbStart) fCurrentWindowTbStart -> SetNumber(fWindowTbStart);
  if (fCurrentWindowTbEnd)   fCurrentWindowTbEnd   -> SetNumber(fWindowTbEnd);

  BuildMenu();
  BuildGeometry(option, level, nNodes);
  Build3DViewer();

  gEve -> GetBrowser() -> HideBottomTab();
  gEve -> ElementSelect(gEve -> GetCurrentEvent());
  gEve -> GetWindowManager() -> HideAllEveDecorations();
}

void 
STEveManager::BuildFrame()
{
  fLogger -> Debug(MESSAGE_ORIGIN, "Building TEve frame.");

  if (fEveMode == kAll) 
    TEveManager::Create(kTRUE, "V");

  else if (fEveMode == kOverview || fEveMode == k3D)
    TEveManager::Create(kTRUE, "");

  gEve -> AddEvent(this);

  Int_t dummy;
  UInt_t width, height;
  UInt_t widthMax  = 1400;
  UInt_t heightMax = 800;
  Double_t ratio = (Double_t)widthMax/heightMax;

  gVirtualX -> GetWindowSize(gClient -> GetRoot() -> GetId(), 
                             dummy, dummy, 
                             width, height);

  if (width > widthMax ) { 
    width  = widthMax; 
    height = heightMax; 
  } 
  else 
    height = (Int_t)(width/ratio);

  gEve -> GetMainWindow() -> Resize(width,height);

  if (fEveMode == kOverview || fEveMode == kAll)
  {
    TEveWindowSlot* slotOverview = NULL;
    TEveWindowSlot* slot3D       = NULL;
    TEveWindowPack* packOverview = NULL;
    TEveWindowPack* packLeft     = NULL;
    TEveWindowSlot* slotPadPlane = NULL;
    TEveWindowSlot* slotPadADC   = NULL;

    TRootEmbeddedCanvas* ecvsPadPlane = new TRootEmbeddedCanvas();
    TRootEmbeddedCanvas* ecvsPadADC   = new TRootEmbeddedCanvas();

    TEveWindowFrame* framePadPlane = NULL;
    TEveWindowFrame* framePadADC   = NULL;

    gEve -> GetBrowser() -> SetTabTitle("Full 3D", TRootBrowser::kRight);

    slotOverview = TEveWindow::CreateWindowInTab(gEve -> GetBrowser() -> GetTabRight());
    slotOverview -> SetShowTitleBar(kFALSE);
    slotOverview -> SetElementName("Overview");
    packOverview = slotOverview -> MakePack();
    packOverview -> SetShowTitleBar(kFALSE);
    packOverview -> SetElementName("Overview");
    packOverview -> SetHorizontal();

    packLeft = packOverview -> NewSlot() -> MakePack();
    packLeft -> SetShowTitleBar(kFALSE);
    packLeft -> SetElementName("left pack");
    packLeft -> SetVertical();

    slot3D = packLeft -> NewSlot();
    slot3D -> SetShowTitleBar(kFALSE);
    slot3D -> MakeCurrent();

    TEveViewer* viewer3D = gEve -> SpawnNewViewer("3D View", "");
    viewer3D -> SetShowTitleBar(kFALSE);
    viewer3D -> AddScene(gEve -> GetGlobalScene());
    viewer3D -> AddScene(gEve -> GetEventScene());

    slotPadPlane = packOverview -> NewSlot();
    slotPadPlane -> SetShowTitleBar(kFALSE);
    framePadPlane = slotPadPlane -> MakeFrame(ecvsPadPlane);
    framePadPlane -> SetElementName("SpiRIT Pad Plane");
    fCvsPadPlane = ecvsPadPlane -> GetCanvas();

    slotPadADC = packLeft -> NewSlotWithWeight(.6);
    slotPadADC -> SetShowTitleBar(kFALSE);
    framePadADC = slotPadADC -> MakeFrame(ecvsPadADC);
    framePadADC -> SetElementName("pad");
    fCvsPadADC = ecvsPadADC -> GetCanvas();
  }
  else if (fEveMode == k3D)
  {
    TEveViewer* viewer3D = gEve -> SpawnNewViewer("3D View", "");
    viewer3D -> SetShowTitleBar(kFALSE);
    viewer3D -> AddScene(gEve -> GetGlobalScene());
    viewer3D -> AddScene(gEve -> GetEventScene());
  }

}

void 
STEveManager::BuildMenu()
{
  fLogger -> Debug(MESSAGE_ORIGIN, "Building menu.");

  gEve -> GetBrowser() -> StartEmbedding(TRootBrowser::kLeft);
  TGMainFrame* frame = new TGMainFrame(gClient -> GetRoot(), 1000, 600);

  FairRootManager* fRootManager = FairRootManager::Instance();
  TChain* chain = fRootManager -> GetInChain();
  fTotalNumEntries = chain -> GetEntriesFast();

  /********************************************************************/

  TGCompositeFrame* frameMain = new TGCompositeFrame(frame, 230, 10,
      kVerticalFrame | kLHintsExpandX |
      kFixedWidth    | kOwnBackground);

  /********************************************************************/

  fLogger -> Debug(MESSAGE_ORIGIN, "Building Info frame.");

  TGGroupFrame* frameEventControl = new TGGroupFrame(frameMain,"Control",kVerticalFrame);
  frameEventControl -> SetTitlePos(TGGroupFrame::kLeft);

  TString fileName = FairRootManager::Instance() -> GetInChain() -> GetFile() -> GetName();
  TObjString *last = (TObjString*) fileName.Tokenize("/") -> Last();
  fileName = last -> GetString();

  TGLabel* labelFileName = new TGLabel(frameEventControl, fileName);
  TGLabel* labelEventID  = new TGLabel(frameEventControl, TString("No. of events : ") + Form("%d", fTotalNumEntries));

  frameEventControl -> AddFrame(labelFileName, new TGLayoutHints(kLHintsLeft, 5,5,5,1));
  frameEventControl -> AddFrame(labelEventID, new TGLayoutHints(kLHintsLeft, 5,5,1,3));

  //frameMain -> AddFrame(frameEventControl, new TGLayoutHints(kLHintsRight | kLHintsExpandX));

  /********************************************************************/

  fLogger -> Debug(MESSAGE_ORIGIN, "Building event update frame.");

  //TGGroupFrame* frameEvent = new TGGroupFrame(frameMain,"Event",kVerticalFrame);
  //frameEvent -> SetTitlePos(TGGroupFrame::kLeft);

  TGHorizontalFrame* frameEventEntry = new TGHorizontalFrame(frameEventControl);
  TGLabel* labelEvent = new TGLabel(frameEventEntry, "Current Event : ");

  fCurrentEventNumberEntry = new TGNumberEntry(frameEventEntry, 0., 6, -1,
      TGNumberFormat::kNESInteger, TGNumberFormat::kNEAAnyNumber,
      TGNumberFormat::kNELLimitMinMax, 0, fTotalNumEntries - 1);
  fCurrentEventNumberEntry -> Connect("ValueSet(Long_t)","STEveManager", this, "SelectEventButton()");

  frameEventEntry -> AddFrame(labelEvent, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 1,2,1,1));
  frameEventEntry -> AddFrame(fCurrentEventNumberEntry, new TGLayoutHints(kLHintsLeft, 1,1,1,1));

  TGTextButton* buttonNextEvent = new TGTextButton(frameEventControl, "Next");
  buttonNextEvent -> Connect("Clicked()", "STEveManager", this, "NextEventButton()");

  TGTextButton* buttonPreviousEvent = new TGTextButton(frameEventControl, "Previous");
  buttonPreviousEvent -> Connect("Clicked()", "STEveManager", this, "PrevEventButton()");

  TGTextButton* buttonUpdate = new TGTextButton(frameEventControl, "Update");
  buttonUpdate -> Connect("Clicked()", "STEveManager", this, "SelectEventButton()");

  frameEventControl -> AddFrame(frameEventEntry, new TGLayoutHints(kLHintsLeft, 1,1,3,3));
  frameEventControl -> AddFrame(buttonNextEvent, new TGLayoutHints(kLHintsRight | kLHintsExpandX, 5,5,5,1));
  frameEventControl -> AddFrame(buttonPreviousEvent, new TGLayoutHints(kLHintsRight | kLHintsExpandX, 5,5,1,3));
  frameEventControl -> AddFrame(buttonUpdate, new TGLayoutHints(kLHintsRight | kLHintsExpandX, 5,5,5,3));

  //frameMain -> AddFrame(frameEventControl, new TGLayoutHints(kLHintsRight | kLHintsExpandX));

  /********************************************************************/

  fLogger -> Debug(MESSAGE_ORIGIN, "Building Eve object control frame.");

  //TGGroupFrame* frameEveControl = new TGGroupFrame(frameMain, "On/Off");
  //frameEveControl -> SetTitlePos(TGGroupFrame::kLeft);

  TGHorizontalFrame* frameSimButtons = new TGHorizontalFrame(frameEventControl);
  {
    fButtonOnOffMC = new TGCheckButton(frameSimButtons, "MC-Hit");
    fButtonOnOffMC -> Connect("Clicked()", "STEveManager", this, "ClickOnOffMC()");
    frameSimButtons -> AddFrame(fButtonOnOffMC, new TGLayoutHints(kLHintsLeft | kLHintsExpandX, 3,3,2,2));
    if (fEveTask -> IsSetTask("mc") != 1)
      fButtonOnOffMC -> SetState(kButtonDisabled);
    else if (fEveTask -> RnrEveObjectTask("mc", 0) == 1)
      fButtonOnOffMC -> SetState(kButtonDown);

    fButtonOnOffDigi = new TGCheckButton(frameSimButtons, "Digi-Hit");
    fButtonOnOffDigi -> Connect("Clicked()", "STEveManager", this, "ClickOnOffDigi()"); 
    frameSimButtons -> AddFrame(fButtonOnOffDigi, new TGLayoutHints(kLHintsRight | kLHintsExpandX, 3,3,2,2));
    if (fEveTask -> IsSetTask("digi") != 1)
      fButtonOnOffDigi -> SetState(kButtonDisabled);
    else if (fEveTask -> RnrEveObjectTask("digi", 0) == 1)
      fButtonOnOffDigi -> SetState(kButtonDown);
  }
  frameEventControl -> AddFrame(frameSimButtons, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY | kLHintsExpandX, 1,1,10,1));

  TGHorizontalFrame* frameHitButtons = new TGHorizontalFrame(frameEventControl);
  {
    fButtonOnOffHit = new TGCheckButton(frameHitButtons, "Hit");
    fButtonOnOffHit -> Connect("Clicked()", "STEveManager", this, "ClickOnOffHit()");
    frameHitButtons -> AddFrame(fButtonOnOffHit, new TGLayoutHints(kLHintsLeft | kLHintsExpandX, 3,3,2,2));
    if (fEveTask -> IsSetTask("hit") != 1)
      fButtonOnOffHit -> SetState(kButtonDisabled);
    else if (fEveTask -> RnrEveObjectTask("hit", 0) == 1)
      fButtonOnOffHit -> SetState(kButtonDown);

    fButtonOnOffHitBox = new TGCheckButton(frameHitButtons, "Hit-Box");
    fButtonOnOffHitBox -> Connect("Clicked()", "STEveManager", this, "ClickOnOffHitBox()");
    frameHitButtons -> AddFrame(fButtonOnOffHitBox, new TGLayoutHints(kLHintsRight | kLHintsExpandX, 3,3,2,2));
    if (fEveTask -> IsSetTask("hitbox") != 1)
      fButtonOnOffHitBox -> SetState(kButtonDisabled);
    else if (fEveTask -> RnrEveObjectTask("hitbox", 0) == 1)
      fButtonOnOffHitBox -> SetState(kButtonDown);
  }
  frameEventControl -> AddFrame(frameHitButtons, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY | kLHintsExpandX, 1,1,1,1));

  TGHorizontalFrame* frameClusterButtons= new TGHorizontalFrame(frameEventControl);
  {
    fButtonOnOffCluster = new TGCheckButton(frameClusterButtons, "Cluster");
    fButtonOnOffCluster -> Connect("Clicked()", "STEveManager", this, "ClickOnOffCluster()");
    frameClusterButtons -> AddFrame(fButtonOnOffCluster, new TGLayoutHints(kLHintsLeft | kLHintsExpandX, 3,3,2,2));
    if (fEveTask -> IsSetTask("cluser") != 1)     
      fButtonOnOffCluster -> SetState(kButtonDisabled);
    else if (fEveTask -> RnrEveObjectTask("Cluster", 0) == 1)
      fButtonOnOffCluster -> SetState(kButtonDown);

    fButtonOnOffClusterBox = new TGCheckButton(frameClusterButtons, "Cluster-Box");
    fButtonOnOffClusterBox -> Connect("Clicked()", "STEveManager", this, "ClickOnOffClusterBox()");
    frameClusterButtons -> AddFrame(fButtonOnOffClusterBox, new TGLayoutHints(kLHintsRight | kLHintsExpandX, 3,3,2,2));
    if (fEveTask -> IsSetTask("clusterbox") != 1) 
      fButtonOnOffClusterBox -> SetState(kButtonDisabled);
    else if (fEveTask -> RnrEveObjectTask("ClusterBox", 0) == 1)
      fButtonOnOffClusterBox -> SetState(kButtonDown);
  }
  frameEventControl -> AddFrame(frameClusterButtons, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY | kLHintsExpandX, 1,1,1,1));

  {
    fButtonOnOffRiemannHit = new TGCheckButton(frameEventControl, "Rieimann-Hit");
    fButtonOnOffRiemannHit -> Connect("Clicked()", "STEveManager", this, "ClickOnOffRiemannHit()");
    frameEventControl -> AddFrame(fButtonOnOffRiemannHit, new TGLayoutHints(kLHintsRight | kLHintsExpandX, 5,5,2,2));
    if (fEveTask -> IsSetTask("riemannhit") != 1) 
      fButtonOnOffRiemannHit -> SetState(kButtonDisabled);
    else if (fEveTask -> RnrEveObjectTask("RiemannHit", 0) == 1)
      fButtonOnOffRiemannHit -> SetState(kButtonDown);
  }

  TGHorizontalFrame* frameLinearButtons = new TGHorizontalFrame(frameEventControl);
  {
    fButtonOnOffLinear = new TGCheckButton(frameLinearButtons, "Linear-Track");
    fButtonOnOffLinear -> Connect("Clicked()", "STEveManager", this, "ClickOnOffLinear()");
    frameLinearButtons -> AddFrame(fButtonOnOffLinear, new TGLayoutHints(kLHintsLeft | kLHintsExpandX, 3,3,2,2));
    if (fEveTask -> IsSetTask("linear") != 1)     
      fButtonOnOffLinear -> SetState(kButtonDisabled);
    else if (fEveTask -> RnrEveObjectTask("Linear", 0) == 1)
      fButtonOnOffLinear -> SetState(kButtonDown);

    fButtonOnOffLinearHit = new TGCheckButton(frameLinearButtons, "Linear-Hit");
    fButtonOnOffLinearHit -> Connect("Clicked()", "STEveManager", this, "ClickOnOffLinearHit()");
    frameLinearButtons -> AddFrame(fButtonOnOffLinearHit, new TGLayoutHints(kLHintsRight | kLHintsExpandX, 3,3,2,2));
    if (fEveTask -> IsSetTask("linearhit") != 1)  
      fButtonOnOffLinearHit  -> SetState(kButtonDisabled);
    else if (fEveTask -> RnrEveObjectTask("LinearHit", 0) == 1)
      fButtonOnOffLinearHit -> SetState(kButtonDown);
  }
  frameEventControl -> AddFrame(frameLinearButtons, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY | kLHintsExpandX, 1,1,1,1));

  frameMain -> AddFrame(frameEventControl, new TGLayoutHints(kLHintsRight | kLHintsExpandX));

  /********************************************************************/

  fLogger -> Debug(MESSAGE_ORIGIN, "Building pad control frame.");

  if (fEveMode != k3D)
  {
    TGGroupFrame* framePad = new TGGroupFrame(frameMain,"Pad");
    framePad -> SetTitlePos(TGGroupFrame::kLeft);

    TGHorizontalFrame* framePadRow = new TGHorizontalFrame(framePad);
    TGLabel* labelRow   = new TGLabel(framePadRow, "row:");
    fCurrentRow = new TGNumberEntry(framePadRow, 0., 6, -1,
                                    TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative,
                                    TGNumberFormat::kNELLimitMinMax, 0, 107);
    fCurrentRow -> Connect("ValueSet(Long_t)", "STEveManager", this, "SelectPad()");
    framePadRow -> AddFrame(labelRow, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 1, 2, 1, 1));
    framePadRow -> AddFrame(fCurrentRow, new TGLayoutHints(kLHintsLeft, 1, 1, 1, 1));

    //TGHorizontalFrame* framePadLayer = new TGHorizontalFrame(framePad);
    TGLabel* labelLayer = new TGLabel(framePadRow, "layer:");
    fCurrentLayer = new TGNumberEntry(framePadRow, 0., 6, -1,
                                      TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative,
                                      TGNumberFormat::kNELLimitMinMax, 0, 111);
    fCurrentLayer -> Connect("ValueSet(Long_t)", "STEveManager", this, "SelectPad()");
    framePadRow -> AddFrame(labelLayer, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 2, 1, 1));
    framePadRow -> AddFrame(fCurrentLayer, new TGLayoutHints(kLHintsLeft, 1, 1, 1, 1));

    TGTextButton* buttonUpdatePad = new TGTextButton(framePad, "Update");
    buttonUpdatePad -> Connect("Clicked()", "STEveManager", this, "SelectPad()");

    framePad -> AddFrame(framePadRow, new TGLayoutHints(kLHintsLeft | kLHintsExpandX, 0,0,10,3));
    //framePad -> AddFrame(framePadLayer, new TGLayoutHints(kLHintsLeft, 1,1,3,3));
    framePad -> AddFrame(buttonUpdatePad, new TGLayoutHints(kLHintsRight | kLHintsExpandX, 5,5,10,5));

    frameMain -> AddFrame(framePad, new TGLayoutHints(kLHintsRight | kLHintsExpandX));
  }

  /********************************************************************/

  fLogger -> Debug(MESSAGE_ORIGIN, "Building time bucket window control frame.");

  TGGroupFrame* frameWindowTb = new TGGroupFrame(frameMain,"Time Bucket Window");
  frameWindowTb -> SetTitlePos(TGGroupFrame::kLeft);

  TGHorizontalFrame* frameWindowTbStart = new TGHorizontalFrame(frameWindowTb);
  TGLabel* labelWindowTbStart = new TGLabel(frameWindowTbStart, "start:");
  fCurrentWindowTbStart = new TGNumberEntry(frameWindowTbStart, fWindowTbStartDefault, 6, -1,
                                  TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative,
                                  TGNumberFormat::kNELLimitMinMax, 0, 512);
  frameWindowTbStart -> AddFrame(labelWindowTbStart, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 1, 2, 1, 1));
  frameWindowTbStart -> AddFrame(fCurrentWindowTbStart, new TGLayoutHints(kLHintsLeft, 1, 1, 1, 1));

  //TGHorizontalFrame* frameWindowTbEnd = new TGHorizontalFrame(frameWindowTb);
  TGLabel* labelWindowTbEnd = new TGLabel(frameWindowTbStart, "end:");
  fCurrentWindowTbEnd = new TGNumberEntry(frameWindowTbStart, fWindowTbEndDefault, 6, -1,
                                  TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative,
                                  TGNumberFormat::kNELLimitMinMax, 1, 512);
  frameWindowTbStart -> AddFrame(labelWindowTbEnd, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 10, 2, 1, 1));
  frameWindowTbStart -> AddFrame(fCurrentWindowTbEnd, new TGLayoutHints(kLHintsLeft, 1, 1, 1, 1));

  TGTextButton* buttonUpdateWindowTb = new TGTextButton(frameWindowTb, "Update");
  buttonUpdateWindowTb -> Connect("Clicked()", "STEveManager", this, "UpdateWindowTb()");

  TGTextButton* buttonDefaultWindowTb = new TGTextButton(frameWindowTb, "Reset to default");
  buttonDefaultWindowTb -> Connect("Clicked()", "STEveManager", this, "DefaultWindowTb()");

  TGTextButton* buttonResetWindowTb = new TGTextButton(frameWindowTb, "Reset (0,512)");
  buttonResetWindowTb -> Connect("Clicked()", "STEveManager", this, "ResetWindowTb()");

  frameWindowTb -> AddFrame(frameWindowTbStart, new TGLayoutHints(kLHintsLeft | kLHintsExpandX, 0,0,10,3));
  //frameWindowTb -> AddFrame(frameWindowTbEnd, new TGLayoutHints(kLHintsLeft, 1,1,3,3));
  frameWindowTb -> AddFrame(buttonUpdateWindowTb, new TGLayoutHints(kLHintsRight | kLHintsExpandX, 5,5,10,3));
  frameWindowTb -> AddFrame(buttonDefaultWindowTb, new TGLayoutHints(kLHintsRight | kLHintsExpandX, 5,5,3,1));
  frameWindowTb -> AddFrame(buttonResetWindowTb, new TGLayoutHints(kLHintsRight | kLHintsExpandX, 5,5,1,5));

  frameMain -> AddFrame(frameWindowTb, new TGLayoutHints(kLHintsRight | kLHintsExpandX));

  frame -> AddFrame(frameMain, new TGLayoutHints(kLHintsTop, 0, 0, 2, 0));
  frame -> MapSubwindows();
  frame -> MapWindow();

  gEve -> GetBrowser() -> StopEmbedding();
  gEve -> GetBrowser() -> SetTabTitle("Event control", TRootBrowser::kLeft);
}

void 
STEveManager::BuildGeometry(Int_t option, Int_t level, Int_t nNodes)
{
  fLogger -> Debug(MESSAGE_ORIGIN, "Building geometry.");

  if (gGeoManager) 
  {
    fLogger -> Debug(MESSAGE_ORIGIN, "Setting geometry file from FairRun.");
    TGeoNode* geoNode = gGeoManager -> GetTopNode();
    TEveGeoTopNode* topNode
      = new TEveGeoTopNode(gGeoManager, geoNode, option, level, nNodes);
    gEve -> AddGlobalElement(topNode);
  }
  else
  {
    fLogger -> Debug(MESSAGE_ORIGIN, "Reading geometry file: " + fGeomFileName);
    TFile* file = new TFile(fGeomFileName, "read");
    if (file -> IsZombie())
      fLogger -> Fatal(MESSAGE_ORIGIN, "Geometry not found!");

    file -> Get("SpiRIT");
    TGeoNode* geoNode = gGeoManager -> GetTopNode();
    TEveGeoTopNode* topNode 
      = new TEveGeoTopNode(gGeoManager, geoNode, option, level, nNodes);
    gEve -> AddGlobalElement(topNode);
  }

  TObjArray* listVolume = gGeoManager -> GetListOfVolumes();
  Int_t nVolumes = listVolume -> GetEntries();
  for (Int_t i=0; i<nVolumes; i++)
    ((TGeoVolume*) listVolume -> At(i)) -> SetTransparency(fTransparency);

  gEve -> FullRedraw3D(kTRUE);
}

void
STEveManager::Build3DViewer()
{
  fLogger -> Debug(MESSAGE_ORIGIN, "Building viewer.");

  TGLViewer *dfViewer = gEve -> GetDefaultGLViewer();

  if (dfViewer == NULL)
    dfViewer = ((TEveViewer*) gEve -> GetDefaultViewer()) -> GetGLViewer();

  if (dfViewer == NULL)
    return;

  dfViewer -> SetClearColor(fClearColor);
  dfViewer -> CurrentCamera().RotateRad(fHRotate, fVRotate);
  dfViewer -> DoDraw(kFALSE);
}

void STEveManager::NextEventButton()  
{
  Int_t entry = fCurrentEventNumberEntry -> GetIntNumber();

  if (entry == fTotalNumEntries - 1)
    return;

  entry = entry + 1;
  fCurrentEventNumberEntry -> SetNumber(entry);
  SelectEventButton();
}

void STEveManager::PrevEventButton()
{
  Int_t entry = fCurrentEventNumberEntry -> GetIntNumber();

  if (entry == 0)
    return;

  entry = entry - 1;
  fCurrentEventNumberEntry -> SetNumber(entry);
  SelectEventButton();
}

void STEveManager::SelectEventButton() 
{
  RunEvent(fCurrentEventNumberEntry -> GetIntNumber());

  fEveTask -> PushParametersTask();

  if (fNumRiemannSet <= 0) fNumRiemannSet = 1;
  if (fNumLinearSet <= 0)  fNumLinearSet = 1;

  if (fCurrentRiemannSet) fCurrentRiemannSet -> SetLimitValues(0, fNumRiemannSet);
  if (fCurrentLinearSet)  fCurrentLinearSet  -> SetLimitValues(0, fNumLinearSet);
}

void 
STEveManager::SelectPad()
{
  fEveTask -> DrawADC(fCurrentRow -> GetIntNumber(), 
                      fCurrentLayer -> GetIntNumber());
}

void STEveManager::UpdateWindowTb()
{
  fEveTask -> UpdateWindowTbTask(fCurrentWindowTbStart -> GetIntNumber(), fCurrentWindowTbEnd -> GetIntNumber());
  UpdateEvent();
}

void 
STEveManager::ResetWindowTb()
{
  fEveTask -> UpdateWindowTbTask(0, 511);
  fCurrentWindowTbStart -> SetNumber(0);
  fCurrentWindowTbEnd -> SetNumber(511);
  UpdateEvent();
}

void 
STEveManager::DefaultWindowTb()
{
  fCurrentWindowTbStart -> SetNumber(fWindowTbStartDefault);
  fCurrentWindowTbEnd -> SetNumber(fWindowTbEndDefault);
  fEveTask -> UpdateWindowTbTask(fCurrentWindowTbStart -> GetIntNumber(), fCurrentWindowTbEnd -> GetIntNumber());
  UpdateEvent();
}

void STEveManager::ClickOnOffMC()         { fEveTask -> RnrEveObjectTask("mc");         gEve -> Redraw3D(); }
void STEveManager::ClickOnOffDigi()       { fEveTask -> RnrEveObjectTask("digi");       gEve -> Redraw3D(); }
void STEveManager::ClickOnOffHit()        { fEveTask -> RnrEveObjectTask("hit");        gEve -> Redraw3D(); }
void STEveManager::ClickOnOffHitBox()     { fEveTask -> RnrEveObjectTask("hitbox");     gEve -> Redraw3D(); }
void STEveManager::ClickOnOffCluster()    { fEveTask -> RnrEveObjectTask("cluser");     gEve -> Redraw3D(); }
void STEveManager::ClickOnOffClusterBox() { fEveTask -> RnrEveObjectTask("clusterbox"); gEve -> Redraw3D(); }
void STEveManager::ClickOnOffRiemannHit() { fEveTask -> RnrEveObjectTask("riemannhit"); gEve -> Redraw3D(); }
void STEveManager::ClickOnOffLinear()     { fEveTask -> RnrEveObjectTask("linear");     gEve -> Redraw3D(); }
void STEveManager::ClickOnOffLinearHit()  { fEveTask -> RnrEveObjectTask("linearhit");  gEve -> Redraw3D(); }
