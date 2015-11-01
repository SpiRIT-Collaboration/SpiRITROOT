/*
 * @breif SPiRIT Event display manager
 * @author JungWoo Lee (Korea Univ.)
 */

#include "STEventManager.hh"
#include "STEventDrawTask.hh"

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

ClassImp(STEventManager);

STEventManager* STEventManager::fInstance = 0;
STEventManager* STEventManager::Instance()
{
  return fInstance;
}

STEventManager::STEventManager()
: TEveEventManager("STEventManager","")
{
  fLogger = FairLogger::GetLogger();

  fRunOnline = FairRunOnline::Instance();
  fRunAna = FairRunAna::Instance();

  if (fRunOnline != NULL) {
    fOnline = kTRUE;
    fOnlineEditor = kTRUE;
    fLogger -> Info(MESSAGE_ORIGIN, "Eve initalized with FairRunOnline.");
  }
  else {
    fOnline = kFALSE;
    fOnlineEditor = kTRUE;
    fLogger -> Info(MESSAGE_ORIGIN, "Eve initalized with FairRunAna.");
  }

  fEntry = -1;

  fEvent = NULL;
  fCvsPadPlane = NULL;
  fCvsPad = NULL;

  fGeomFileName = "";
  fTransparency = 80;

  fClearColor = kWhite;

  fUseUserViewerPoint = kFALSE;
  fHRotate = 0;
  fVRotate = 0;

  fInstance = this;
}

STEventManager::~STEventManager()
{
}

void 
STEventManager::Init(Int_t option, Int_t level, Int_t nNodes)
{
  fLogger -> Debug(MESSAGE_ORIGIN, "STEventManager Init().");

  TEveManager::Create(kTRUE, "V");

  Int_t dummy;
  UInt_t width, height;
  UInt_t widthMax  = 1400;
  UInt_t heightMax = 850;
  Double_t ratio = (Double_t)widthMax/heightMax;

  gVirtualX -> GetWindowSize(gClient -> GetRoot() -> GetId(), dummy, dummy, width, height);

  if (width > widthMax ) { 
    width  = widthMax; 
    height = heightMax; 
  } 
  else 
    height = (Int_t)(width/ratio);

  gEve -> GetMainWindow() -> Resize(width,height);

  /**************************************************************************/

  TEveWindowSlot* slotOverview = NULL;
  TEveWindowSlot* slot3D = NULL;
  TEveWindowPack* packOverview = NULL;
  TEveWindowPack* packLeft = NULL;
  TEveWindowSlot* slotPadPlane = NULL;
  TEveWindowSlot* slotPad = NULL;

  TRootEmbeddedCanvas* ecvsPadPlane = new TRootEmbeddedCanvas();
  TRootEmbeddedCanvas* ecvsPad = new TRootEmbeddedCanvas();
  TEveWindowFrame* framePadPlane = NULL;
  TEveWindowFrame* framePad = NULL;

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

  slotPad = packLeft -> NewSlotWithWeight(.6);
  slotPad -> SetShowTitleBar(kFALSE);
  framePad = slotPad -> MakeFrame(ecvsPad);
  framePad -> SetElementName("pad");
  fCvsPad = ecvsPad -> GetCanvas();

  /**************************************************************************/

  if (fOnline) 
    fRunOnline -> Init();
  else         
    fRunAna -> Init();

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
    TEveGeoTopNode* topNode = new TEveGeoTopNode(gGeoManager,geoNode);
    gEve -> AddGlobalElement(topNode);
  }

  TObjArray* listVolume = gGeoManager -> GetListOfVolumes();
  Int_t nVolumes = listVolume -> GetEntries();
  for (Int_t i=0; i<nVolumes; i++)
    ((TGeoVolume*) listVolume -> At(i)) -> SetTransparency(fTransparency);

  gEve -> FullRedraw3D(kTRUE);

  fLogger -> Debug(MESSAGE_ORIGIN, "Adding STEventManager to TEveManager.");
  fEvent = gEve -> AddEvent(this);

  /**************************************************************************/

  gEve -> GetBrowser() -> GetTabRight() -> SetTab(1);
  gEve -> GetBrowser() -> HideBottomTab();
  gEve -> GetWindowManager() -> HideAllEveDecorations();

  TGLViewer *dfViewer3D = viewer3D -> GetGLViewer();
  dfViewer3D -> SetClearColor(fClearColor);
  if (fUseUserViewerPoint)
    dfViewer3D -> CurrentCamera().RotateRad(fHRotate, fVRotate);
  else
    dfViewer3D -> CurrentCamera().RotateRad(-0.7, -1.1);
  dfViewer3D -> DoDraw(kFALSE);

  TGLViewer *dfViewer = gEve -> GetDefaultGLViewer();
  dfViewer -> SetClearColor(fClearColor);
  if (fUseUserViewerPoint)
    dfViewer -> CurrentCamera().RotateRad(fHRotate, fVRotate);
  else
    dfViewer -> CurrentCamera().RotateRad(-0.7, -1.1);
  dfViewer -> DoDraw(kFALSE);

  gEve -> ElementSelect(gEve -> GetCurrentEvent());
  gEve -> GetBrowser() -> GetTabLeft() -> SetTab(2);

  fLogger -> Debug(MESSAGE_ORIGIN, "STEventManager End of Init().");
}

