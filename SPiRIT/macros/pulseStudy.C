void pulseStudy(TString tag = "urqmd1")
{
  TClonesArray *eventArray;

  TFile* file = new TFile("data/spirit_" + tag + ".raw.root");
  TTree* tree = (TTree*) file -> Get("cbmsim");
  tree -> SetBranchAddress("STRawEvent", &eventArray);
  eventArray -> Print();

  tree -> GetEntry(0);
  STRawEvent *event = (STRawEvent *) eventArray -> At(0);
  event -> PrintPads();

  const Int_t rows        = 108;
  const Int_t layers      = 112;
  const Int_t timeBuckets = 512;
  Int_t count[rows][layers] = {0};
  TH2D* histRL = new TH2D("histRL","",rows,0,rows,layers,0,layers);

  Double_t adcVal;
  STPad*   pad;

  for(Int_t row=0; row<rows; row++){
    for(Int_t layer=0; layer<layers; layer++){

      pad = event -> GetPad(row,layer);
      for(Int_t timeBucket=0; timeBucket<timeBuckets; timeBucket++){
        adcVal = pad -> GetADC(timeBucket);

        if(adcVal>1){
          //cout << setw(5)  << row
          //     << setw(5)  << layer
          //     << setw(5)  << timeBucket 
          //     << setw(15) << adc << endl;

          count[row][layer]++;
          histRL -> Fill(row,layer);
        }

      }
    }
  }

  histRL -> Draw("colz");
  

  Double_t *adc;

  pad = event -> GetPad(54,3);
  adc = pad -> GetADC();
  Double_t tb[timeBuckets];
  for(Int_t i=0; i<timeBuckets; i++) tb[i] = i;

  TGraph *gra = new TGraph(timeBuckets, tb, adc);
  gra -> Draw("APL");
}
