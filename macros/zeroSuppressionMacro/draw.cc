void draw() {
  auto file = new TFile("run2543.zeroSuppressed.root");
  auto tree = (TTree *) file -> Get("tree");

  Int_t eventid;
  std::vector<STSlimPad> padd;
  std::vector<STSlimPad> *pads = &padd;

  tree -> SetBranchAddress("eventid", &eventid);
  tree -> SetBranchAddress("pads", &pads);

  auto event = new STRawEvent();
  tree -> GetEntry(0);
  
  event -> SetEventID(eventid);
  Int_t numPads = pads -> size();
  auto fatPad = new STPad();
  for (Int_t iPad = 0; iPad < numPads; iPad++) {
    fatPad -> Clear();
    STSlimPad pad = pads -> at(iPad);
    
    Int_t row = pad.id%108;
    Int_t layer = pad.id/108;

    fatPad -> SetRow(row);
    fatPad -> SetLayer(layer);

    Int_t numPoints = pad.tb.size();
    Int_t pointer = 0;
    for (Int_t iTb = 0; iTb < numPoints; iTb += 2) {
      for (Int_t iData = 0; iData < pad.tb.at(iTb + 1); iData++)
        fatPad -> SetADC(pad.tb.at(iTb) + iData, pad.adc.at(pointer + iData));

      pointer += pad.tb.at(iTb + 1);
    }

    fatPad -> SetPedestalSubtracted(kTRUE);
    event -> SetPad(fatPad);
  }

  auto plot = new STPlot();
  plot -> SetNumTbs(270);
  plot -> SetEvent(event);
  plot -> DrawPadplane();
}
