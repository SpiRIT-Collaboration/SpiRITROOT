void makePedestal() {
  TString filename;

  const Int_t numEvents = 16;
  filename = "CoBo_AsAd0-2014-07-11T18-56-57.670_0000.graw";
  Int_t eventList[numEvents] = {0, 1, 2, 3, 7, 8, 11, 13, 14, 15, 18, 19, 20, 22, 23};
//  filename = "CoBo_AsAd0_2014-07-11T19-34-57.035_0000.graw";
//  Int_t eventList[numEvents] = {0, 7, 10, 11, 13, 14, 16, 17, 19, 20, 21, 24, 25, 26, 29, 30};

  GETDecoder *decoder = new GETDecoder(filename);

  TFile *file = new TFile(filename.Append(".root"), "RECREATE");
  
  Double_t pedestal[2][512] = {{0}};
  TTree *tree = new TTree("pedestal", "");
  tree -> Branch("pedestal", &pedestal[0], "pedestal[512]/D");
  tree -> Branch("pedestalSigma", &pedestal[1], "pedestalSigma[512]/D");

  GETMath *math[4*68*512];
  for (Int_t i = 0; i < 4*68*512; i++)
    math[i] = new GETMath();

  for (Int_t iEvent = 0; iEvent < numEvents; iEvent++) {
    cout << "Start event: " << iEvent << endl;
    GETFrame *frame = decoder -> GetFrame(eventList[iEvent]);

    for (Int_t iAget = 0; iAget < 4; iAget++) {
      for (Int_t iCh = 0; iCh < 68; iCh++) {
        Int_t *adc = frame -> GetRawADC(iAget, iCh);

        for (Int_t iTb = 0; iTb < 512; iTb++) {
          math[iAget*68*512 + iCh*512 + iTb] -> Add(adc[iTb]);
        }
      }
    }
    cout << "Done event: " << iEvent << endl;
  } 

  for (Int_t iAget = 0; iAget < 4; iAget++) {
    for (Int_t iCh = 0; iCh < 68; iCh++) {
      for (Int_t iTb = 0; iTb < 512; iTb++) {
        pedestal[0][iTb] = math[iAget*68*512 + iCh*512 + iTb] -> GetMean();
        pedestal[1][iTb] = math[iAget*68*512 + iCh*512 + iTb] -> GetRMS();
      }

      tree -> Fill();
    }
  }

  file -> Write();
}
