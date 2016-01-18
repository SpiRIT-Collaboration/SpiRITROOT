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
#include "TGNumberEntry.h"
#include "TGeoManager.h"
#include "TGedFrame.h"
#include "GuiTypes.h"
#include "TGLayout.h"
#include "TGButton.h"
#include "TGLabel.h"
#include "TGFrame.h"

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
    enum STEveMode { kAll, kOverview, k3D };

    static STEveManager* Instance();
    STEveManager();
    ~STEveManager() {};

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
    void SetEveMode(STEveMode mode);          //!< Set eve mode
    void SetGeomFileManual(TString name);     //!< Set path to geometry file
    void SetVolumeTransparency(Int_t val);    //!< Set detector geometry transparency
    void SetBackgroundColor(Color_t color);   //!< Set back ground color 
    void SetViewerPoint(Double_t hRotate, Double_t vRotate);    //!< Set viewer point
    void SetNumRiemannSet(Int_t num);         //!< Set number of riemann set
    void SetNumLinearSet(Int_t num);          //!< Set number of linear set
    void SetRowLayer(Int_t row, Int_t layer); //!< Set row & layer

    // GETTERS
    STEveMode GetEveMode();
    TCanvas*  GetCvsPadPlane();
    TCanvas*  GetCvsPad();
    TCanvas*  GetCvsPadADC();
    Long64_t  GetCurrentEventEntry();

    // BUTTONS
    void NextEventButton();
    void PrevEventButton();
    void SelectEventButton();
    void SelectPad();
    void UpdateWindowTb();
    void ResetWindowTb();
    void DefaultWindowTb();

    void ClickOnOffMC();
    void ClickOnOffDigi();
    void ClickOnOffHit();
    void ClickOnOffHitBox();
    void ClickOnOffCluster();
    void ClickOnOffClusterBox();
    void ClickOnOffRiemannHit();
    void ClickOnOffLinear();
    void ClickOnOffLinearHit();

  private : // Frame
    TCanvas* fCvsPadPlane;  //!< Canvas for pad plane
    TCanvas* fCvsPadADC;    //!< Canvas for adc distribution of the pad

  private : // Button
    TGNumberEntry *fCurrentEventNumberEntry;
    TGNumberEntry *fCurrentWindowTbStart;
    TGNumberEntry *fCurrentWindowTbEnd;

    TGNumberEntry *fCurrentRiemannSet;
    TGNumberEntry *fTempRiemannSet;

    TGNumberEntry *fCurrentLinearSet;
    TGNumberEntry *fTempLinearSet;

    TGNumberEntry *fCurrentRow;
    TGNumberEntry *fCurrentLayer;
    TGLabel       *fEventTime;

    TGCheckButton *fButtonOnOffMC;
    TGCheckButton *fButtonOnOffDigi;
    TGCheckButton *fButtonOnOffHit;
    TGCheckButton *fButtonOnOffHitBox;
    TGCheckButton *fButtonOnOffCluster;
    TGCheckButton *fButtonOnOffClusterBox;
    TGCheckButton *fButtonOnOffRiemannHit;
    TGCheckButton *fButtonOnOffLinear;
    TGCheckButton *fButtonOnOffLinearHit;

  private :
    FairLogger *fLogger;    //! < Logger
    STEveMode   fEveMode;   //!<  Eve mode (display frame style)

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
    Int_t fNumLinearSet;

    Int_t fWindowTbStart;
    Int_t fWindowTbEnd;

    Int_t fWindowTbStartDefault;
    Int_t fWindowTbEndDefault;


    static STEveManager *fInstance;

    STEveManager(const STEveManager&);
    STEveManager& operator=(const STEveManager&);


  ClassDef(STEveManager, 1);
};

#endif
