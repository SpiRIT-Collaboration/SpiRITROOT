/*
 * @breif SPiRIT EVE
 * @author JungWoo Lee (Korea Univ.)
 */

#ifndef STEVEMANAGER
#define STEVEMANAGER

#include "STEveTask.hh"

#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include "FairRootManager.h"
#include "FairTask.h"
#include "FairLogger.h"

#include "TEveEventManager.h"
#include "TEveGValuators.h"
#include "TEveViewer.h"
#include "TGNumberEntry.h"
#include "TGeoManager.h"
#include "TGedFrame.h"
#include "GuiTypes.h"
#include "TGLayout.h"
#include "TGButton.h"
#include "TGLabel.h"
#include "TGLViewer.h"
#include "TGFrame.h"
#include "TTimer.h"

class TEveGValuator;
class TGCheckButton;
class TGWindow;

#include "TCanvas.h" 
#include "TString.h"
#include "TFile.h"

class TGListTreeItem;

#include "STSource.hh"

class STEveManager : public TEveEventManager
{
  public : 
    static STEveManager* Instance();
    STEveManager();
    ~STEveManager() {};

    Bool_t EveMode(TString name);

    /// For FairRun function
    void SetInputFile(TString name);
    void AddFriend(TString name);
    void SetOutputFile(TString name);
    void SetGeomFile(TString name);
    void SetParInputFile(TString name);

    /// Add tasks those needed to be executed before the event display.
    /// These task must be added before the eve tasks (AddEveTask)
    void AddTask(FairTask* task);

    /// Add tasks those only needed for updating event display.
    void AddEveTask(FairTask* task); 

    /// Initialize and build FairRun, frame, menu, geometry, viewer.
    void Init(Int_t option = 1, Int_t level = 3, Int_t nNodes = 10000);
    void BuildFrame();
    void BuildMenu();
    void BuildGeometry(Int_t option, Int_t level, Int_t nNodes);
    void Build3DViewer();

    // FUNCTIONS
    void UpdateEvent();         //!< Updated event (just eve tasks)
    void NextEvent();           //!< Move to next event
    void PrevEvent();           //!< Move to previous event
    void RunEvent(Long64_t entry); //!< Run event of entry

    // SETTERS
    void SetEveMode(TString mode);            //!< Set eve mode
    void SetGeomFileManual(TString name);     //!< Set path to geometry file
    void SetVolumeTransparency(Int_t val);    //!< Set detector geometry transparency
    void SetBackgroundColor(Color_t color);   //!< Set back ground color 
    void SetViewerPoint(Double_t hRotate, Double_t vRotate);    //!< Set viewer point
    void SetNumRiemannSet(Int_t num);         //!< Set number of riemann set
    void SetNumHelixSet(Int_t num);           //!< Set number of helix set
    void SetNumCurveSet(Int_t num);           //!< Set number of curve set
    void SetRowLayer(Int_t row, Int_t layer); //!< Set row & layer

    // GETTERS
    TCanvas*  GetCvsPadPlane();
    TCanvas*  GetCvsPad();
    TCanvas*  GetCvsPadADC();
    Long64_t  GetCurrentEventEntry();

    void UpdateSubNumberEntry1();
    void UpdateSubNumberEntry2();

    // BUTTONS
    void NextEventButton();
    void PrevEventButton();
    void SelectEventButton();
    void SelectPad();
    void UpdateWindowTb();
    void ResetWindowTb();
    void DefaultWindowTb();
    void RunEveSubTask();
    void RepeatEveSubTask();
    void StopEveSubTask();

    void ClickOnOffMC();
    void ClickOnOffDigi();
    void ClickOnOffHit();
    void ClickOnOffHitBox();
    void ClickOnOffCluster();
    void ClickOnOffClusterBox();
    void ClickOnOffRiemannTrack();
    void ClickOnOffRiemannHit();
    void ClickOnOffHelix();
    void ClickOnOffHelixHit();
    void ClickOnOffCurve();
    void ClickOnOffCurveHit();
    void ClickOnOffRecoTrack();
    void ClickOnOffRecoVertex();

    void Exit();

  private : // Frame
    TCanvas* fCvsPadPlane = NULL;  //!< Canvas for pad plane
    TCanvas* fCvsPadADC   = NULL;  //!< Canvas for adc distribution of the pad

  private : // Button
    TGNumberEntry *fCurrentEventNumberEntry = NULL;
    TGNumberEntry *fCurrentWindowTbStart    = NULL;
    TGNumberEntry *fCurrentWindowTbEnd      = NULL;
    TGNumberEntry *fCurrentRiemannSet       = NULL;
    TGNumberEntry *fCurrentHelixSet         = NULL;
    TGNumberEntry *fCurrentCurveSet         = NULL;
    TGNumberEntry *fCurrentRow              = NULL;
    TGNumberEntry *fCurrentLayer            = NULL;
    TGNumberEntry *fSubNumberEntry1         = NULL;
    TGNumberEntry *fSubNumberEntry2         = NULL;

    TGCheckButton *fButtonOnOffMC;
    TGCheckButton *fButtonOnOffDigi;
    TGCheckButton *fButtonOnOffHit;
    TGCheckButton *fButtonOnOffHitBox;
    TGCheckButton *fButtonOnOffCluster;
    TGCheckButton *fButtonOnOffClusterBox;
    TGCheckButton *fButtonOnOffRiemannTrack;
    TGCheckButton *fButtonOnOffRiemannHit;
    TGCheckButton *fButtonOnOffHelix;
    TGCheckButton *fButtonOnOffHelixHit;
    TGCheckButton *fButtonOnOffCurve;
    TGCheckButton *fButtonOnOffCurveHit;
    TGCheckButton *fButtonOnOffRecoTrack;
    TGCheckButton *fButtonOnOffRecoVertex;

  private :
    FairLogger *fLogger;    //! < Logger
    TString   fEveMode;     //!<  Eve mode (display frame style)

    FairRunAna *fRun;       //!<  Fair Run
    FairTask   *fMainTask;  //!<  Main tasks
    STEveTask  *fEveTask;   //!<  Eve tasks (part of main tasks - for online)

    TString  fGeomFileName; //!<  Name(path) of the geometry file 
    Int_t    fTransparency; //!<  Transparency of the geometry (0 - 100)
    Double_t fHRotate;      //!<  Horizontal rotation angle of 3D viewer point
    Double_t fVRotate;      //!<  Vertical   rotation angle of 3D viewer point
    Color_t  fClearColor;   //!<  Background color of the 3D viewer

    Long64_t fCurrentEventEntry; //!< Current entry (in tree) of the event.
    Long64_t fTotalNumEntries;   //!< Total number of entries

    Int_t fNumRiemannSet;
    Int_t fNumHelixSet;
    Int_t fNumCurveSet;

    Int_t fWindowTbStart;
    Int_t fWindowTbEnd;

    Int_t fWindowTbStartDefault;
    Int_t fWindowTbEndDefault;

    TEveViewer* fViewer3D = NULL;

    TTimer *fSubTaskTimer;


    static STEveManager *fInstance;

    STEveManager(const STEveManager&);
    STEveManager& operator=(const STEveManager&);


  ClassDef(STEveManager, 1);
};

#endif
