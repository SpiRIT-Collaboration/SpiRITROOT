/*
 * @breif SPiRIT Event display manager
 * @author JungWoo Lee (Korea Univ.)
 */

#pragma once

#include "TEveEventManager.h"
//#include "STEventDrawTask.hh"

#include "FairRunAna.h"
#include "FairRootManager.h"
#include "FairTask.h"

#include "TCanvas.h"

class TGListTreeItem;

class STEventManager : public TEveEventManager
{
  public : 
    static STEventManager* Instance();
    STEventManager();
    virtual ~STEventManager();

    virtual void GotoEvent(Int_t event); ///< *MENU*
    virtual void NextEvent();            ///< *MENU*
    virtual void PrevEvent();            ///< *MENU*

    void AddTask(FairTask* task);
    virtual void InitRiemann(Int_t option=1, Int_t level=3, Int_t nNodes=10000);
    virtual void Init(Int_t option=1, Int_t level=3, Int_t nNodes=10000);

    void SetSelfRiemannSet(Int_t val) {};

    virtual Int_t GetCurrentEvent() {return fEntry;}

    TCanvas* GetCvsPadPlane() { return fCvsPadPlane; }

    void RunEvent();

  private :
    FairRootManager* fRootManager;
    FairRunAna* fRunAna;

    Int_t fEntry;
    TGListTreeItem* fEvent;

    TCanvas* fCvsPadPlane;

    static STEventManager* fInstance;

    STEventManager(const STEventManager&);
    STEventManager& operator=(const STEventManager&);


  ClassDef(STEventManager,1);
};
