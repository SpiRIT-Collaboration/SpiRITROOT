//---------------------------------------------------------------------
// Description:
//      RIDF analysis code for commissioning run
//
// Author List:
//      Giordano Cerizza         MSU      (original author)
//----------------------------------------------------------------------

#include "STEventRIDFTask.hh"

ClassImp(STEventRIDFTask)

STEventRIDFTask::STEventRIDFTask()
{

  fEventManager = NULL;
  
  fHist_a = NULL; 
  fHist_b = NULL; 

  fCvs_a  = NULL; 
  fCvs_b  = NULL; 

  fBeamPlots = kFALSE;
}

STEventRIDFTask::~STEventRIDFTask()
{
}

InitStatus
STEventRIDFTask::Init()
{

  fLogger -> Debug(MESSAGE_ORIGIN,"Init()");

  fEventManager = STEventManager::Instance();
  
  fCvs_a = fEventManager -> GetCvs_a(); 
  fCvs_b = fEventManager -> GetCvs_b(); 

  if (fBeamPlots){
    SetHist_scaler_ch();
    SetHist_scaler_val();
  }
  
  return kSUCCESS;

}

void
STEventRIDFTask::SetParContainers()
{}

void
STEventRIDFTask::Exec(Option_t *opt)
{
  if (fBeamPlots)
    Fill_scaler_histograms();
}

void
STEventRIDFTask::SetBeamScalersPlot(Bool_t var)
{
  fBeamPlots = var; 
}

void 
STEventRIDFTask::Fill_scaler_histograms()
{

  int neve = 0;
  int max = 1;
  if (fHist_a->GetEntries() == 0) {

    while (estore->GetNextEvent() && neve<max){
      // BEAM
      /*
      for(int i=0 ; i<rawevent->GetNumScaler(); i++){
	TArtRawScalerObject *sca = rawevent->GetScaler(i);
	int id = sca->GetScalerID();
	int ch = sca->GetNumChannel(); 
	int scaval = sca->GetScaler(i);
	//	std::cout << i+1 << " id: " << id << " ch: " << ch << " scaler value: " << scaval << std::endl;
	// control on beamVME trigger
	//      if (id == 84){
	// SpiRIT scalers
	//      if (id == 96){	
	fHist_a->Fill(ch);
	if (ch == 9)
	  fHist_b->Fill(scaval);
	
	//      }
      }
      */

      // KYOTO ARRAY
      /*
       * @file TArtDecoderVmeEasyroc.cc
       * geo:0 High gain ADC
       * geo:1 Low gain ADC
       * geo:2 Leading edge TDC
       * geo:3 Trailing edge TDC
       * geo:4 Scaler
       */    
      for(int i=0;i<rawevent->GetNumSeg();i++){
	TArtRawSegmentObject *seg = rawevent->GetSegment(i);
	if(35 == seg->GetModule()){
	  for(int j=0;j<seg->GetNumData();j++){
	    TArtRawDataObject *d = seg->GetData(j);
	    int geo = d->GetGeo(); 
	    int ch = d->GetCh();
	    int val = d->GetVal(); 
	    std::cout << "geo: " << d->GetGeo() << " ch: " << d->GetCh() << " val: " << d->GetVal() << std::endl;
	    //	    ntp->Fill(geo,ch,val); 
	  }
	  //	  h1->Fill(seg->GetNumData());
	}
      }

      estore->ClearData();
      neve++;
    }
    
    fCvs_a->cd();
    fHist_a->Draw();
    fCvs_a->Modified();
    fCvs_a->Update();
    
    fCvs_b->cd();
    fHist_b->Draw();
    fCvs_b->Modified();
    fCvs_b->Update();
  }
}

void
STEventRIDFTask::SetFilename(TString pathfile)
{

  delete gROOT->GetListOfFiles()->FindObject(pathfile); // clear memory of file name

  std::cout << "ROOT file to open: " << pathfile << std::endl;
  if( gSystem->AccessPathName(pathfile) ) {
    std::cout << std::endl << "File: " << pathfile << " does not exist!!!" << std::endl << std::endl;
    return;
  }

  estore = new TArtEventStore();
  estore->Open(pathfile);

  rawevent = estore->GetRawEventObject();

}

void
STEventRIDFTask::SetHist_scaler_ch()
{
  if (fHist_a)
    {
      fHist_a -> Reset();
      return;
    }

  fCvs_a -> cd();
  fHist_a = new TH1D("a","",33,0,33);
  fHist_a -> SetLineColor(9);
  fHist_a -> SetFillColor(9);
  fHist_a -> SetFillStyle(3002);
  fHist_a -> GetXaxis() -> SetTickLength(0.01);
  fHist_a -> GetXaxis() -> SetTitle("beamVME scaler channels");
  fHist_a -> GetXaxis() -> CenterTitle();
  fHist_a -> GetXaxis() -> SetLabelSize(0.05);
  fHist_a -> GetXaxis() -> SetTitleSize(0.05);
  fHist_a -> GetYaxis() -> SetTickLength(0.01);
  fHist_a -> GetYaxis() -> SetTitle("");
  fHist_a -> GetYaxis() -> CenterTitle();
  fHist_a -> GetYaxis() -> SetLabelSize(0.05);
  fHist_a -> GetYaxis() -> SetTitleSize(0.05);
  fHist_a -> SetMinimum(0);
  fHist_a -> SetStats(0);
  fHist_a -> Draw();
  fCvs_a -> SetGridy();
  fCvs_a -> SetGridx();
}

void 
STEventRIDFTask::SetHist_scaler_val()
{
  if (fHist_b)
    {
      fHist_b -> Reset();
      return;
    }

  fCvs_b -> cd();
  fHist_b = new TH1D("b","",50,-5,45);
  fHist_b -> SetLineColor(9);
  fHist_b -> SetFillColor(9);
  fHist_b -> SetFillStyle(3002);
  fHist_b -> GetXaxis() -> SetTickLength(0.01);
  fHist_b -> GetXaxis() -> SetTitle("scaler value for channel 9");
  fHist_b -> GetXaxis() -> CenterTitle();
  fHist_b -> GetXaxis() -> SetLabelSize(0.05);
  fHist_b -> GetXaxis() -> SetTitleSize(0.05);
  fHist_b -> GetYaxis() -> SetTickLength(0.01);
  fHist_b -> GetYaxis() -> SetTitle("");
  fHist_b -> GetYaxis() -> CenterTitle();
  fHist_b -> GetYaxis() -> SetLabelSize(0.05);
  fHist_b -> GetYaxis() -> SetTitleSize(0.05);
  fHist_b -> SetMinimum(0);
  fHist_b -> SetStats(0);
  fHist_b -> Draw();
  fCvs_b -> SetGridy();
  fCvs_b -> SetGridx();
}

