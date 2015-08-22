/*
 * @breif SPiRIT Event display manager
 * @author JungWoo Lee (Korea Univ.)
 */

#ifndef STEVENTMANAGER
#define STEVENTMANAGER

#include "TEveEventManager.h"

#include "FairRunAna.h"
#include "FairRootManager.h"
#include "FairTask.h"
#include "FairLogger.h"

#include "TCanvas.h"

#include "STEventManagerEditor.hh"

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
    //virtual void InitRiemann(Int_t option=1, Int_t level=3, Int_t nNodes=10000);
    virtual void Init(Int_t option=1, Int_t level=3, Int_t nNodes=10000);

    void SetSelfRiemannSet(Int_t val) {}
    void SetVolumeTransparency(Int_t val) { fTransparency = val; }

    virtual Int_t GetCurrentEvent() { return fEntry; }

    TCanvas* GetCvsPadPlane() { return fCvsPadPlane; }
    TCanvas* GetCvsPad()      { return fCvsPad; }

    void RunEvent();

    void SetEventManagerEditor(STEventManagerEditor*);
    void InitByEditor();

  private :
    FairRootManager* fRootManager;
    FairRunAna* fRunAna;
    FairLogger* fLogger;

    STEventManagerEditor* fEditor;

    Int_t fEntry;
    TGListTreeItem* fEvent;

    TCanvas* fCvsPadPlane;
    TCanvas* fCvsPad;

    Int_t fTransparency;


    static STEventManager* fInstance;

    STEventManager(const STEventManager&);
    STEventManager& operator=(const STEventManager&);


  ClassDef(STEventManager,1);
};

#endif
