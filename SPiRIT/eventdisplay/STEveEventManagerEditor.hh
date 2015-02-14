#pragma once

#include "TGedFrame.h"
#include "GuiTypes.h"
#include "Rtypes.h"
#include "TGFrame.h"

class STEveEventManager;
class TEveGValuator;
class TGCheckButton;
class TGLabel;
class TGNumberEntry;
class TGWindow;
class TObject;

class STEveEventManagerEditor : public TGedFrame
{
    STEveEventManagerEditor(const STEveEventManagerEditor&); // Not implemented
    STEveEventManagerEditor& operator=(const STEveEventManagerEditor&); // Not implemented

  protected:
    TObject* fObject;
    STEveEventManager* fManager;
    TGNumberEntry*  fCurrentEvent;
    TGLabel* fEventTime;
  public:
    STEveEventManagerEditor(const TGWindow* p=0, Int_t width=170, Int_t height=30,
                           UInt_t options = kChildFrame, Pixel_t back=GetDefaultFrameBackground());
    virtual ~STEveEventManagerEditor() {}
    void SetModel(TObject* obj);
    virtual void SelectEvent();
    virtual void Init();

    ClassDef(STEveEventManagerEditor, 0); // Specialization of TGedEditor for proper update propagation to TEveManager.
};
