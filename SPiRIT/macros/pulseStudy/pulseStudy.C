void pulseStudy
(TString tag         = "test",
 Int_t   rowIn       = 58, 
 Int_t   layerIn     = 33,
 Bool_t  drawPlane   = kTRUE,
 Bool_t  saveFigures = kTRUE)
{
  gStyle -> SetOptStat(0);
  gStyle -> SetTitleOffset(2.2,"y");
  gStyle -> SetTitleOffset(1.2,"x");
  gStyle -> SetPadLeftMargin(0.15);
  gStyle -> SetPadRightMargin(0.15);
  gStyle -> SetPalette(55);

  TString rowInName      = TString::Itoa(rowIn,10);
  TString layerInName    = TString::Itoa(layerIn,10);
  TString histPulseTitle = "row #" + rowInName + ", layer #" + layerInName + ";Time buckets;ADC channel";
  TString histPulseSave  = "pulse_" + tag + "_R" + rowInName + "L" + layerInName + ".pdf";
  TString histPulseZSave = "pulse_" + tag + "_R" + rowInName + "L" + layerInName + "_zoom.pdf";

  const Int_t rows        = 108;
  const Int_t layers      = 112;
  const Int_t timeBuckets = 512;

  Int_t count[rows][layers] = {0};

  TClonesArray* ppEventArray;
  TClonesArray* rawEventArray;
  STRawEvent*   ppEvent;
  STRawEvent*   rawEvent;
  STPad*        pad;

  TH2D* histPlane = new TH2D("histPlane","Pad Plane; Layer; Row",layers,0,layers,rows,0,rows);
  TH1D* histRawPulse = new TH1D("histRawPulse",histPulseTitle,timeBuckets,0,timeBuckets);
        histRawPulse -> SetLineColor(46);
        histRawPulse -> SetMinimum(0);
  TH1D* histPPPulse = new TH1D("histPPPulse",histPulseTitle,timeBuckets,0,timeBuckets);
        histPPPulse -> SetLineColor(9);
        //histPPPulse -> SetFillColor(9);
  TH1D* histRawPulseZ;
  TH1D* histPPPulseZ;

  TFile* file = new TFile("../data/spirit_" + tag + ".raw.root");
  TTree* tree = (TTree*) file -> Get("cbmsim");
         tree -> SetBranchAddress("STRawEvent", &rawEventArray);
         tree -> SetBranchAddress("PPEvent", &ppEventArray);
         tree -> GetEntry(0);
  rawEvent = (STRawEvent *) rawEventArray -> At(0);
  ppEvent  = (STRawEvent *) ppEventArray -> At(0);




  // -- For All Pads ----------------------------------------------------------
  if(drawPlane)
  {
    for(Int_t row=0; row<rows; row++){
    for(Int_t layer=0; layer<layers; layer++){

        pad = ppEvent -> GetPad(row,layer);

        for(Int_t timeBucket=0; timeBucket<timeBuckets; timeBucket++){
          Double_t adcVal = pad -> GetADC(timeBucket);
          if(adcVal>1){
            count[row][layer]++;
            histPlane -> Fill(layer,row,adcVal);
          }
        }
        if(count[row][layer]>20){
          //cout << setw(5) << row << setw(5) << layer << setw(5) << count[row][layer] << endl;
        }
    }
    }
  }




  // -- Pad Plane -------------------------------------------------------------
  if(drawPlane)
  {
    TCanvas* cvsPlane = new TCanvas("cvsPlane","cp",900,700);
             cvsPlane -> SetGrid();
             cvsPlane -> SetLogz();
    histPlane -> SetMinimum(1000);
    histPlane -> Draw("colz");
    if(saveFigures) cvsPlane -> SaveAs("figures/PadPlane.pdf");
  }
  




  // -- Pulse -----------------------------------------------------------------
  pad = rawEvent -> GetPad(rowIn,layerIn);
  Double_t *rADC = pad -> GetADC();

  pad = ppEvent -> GetPad(rowIn,layerIn);
  Double_t *pADC = pad -> GetADC();

  Double_t c = 0.005;

  for(Int_t timeBucket=0; timeBucket<timeBuckets; timeBucket++) {
    histRawPulse -> Fill(timeBucket,rADC[timeBucket]);
    histPPPulse  -> Fill(timeBucket,c*pADC[timeBucket]);
  }

  TLegend *legend = new TLegend(0.55,0.80,0.85,0.9);
           legend -> AddEntry(histPPPulse,"Electron signal","L");
           legend -> AddEntry(histRawPulse,"GET electronics signal", "L");

  TCanvas* cvsPulse = new TCanvas("cvsPulse","cvspulse",700,700);
  histRawPulse -> Draw();
  histPPPulse -> Draw("same");
  legend -> Draw("same");

  if(saveFigures) cvsPulse -> SaveAs("figures/" + histPulseSave);





  // -- Pulse Zoomed ----------------------------------------------------------
  Int_t maximumBin = histRawPulse -> GetMaximumBin();
  Int_t countIn    = 0;

  pad = rawEvent -> GetPad(rowIn,layerIn);
  for(Int_t timeBucket=0; timeBucket<timeBuckets; timeBucket++) 
    if(pad->GetADC(timeBucket)>0) countIn++;
  //Int_t xLow  = maximumBin - countIn/4;
  //Int_t xHigh = maximumBin + countIn/4;
  Int_t xLow  = 60;
  Int_t xHigh = 120;


  histRawPulseZ = new TH1D("histRawPulseZ",histPulseTitle,xHigh-xLow,xLow,xHigh);
  histRawPulseZ -> SetLineColor(46);
  histRawPulseZ -> SetMinimum(0);
  histPPPulseZ = new TH1D("histPPPulseZ",histPulseTitle,xHigh-xLow,xLow,xHigh);
  histPPPulseZ -> SetLineColor(9);
  //histPPPulseZ -> SetFillColor(9);

  for(Int_t timeBucket=xLow; timeBucket<xHigh; timeBucket++) {
    histRawPulseZ -> Fill(timeBucket,rADC[timeBucket]);
    histPPPulseZ  -> Fill(timeBucket,c*pADC[timeBucket]);
  }

  TCanvas* cvsPulseZ = new TCanvas("cvsPulseZ","cvspulsez",700,700);
  histRawPulseZ -> Draw();
  histPPPulseZ -> Draw("same");
  legend -> Draw("same");
  if(saveFigures) cvsPulseZ -> SaveAs("figures/" + histPulseZSave);

  test();

}

