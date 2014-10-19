void pulseStudy
(TString tag         = "urqmd1",
 Int_t   rowIn       = 37, 
 Int_t   layerIn     = 108,
 Bool_t  saveFigures = kFALSE)
{
  gStyle -> SetOptStat(0);
  gStyle -> SetTitleOffset(2.0,"y");
  gStyle -> SetTitleOffset(1.2,"x");
  gStyle -> SetPadLeftMargin(0.15);
  gStyle -> SetPadRightMargin(0.15);

  TString rowInName      = TString::Itoa(rowIn,10);
  TString layerInName    = TString::Itoa(layerIn,10);
  TString histPulseTitle = "row #" + rowInName + ", layer #" + layerInName + ";Time Bucket;ADC";
  TString histPulseSave  = "pulse_" + tag + "_R" + rowInName + "L" + layerInName + ".pdf";
  TString histPulseZSave = "pulse_" + tag + "_R" + rowInName + "L" + layerInName + "_zoom.pdf";

  const Int_t rows        = 108;
  const Int_t layers      = 112;
  const Int_t timeBuckets = 512;

  Int_t count[rows][layers] = {0};

  TClonesArray* eventArray;
  STRawEvent*   event;
  STPad*        pad;

  TH2D* histPlane = new TH2D("histPlane","Pad Plane; Row; Layer",rows,0,rows,layers,0,layers);
  TH1D* histPulse = new TH1D("histPulse",histPulseTitle,timeBuckets,0,timeBuckets);
        histPulse -> SetLineColor(9);
  TH1D* histPulseZ;

  TFile* file = new TFile("../data/spirit_" + tag + ".raw.root");
  TTree* tree = (TTree*) file -> Get("cbmsim");
         tree -> SetBranchAddress("STRawEvent", &eventArray);
         tree -> GetEntry(0);
  event = (STRawEvent *) eventArray -> At(0);




  // -- For All Pads ----------------------------------------------------------
  for(Int_t row=0; row<rows; row++){
  for(Int_t layer=0; layer<layers; layer++){

      pad = event -> GetPad(row,layer);

      for(Int_t timeBucket=0; timeBucket<timeBuckets; timeBucket++){
        Double_t adcVal = pad -> GetADC(timeBucket);
        if(adcVal>1){
          count[row][layer]++;
          histPlane -> Fill(row,layer);
        }
      }
      if(count[row][layer]>40){
        //cout << setw(5) << row << setw(5) << layer << setw(5) << count[row][layer] << endl;
      }
  }
  }




  // -- Pad Plane -------------------------------------------------------------
  TCanvas* cvsPlane = new TCanvas("cvsPlane","",700,700);
           cvsPlane -> SetGrid();
  histPlane -> Draw("colz");
  if(saveFigures) cvsPlane -> SaveAs("figures/PadPlane.pdf");
  




  // -- Pulse -----------------------------------------------------------------
  pad = event -> GetPad(rowIn,layerIn);
  Double_t *adc = pad -> GetADC();
  for(Int_t timeBucket=0; timeBucket<timeBuckets; timeBucket++)
    histPulse -> Fill(timeBucket,adc[timeBucket]);

  TCanvas* cvsPulse = new TCanvas("cvsPulse","",700,700);
  histPulse -> Draw();
  if(saveFigures) cvsPulse -> SaveAs("figures/" + histPulseSave);





  // -- Pulse Zoom ------------------------------------------------------------
  Int_t maximumBin = histPulse -> GetMaximumBin();
  Int_t xLow  = maximumBin - count[rowIn][layerIn];
  Int_t xHigh = maximumBin + count[rowIn][layerIn];
  histPulseZ = new TH1D("histPulseZ",histPulseTitle,xHigh-xLow+1,xLow,xHigh);
  for(Int_t timeBucket=0; timeBucket<timeBuckets; timeBucket++)
    histPulseZ -> Fill(timeBucket,adc[timeBucket]);

  TCanvas* cvsPulseZ = new TCanvas("cvsPulseZ","",700,700);
  histPulseZ -> Draw();
  if(saveFigures) cvsPulseZ -> SaveAs("figures/" + histPulseZSave);
}
