/*
 * @breif SPiRIT Event display manager
 * @author JungWoo Lee (Korea Univ.)
 */

#ifndef STEVENTMANAGER
#define STEVENTMANAGER

#include "TEveEventManager.h"

#include "FairRunAna.h"
#include "FairRunOnline.h"
#include "FairRootManager.h"
#include "FairTask.h"
#include "FairLogger.h"

#include "TCanvas.h"

#include "STEventManagerEditor.hh"
#include "STSource.hh"

class TGListTreeItem;

class STEventManager : public TEveEventManager
{
  public : 
    static STEventManager* Instance();
    STEventManager();
    ~STEventManager();

    void NextEvent();            ///< *MENU*
    void PrevEvent();            ///< *MENU*
    void RunEvent(Int_t eventid);
    void UpdateEvent();

    void AddTask(FairTask* task);
    void Init(Int_t option=1, Int_t level=3, Int_t nNodes=10000);
    void InitByEditor();

    void SetEventManagerEditor(STEventManagerEditor*);
    void SetVolumeTransparency(Int_t val);
    void SetClearColor(Color_t color);
    void SetViwerPoint(Double_t hRotate, Double_t vRotate);
    void SetGeomFile(TString name);

    void   SetOnlineEditor(Bool_t val);
    Bool_t Online();
    Bool_t OnlineEditor();

    Int_t    GetCurrentEvent();
    TCanvas* GetCvsPadPlane();
    TCanvas* GetCvsPad();

  private :
    FairRunAna* fRunAna;
    FairRunOnline* fRunOnline;
    FairLogger* fLogger; //!

    Bool_t fOnlineEditor;
    Bool_t fOnline;

    STEventManagerEditor* fEditor;

    Int_t fEntry;
    TGListTreeItem* fEvent;

    TCanvas* fCvsPadPlane;
    TCanvas* fCvsPad;

    TString fGeomFileName;
    Int_t fTransparency;

    Bool_t fUseUserViewerPoint;
    Double_t fHRotate;
    Double_t fVRotate;

    Color_t fClearColor;

    static STEventManager* fInstance;

    STEventManager(const STEventManager&);
    STEventManager& operator=(const STEventManager&);


  ClassDef(STEventManager,1);
};

#endif
