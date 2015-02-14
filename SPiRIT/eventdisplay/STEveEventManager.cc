/*
 * @breif SPiRIT Event display manager
 * @author JungWoo Lee (Korea Univ.)
 */

#include "STEveEventManager.hh"

#include "TEveGeoNode.h"
#include "TEveManager.h"
#include "TEveProjectionManager.h"
#include "TEveScene.h"
#include "TEveViewer.h"
#include "TEveWindow.h"
#include "TEveBrowser.h"

#include "TRootEmbeddedCanvas.h"

#include "TGTab.h"
#include "TGLViewer.h"
#include "TGeoManager.h"
#include "TVirtualX.h"
#include "TGWindow.h"

#include <iostream>

class TGeoNode;

using namespace std;

ClassImp(STEveEventManager);

STEveEventManager* STEveEventManager::fInstance=0;
STEveEventManager* STEveEventManager::Instance()
{
  return fInstance;
}

STEveEventManager::STEveEventManager()
: TEveEventManager("STEveEventManager",""),
  fRootManager(FairRootManager::Instance()),
  fRunAna(FairRunAna::Instance()),
  fEntry(0),
  fEvent(0),
  fCvsPadPlane(0)
{
  //SetName("STEveEventManager");
  fInstance=this;
}

STEveEventManager::~STEveEventManager()
{
}

void 
STEveEventManager::Init(Int_t option, Int_t level, Int_t nNodes)
{
  TEveManager::Create();

  Int_t  dummy;
  UInt_t width, height;
  UInt_t widthMax = 1400, heightMax = 650;
  Double_t ratio = (Double_t)widthMax/heightMax;
  gVirtualX->GetWindowSize(gClient->GetRoot()->GetId(), dummy, dummy, width, height);
  // Assume that width of screen is always larger than the height of screen
  if(width>widthMax){ width = widthMax; height = heightMax; } 
  else height = (Int_t)(width/ratio);
  gEve->GetMainWindow()->Resize(width,height);

  /**************************************************************************/

  TEveWindowSlot* slot = 0;
  TEveWindowPack* pack = 0;

  // 3D
  slot = TEveWindow::CreateWindowInTab(gEve->GetBrowser()->GetTabRight());
  pack = slot->MakePack();
  pack->SetElementName("Overview");
  pack->SetHorizontal();
  pack->SetShowTitleBar(kFALSE);

  pack->NewSlot()->MakeCurrent();
  TEveViewer* view3D = gEve->SpawnNewViewer("3D View", "");
  view3D->AddScene(gEve->GetGlobalScene());
  view3D->AddScene(gEve->GetEventScene());


  slot = pack->NewSlotWithWeight(1.5);
  TRootEmbeddedCanvas* ecvs = new TRootEmbeddedCanvas();
  TEveWindowFrame* frame = slot->MakeFrame(ecvs);
  frame->SetElementName("SpiRIT Pad Plane");
  pack->GetEveFrame()->SetShowTitleBar(kFALSE);
  fCvsPadPlane = ecvs->GetCanvas();

  /**************************************************************************/

  fRunAna->Init();

  if(gGeoManager) {
    TGeoNode* geoNode = gGeoManager->GetTopNode();
    TEveGeoTopNode* topNode
      = new TEveGeoTopNode(gGeoManager, geoNode, option, level, nNodes);
    gEve->AddGlobalElement(topNode);
    gEve->FullRedraw3D(kTRUE);
    fEvent= gEve->AddEvent(this);
  }

  /**************************************************************************/

  gEve->GetBrowser()->GetTabRight()->SetTab(1);
  gEve->Redraw3D(kTRUE, kTRUE);

  //TGLViewer *dfViewer = gEve->GetDefaultGLViewer();
  //dfViewer->CurrentCamera().RotateRad(-.7, 0.5);
  //dfViewer->DoDraw();

  RunEvent();
}

void 
STEveEventManager::GotoEvent(Int_t event)
{
  fEntry=event;
  fRunAna->Run((Long64_t)event);
}

void 
STEveEventManager::NextEvent()
{
  fEntry+=1;
  fRunAna->Run((Long64_t)fEntry);
}

void 
STEveEventManager::PrevEvent()
{
  fEntry-=1;
  fRunAna->Run((Long64_t)fEntry);
}

void
STEveEventManager::RunEvent()
{
  fRunAna->Run((Long64_t)fEntry);
}