void test()
{
  gStyle -> SetOptStat(0);
  gStyle -> SetTitleOffset(2.2,"y");
  gStyle -> SetTitleOffset(1.2,"x");
  gStyle -> SetPadLeftMargin(0.15);
  gStyle -> SetPadRightMargin(0.15);
  gStyle -> SetPalette(55);



  // parameter setting
  Double_t coulombToEV = 6.241e18; 
  Double_t fADCDynamicRange = 120.e-15;
  Int_t    fADCMax = 4095;
  Int_t    fADCMaxUseable = 3600;
  Int_t    fADCDefualt = 400;
  Double_t pulserConstant = (fADCMaxUseable-fADCDefualt)
                           /(fADCDynamicRange*coulombToEV);


  //pulser setting
  TString workDir = gSystem -> Getenv("SPIRITDIR");
  TString pulserFileName = workDir + "/parameters/Pulser.dat";
  ifstream pulserFile(pulserFileName.Data());
  Double_t pval;
  Int_t i = 0;
  Double_t fPulser[50];
  while(pulserFile >> pval) 
    fPulser[i++] = pulserConstant*pval;




  const Int_t fNTBs = 512;

  Double_t adcI[fNTBs] = {0};
           adcI[100] = 7000.e2;
  Double_t adcO[fNTBs] = {0};

  for(Int_t iTB=0; iTB<fNTBs; iTB++) {
    Double_t val = adcI[iTB];
    Int_t jTB=iTB;
    Int_t kTB=0;
    while(jTB<fNTBs && kTB<i) 
      adcO[jTB++] += val*fPulser[kTB++];
  }
  // AGET chip protection from ZAP board
  for(Int_t iTB=0; iTB<fNTBs; iTB++)
    if(adcO[iTB]>fADCMaxUseable) 
      adcO[iTB] = fADCMaxUseable;
  // set ADC
    while(jTB<fNTBs && kTB<i) 
      adcO[jTB++] += val*fPulser[kTB++];

  for(Int_t iTB=0; iTB<fNTBs; iTB++)
      adcI[iTB] *= pulserConstant;

  TH1D* hist = new TH1D("hist",";Time buckets;ADC channel",60,90,150);
  hist -> SetMaximum(3100);
  TH1D* histI = new TH1D("histI","",512,0,512);
        histI -> SetLineColor(9);
  TH1D* histO = new TH1D("histO","",512,0,512);
        histO -> SetLineColor(46);
  for(Int_t iTB=0; iTB<fNTBs; iTB++) {
    histI -> SetBinContent(iTB,adcI[iTB]);
    histO -> SetBinContent(iTB,adcO[iTB]);
  }

  TCanvas* cvs = new TCanvas("cvs","cvs",700,700);
  hist -> Draw();
  histI -> Draw("same");
  histO -> Draw("same");

  TLegend *legend = new TLegend(0.55,0.80,0.85,0.9);
           legend -> AddEntry(histI,"Electron signal","L");
           legend -> AddEntry(histO,"GET electronics signal", "L");

  legend -> Draw("same");

  cvs-> SaveAs("figures/example.pdf");
}
