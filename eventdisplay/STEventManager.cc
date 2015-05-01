/*
 * @breif SPiRIT Event display manager
 * @author JungWoo Lee (Korea Univ.)
 */

#include "STEventManager.hh"

#include "TEveGeoNode.h"
#include "TEveManager.h"
#include "TEveProjectionManager.h"
#include "TEveScene.h"
#include "TEveViewer.h"
#include "TEveWindow.h"
#include "TEveBrowser.h"
#include "TRootEmbeddedCanvas.h"
#include "TObjArray.h"

#include "TGTab.h"
#include "TGLViewer.h"
#include "TGeoManager.h"
#include "TVirtualX.h"
#include "TGWindow.h"

#include <iostream>

class TGeoNode;

using namespace std;

ClassImp(STEventManager);

STEventManager* STEventManager::fInstance=0;
STEventManager* STEventManager::Instance()
{
  return fInstance;
}

STEventManager::STEventManager()
: TEveEventManager("STEventManager",""),
  fRootManager(FairRootManager::Instance()),
  fRunAna(FairRunAna::Instance()),
  fEntry(0),
  fEvent(0),
  fCvsPadPlane(0),
  fCvsPad(0),
  fTransparency(80)
{
  fInstance=this;
}

STEventManager::~STEventManager()
{
}

void
STEventManager::InitRiemann(Int_t option, Int_t level, Int_t nNodes)
{
  TEveManager::Create();
  fRunAna->Init();
  fEvent= gEve->AddEvent(this);
}

void 
STEventManager::Init(Int_t option, Int_t level, Int_t nNodes)
{
  TEveManager::Create();

  Int_t  dummy;
  UInt_t width, height;
  UInt_t widthMax = 1400, heightMax = 850;
  Double_t ratio = (Double_t)widthMax/heightMax;
  gVirtualX -> GetWindowSize(gClient -> GetRoot() -> GetId(), dummy, dummy, width, height);
  if(width>widthMax){ width = widthMax; height = heightMax; } 
  else height = (Int_t)(width/ratio);
  gEve -> GetMainWindow() -> Resize(width,height);

  /**************************************************************************/

  TEveWindowSlot* slotOverview = NULL;
  TEveWindowPack* packOverview = NULL;
  TEveWindowPack* packLeft = NULL;
  TEveWindowSlot* slotPadPlane = NULL;
  TEveWindowSlot* slotPad = NULL;

  TRootEmbeddedCanvas* ecvsPadPlane = new TRootEmbeddedCanvas();
  TRootEmbeddedCanvas* ecvsPad = new TRootEmbeddedCanvas();
  TEveWindowFrame* framePadPlane = NULL;
  TEveWindowFrame* framePad = NULL;

  slotOverview = TEveWindow::CreateWindowInTab(gEve->GetBrowser()->GetTabRight());
  slotOverview -> SetElementName("Overview");
  slotOverview -> SetShowTitleBar(kFALSE);
  packOverview = slotOverview -> MakePack();
  packOverview -> SetElementName("Overview");
  //packOverview -> SetShowTitleBar(kFALSE);
  packOverview -> SetHorizontal();

  packLeft = packOverview -> NewSlot() -> MakePack();
  packLeft -> SetElementName("left pack");
  packLeft -> SetShowTitleBar(kFALSE);
  packLeft -> SetVertical();
  packLeft -> NewSlot() -> MakeCurrent();
  TEveViewer* viwer3D = gEve -> SpawnNewViewer("3D View", "");
  viwer3D -> AddScene(gEve->GetGlobalScene());
  viwer3D -> AddScene(gEve->GetEventScene());

  slotPadPlane = packOverview -> NewSlot();
  //slotPadPlane -> SetElementName("pad plane");
  //slotPadPlane -> SetShowTitleBar(kFALSE);
  framePadPlane = slotPadPlane -> MakeFrame(ecvsPadPlane);
  framePadPlane -> SetElementName("SpiRIT Pad Plane");
  framePadPlane -> SetShowTitleBar(kFALSE);
  //packOverview -> GetEveFrame() -> SetShowTitleBar(kFALSE);
  fCvsPadPlane = ecvsPadPlane -> GetCanvas();

  slotPad = packLeft -> NewSlotWithWeight(.6);
  //slotPad -> SetElementName("pad");
  //slotPad -> SetShowTitleBar(kFALSE);
  framePad = slotPad -> MakeFrame(ecvsPad);
  framePad -> SetElementName("pad");
  framePad -> SetShowTitleBar(kFALSE);
  //packLeft -> GetEveFrame() -> SetShowTitleBar(kFALSE);
  fCvsPad = ecvsPad -> GetCanvas();

  /**************************************************************************/

  fRunAna->Init();

  if(gGeoManager) {
    TGeoNode* geoNode = gGeoManager->GetTopNode();
    TEveGeoTopNode* topNode
      = new TEveGeoTopNode(gGeoManager, geoNode, option, level, nNodes);
    gEve->AddGlobalElement(topNode);

    TObjArray* listVolume = gGeoManager -> GetListOfVolumes();
    Int_t nVolumes = listVolume -> GetEntries();
    for(Int_t i=0; i<nVolumes; i++)
    {
      ((TGeoVolume*) listVolume -> At(i)) -> SetTransparency(fTransparency);
    }

    gEve->FullRedraw3D(kTRUE);
    fEvent= gEve->AddEvent(this);
  }

  /**************************************************************************/

  gEve -> GetBrowser() -> GetTabRight() -> SetTab(1);
  gEve -> Redraw3D(kTRUE, kTRUE);

  TGLViewer *dfViewer = gEve->GetDefaultGLViewer();
  dfViewer -> CurrentCamera().RotateRad(-.7, 2.3);
  dfViewer -> DoDraw();
}

void 
STEventManager::GotoEvent(Int_t event)
{
  fEntry=event;
  fRunAna->Run((Long64_t)event);
}

void 
STEventManager::NextEvent()
{
  fEntry+=1;
  fRunAna->Run((Long64_t)fEntry);
}

void 
STEventManager::PrevEvent()
{
  fEntry-=1;
  fRunAna->Run((Long64_t)fEntry);
}

void
STEventManager::RunEvent()
{
  fRunAna->Run((Long64_t)fEntry);
} 

void
STEventManager::AddTask(FairTask* task)
{ 
  fRunAna->AddTask(task); 
}
