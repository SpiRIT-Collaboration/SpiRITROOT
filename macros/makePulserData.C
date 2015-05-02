void makePulserData()
{
  const Int_t nTbs = 512;
  Double_t fADCMaxUseable = 3600;

  GETDecoder *decoder = new GETDecoder("/home/samba/HIMAC/pulser_1.00V_120fC_232ns.graw"); 
  decoder -> SetNumTbs(nTbs);
  GETFrame *frame;
  frame = decoder -> GetFrame();
  frame -> SetFPNPedestal();

  Double_t adcTot[nTbs] = {0};
  Double_t adcValSum = 0;

  Int_t nSignals = 0;

  for(Int_t iAGET=0; iAGET<3; iAGET++){
    for(Int_t iCh=0; iCh<50; iCh++){
      frame -> SubtractPedestal(iAGET,iCh);
      Double_t *adc = frame -> GetADC(iAGET,iCh);
      adcValSum = 0;
      for(Int_t iTb=0; iTb<nTbs; iTb++) {
        adcValSum += adc[iTb];
        adcTot[iTb] += adc[iTb];
      }
      if(adcValSum!=0) nSignals++;
    }
  }

  Int_t maxADCTb = -1;
  Double_t maxADC = 0;
  for(Int_t iTb=0; iTb<nTbs; iTb++) {
    if(adcTot[iTb]>maxADC){ 
      maxADC   = adcTot[iTb];
      maxADCTb = iTb;
    }
  }

  for(Int_t iTb=0; iTb<nTbs; iTb++) 
    adcTot[iTb] = adcTot[iTb]/maxADC;

  TString workDir = gSystem -> Getenv("VMCWORKDIR");
  TString fileName = workDir + "/parameters/PulserExample.dat";
  ofstream file(fileName.Data());
  for(Int_t i=135; i<185; i++)
    file << adcTot[i] << endl;


  TH1D* hist = new TH1D("hist",";Time buckets;ADC channel",nTbs,0,nTbs);
  hist -> SetContent(adcTot);
  hist -> Draw();
  cout << "number of summed signals : " << nSignals << endl;
}