void 
STEventManager::InitByEditor()
{
  fLogger -> Debug(MESSAGE_ORIGIN, "STEventManager InitByEditor().");

  gEve -> GetBrowser() -> StartEmbedding(TRootBrowser::kLeft);

  TGMainFrame* mainFrame = new TGMainFrame(gClient -> GetRoot(), 1000, 600);
  fEditor -> FillFrameContent(mainFrame);

  mainFrame -> MapSubwindows();
  mainFrame -> MapWindow();

  gEve -> GetBrowser() -> StopEmbedding();
  gEve -> GetBrowser() -> SetTabTitle("Event control", TRootBrowser::kLeft);
}

void STEventManager::AddTask(FairTask* task) 
{ 
  if (fOnline) fRunOnline -> AddTask(task);
  else         fRunAna -> AddTask(task);
}

void STEventManager::NextEvent()  { RunEvent(fEntry + 1); }
void STEventManager::PrevEvent()  { RunEvent(fEntry - 1); }

void STEventManager::RunEvent(Int_t eventid)
{ 
  if (fOnline) {
    ((STSource*) fRunOnline -> GetSource()) -> SetEventID(eventid);
    fRunOnline -> Run(1, 0);
  }
  else 
    fRunAna -> Run((Long64_t)eventid);

  fEntry = eventid;
}

void STEventManager::UpdateEvent() { STEventDrawTask::Instance() -> Exec(""); }

void STEventManager::SetEventManagerEditor(STEventManagerEditor* editor) { fEditor = editor; }
void STEventManager::SetVolumeTransparency(Int_t val) { fTransparency = val; }
void STEventManager::SetClearColor(Color_t color)     { fClearColor = color; }
void STEventManager::SetViwerPoint(Double_t hRotate, Double_t vRotate)
{
  fUseUserViewerPoint = kTRUE;
  fHRotate = hRotate;
  fVRotate = vRotate;
}
void STEventManager::SetGeomFile(TString name) { fGeomFileName = name; }

void   STEventManager::SetOnlineEditor(Bool_t val) { fOnlineEditor = val; }
Bool_t STEventManager::Online()                    { return fOnline; }
Bool_t STEventManager::OnlineEditor()              { return fOnlineEditor; }

Int_t    STEventManager::GetCurrentEvent() { return fEntry; }
TCanvas* STEventManager::GetCvsPadPlane()  { return fCvsPadPlane; }
TCanvas* STEventManager::GetCvsPad()       { return fCvsPad; }
