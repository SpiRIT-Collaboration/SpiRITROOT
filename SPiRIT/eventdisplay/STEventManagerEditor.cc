#include "STEventManagerEditor.hh"
#include "STEventManager.hh"

#include "FairRootManager.h"
#include "FairRunAna.h"

#include "TChain.h"
#include "TEveGValuators.h"
#include "TFile.h"
#include "TGButton.h"
#include "TGLabel.h"
#include "TGLayout.h"
#include "TGNumberEntry.h"
#include "TGeoManager.h"
#include "TString.h"

#include <stddef.h>

class TGWindow;
class TObject;

ClassImp(STEventManagerEditor)

STEventManagerEditor::STEventManagerEditor
(const TGWindow* p, Int_t width, Int_t height, UInt_t options, Pixel_t back)
: TGedFrame(p, width, height, options | kVerticalFrame, back),
  fObject(0),
  fManager(STEventManager::Instance()),
  fCurrentEvent(0),
  fEventTime(NULL)
{
  Init();
}

void 
STEventManagerEditor::Init()
{
  FairRootManager* fRootManager=FairRootManager::Instance();
  TChain* chain = fRootManager->GetInChain();
  Int_t Entries= chain->GetEntriesFast();

  MakeTitle("STEventManager  Editor");
  TGVerticalFrame* fEventFrame= CreateEditorTabSubFrame("Event");
  TGCompositeFrame* title1 = new TGCompositeFrame(fEventFrame, 250, 10,
      kVerticalFrame | kLHintsExpandX |
      kFixedWidth    | kOwnBackground);

  TString Infile= "Input file : ";
  TFile* file =FairRootManager::Instance()->GetInChain()->GetFile();
  Infile+=file->GetName();
  TGLabel* TFName=new TGLabel(title1, Infile.Data());
  title1->AddFrame(TFName);

  UInt_t RunId= FairRunAna::Instance()->getRunId();
  TString run= "Run Id : ";
  run += RunId;
  TGLabel* TRunId=new TGLabel(title1, run.Data());
  title1->AddFrame(TRunId);

  TString nevent= "No of events : ";
  nevent +=Entries ;
  TGLabel* TEvent=new TGLabel(title1, nevent.Data());
  title1->AddFrame(TEvent);

  Int_t nodes= gGeoManager->GetNNodes();
  TString NNodes= "No. of Nodes : ";
  NNodes += nodes;
  TGLabel* NoNode = new TGLabel(title1, NNodes.Data());
  title1->AddFrame(NoNode);

  TGHorizontalFrame* f = new TGHorizontalFrame(title1);
  TGLabel* l = new TGLabel(f, "Current Event:");
  f->AddFrame(l, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 1, 2, 1, 1));
  fCurrentEvent 
    = new TGNumberEntry(f, 0., 6, -1,
                        TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative,
                        TGNumberFormat::kNELLimitMinMax, 0, Entries);
  f->AddFrame(fCurrentEvent, new TGLayoutHints(kLHintsLeft, 1, 1, 1, 1));
  fCurrentEvent->Connect("ValueSet(Long_t)","STEventManagerEditor", 
                         this, "SelectEvent()");
  title1->AddFrame(f);

  TGHorizontalFrame* f2 = new TGHorizontalFrame(title1);
  TGLabel* EventTimeLabel = new TGLabel(f2, "Event Time: ");
  fEventTime = new TGLabel(f2,"");
  f2->AddFrame(EventTimeLabel);
  f2->AddFrame(fEventTime);
  title1->AddFrame(f2);

  TGTextButton* fUpdate = new TGTextButton(title1, "Update");
  title1->AddFrame(fUpdate, new TGLayoutHints(kLHintsRight | kLHintsExpandX, 5,5,1,1));
  fUpdate->Connect("Clicked()", "STEventManagerEditor", this, "SelectEvent()");

  fEventFrame->AddFrame(title1, new TGLayoutHints(kLHintsTop, 0, 0, 2, 0));
}

void 
STEventManagerEditor::SelectEvent()
{
  fManager->GotoEvent(fCurrentEvent->GetIntNumber());

  TString time;
  time.Form("%.2f", FairRootManager::Instance()->GetEventTime());
  time += " ns";
  fEventTime->SetText(time.Data());

  Update();
}

void 
STEventManagerEditor::SetModel(TObject* obj)
{
  fObject = obj;
}
