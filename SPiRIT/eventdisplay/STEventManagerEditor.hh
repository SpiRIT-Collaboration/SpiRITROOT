#pragma once

#include "TGedFrame.h"
#include "GuiTypes.h"
#include "Rtypes.h"
#include "TGFrame.h"

class STEventManager;
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
    STEventManager* fManager;
    TGNumberEntry*  fCurrentEvent;
    TGLabel* fEventTime;
  public:
    STEventManagerEditor(const TGWindow* p=0, Int_t width=170, Int_t height=30,
                           UInt_t options = kChildFrame, Pixel_t back=GetDefaultFrameBackground());
    virtual ~STEventManagerEditor() {}
    void SetModel(TObject* obj);
    virtual void SelectEvent();
    virtual void Init();

    ClassDef(STEventManagerEditor, 0); // Specialization of TGedEditor for proper update propagation to TEveManager.
};
