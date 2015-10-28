#ifndef STEVENTMANAGEREDITOR
#define STEVENTMANAGEREDITOR

#include "TGedFrame.h"
#include "GuiTypes.h"
#include "Rtypes.h"
#include "TGFrame.h"
#include "FairLogger.h"

class STEventManager;
class STEventDrawTask;
class TEveGValuator;
class TGCheckButton;
class TGLabel;
class TGNumberEntry;
class TGWindow;
class TObject;

class STEventManagerEditor : public TGedFrame
{
    STEventManagerEditor(const STEventManagerEditor&); // Not implemented
    STEventManagerEditor& operator=(const STEventManagerEditor&); // Not implemented

  protected:
    TObject* fObject;
    STEventManager*  fManager;
    STEventDrawTask* fDrawTask;
    TGNumberEntry*   fCurrentEvent;
    TGNumberEntry*   fCurrentWindowTbStart;
    TGNumberEntry*   fCurrentWindowTbEnd;

    TGNumberEntry*   fCurrentRiemannSet;
    TGNumberEntry*   fTempRiemannSet;

    TGNumberEntry*   fCurrentLinearSet;
    TGNumberEntry*   fTempLinearSet;

    TGNumberEntry*   fCurrentRow;
    TGNumberEntry*   fCurrentLayer;
    TGLabel*         fEventTime;

    TGVerticalFrame* fEditorTabSubFrame;

    Int_t fWindowTbStartDefault;
    Int_t fWindowTbEndDefault;

  public:
    STEventManagerEditor(const TGWindow* p=0, Int_t width=170, Int_t height=30,
                           UInt_t options = kChildFrame, Pixel_t back=GetDefaultFrameBackground());
    virtual ~STEventManagerEditor() {}
    void SetModel(TObject* obj);
    virtual void NextEvent();
    virtual void BeforeEvent();
    virtual void SelectEvent();
    virtual void SelectEventIf();
    virtual void SelectPad();
    virtual void SelectPadIf();
    virtual void UpdateWindowTb();
    virtual void ResetWindowTb();
    virtual void DefaultWindowTb();
    virtual void Init();

    void SelectRiemannSet();
    void AddRiemannSet();
    void VisAllRiemannSet();
    void VisOffRiemannSet();
    Int_t GetNRiemannSet();

    void SelectLinearSet();
    void AddLinearSet();
    void VisAllLinearSet();
    void VisOffLinearSet();

    void ToggleAutoUpdate(Bool_t onoff);
    void ToggleAutoUpdatePad(Bool_t onoff);

    void SetRowLayer(Int_t row, Int_t layer);

    TGVerticalFrame* GetEditorTabSubFrame();

    void FillFrameContent(TGCompositeFrame* frame);

  private:
    Bool_t fAutoUpdateFlag;
    Bool_t fAutoUpdatePadFlag;

    FairLogger* fLogger;

    Int_t fEntries;

    Bool_t fOnline;
    Bool_t fOnlineEditor;

    ClassDef(STEventManagerEditor, 0); // Specialization of TGedEditor for proper update propagation to TEveManager.
};

#endif
