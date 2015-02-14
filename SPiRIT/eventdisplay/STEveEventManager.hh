/*
 * @breif SPiRIT Event display manager
 * @author JungWoo Lee (Korea Univ.)
 */

#pragma once

#include "TEveEventManager.h"
//#include "FairEventManager.h"

#include "FairRunAna.h"
#include "FairRootManager.h"
#include "FairTask.h"

#include "TCanvas.h"

class TGListTreeItem;

class STEveEventManager : public TEveEventManager
{
  public : 
    static STEveEventManager* Instance();
    STEveEventManager();
    virtual ~STEveEventManager();

    virtual void GotoEvent(Int_t event); ///< *MENU*
    virtual void NextEvent();            ///< *MENU*
    virtual void PrevEvent();            ///< *MENU*

    void AddTask(FairTask* task) { fRunAna->AddTask(task); }
    virtual void Init(Int_t option=1, Int_t level=3, Int_t nNodes=10000);

    virtual Int_t GetCurrentEvent() {return fEntry;}

    TCanvas* GetCvsPadPlane() { return fCvsPadPlane; }

    void RunEvent();

  private :
    FairRootManager* fRootManager;
    FairRunAna* fRunAna;

    Int_t fEntry;
    TGListTreeItem* fEvent;

    TCanvas* fCvsPadPlane;

    static STEveEventManager* fInstance;

    STEveEventManager(const STEveEventManager&);
    STEveEventManager& operator=(const STEveEventManager&);


  ClassDef(STEveEventManager,1);
};
