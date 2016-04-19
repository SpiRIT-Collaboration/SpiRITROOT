/**
  * This macro generates comparison plots before and after gain calibration.
 **/

////////////////////////////
//                        //
//   Configuration part   //
//                        //
////////////////////////////

// Put the parameter file name. Path is automatically concatenated.
TString fParameterFile = "ST.parameters.par";

// Voltage list
Double_t fNumVoltages = 20;

// Data file list
TString fData[] = {"pulser1.dat",
                   "pulser2.dat",
                   "pulser3.dat",
                   "pulser4.dat",
                   "pulser5.dat"};

// Set use separated data files
Bool_t fUseSeparatedData = kTRUE;

// Set the raw data file with path. If the file having txt with its extension, the macro will load separated data files in the list automatically.
TString fDataList = "list.txt";

// FPN pedestal range selection threshold
Int_t fFPNThreshold = 5;

//////////////////////////////////////////////////////////
//                                                      //
//   Don't edit the below if you don't know about it.   //
//                                                      //
//////////////////////////////////////////////////////////

STCore ***fCore = NULL;
Int_t fNumPulserData = 0;

void dataCalibrationCheck() {
  gStyle -> SetOptStat(0);
  TString workDir = gSystem -> Getenv("VMCWORKDIR");
  TString parameterDir = workDir + "/parameters/";

  STParReader *fPar = new STParReader(parameterDir + fParameterFile);

  Bool_t fUseSeparatedData = kFALSE;
  if (fDataList.EndsWith(".txt"))
    fUseSeparatedData = kTRUE;

  fCore = new STCore**[2];

  std::vector<Double_t> fVoltages;

  if (!fUseSeparatedData) {
    for (Int_t iCore = 0; iCore < 2; iCore++) {
      fCore[iCore] = new STCore*[fNumVoltages];

      for (Int_t iData = 0; iData < fNumVoltages; iData++)
        fCore[iCore][iData] = new STCore(fData[iData]);
    }
  } else {
    std::vector<TString> dataFiles[12];

    std::ifstream listFile(fDataList.Data());
    TString buffer;
    buffer.ReadLine(listFile);
    while (!buffer.IsNull()) {
      fVoltages.push_back(buffer.Atof());

      for (Int_t iCobo = 0; iCobo < 12; iCobo++) {
        buffer.ReadLine(listFile);
        dataFiles[iCobo].push_back(buffer);
      }

      buffer.ReadLine(listFile);
    }

    fNumPulserData = fVoltages.size();
    for (Int_t iCore = 0; iCore < 2; iCore++) {
      fCore[iCore] = new STCore*[fNumPulserData];

      for (Int_t iData = 0; iData < fNumPulserData; iData++) {
        fCore[iCore][iData] = new STCore();
        fCore[iCore][iData] -> SetUseSeparatedData(fUseSeparatedData);
        
        for (Int_t iCobo = 0; iCobo < 12; iCobo++)
          fCore[iCore][iData] -> AddData(dataFiles[iCobo].at(iData), iCobo);
      }
    }
  }

  TF1 *fReferenceLine = new TF1("referenceLine", "pol2", 0, 4096);
  fReferenceLine -> SetParameters(fPar -> GetDoublePar("GCConstant"), fPar -> GetDoublePar("GCLinear"), fPar -> GetDoublePar("GCQuadratic"));

  for (Int_t iData = 0; iData < fNumPulserData; iData++) {
    fCore[1][iData] -> SetGainCalibrationData(fPar -> GetFilePar(fPar -> GetIntPar("GainCalibrationDataFile")));
    fCore[1][iData] -> SetGainReference(fPar -> GetDoublePar("GCConstant"), fPar -> GetDoublePar("GCLinear"), fPar -> GetDoublePar("GCQuadratic"));

    for (Int_t iCore = 0; iCore < 2; iCore++) {
      fCore[iCore][iData] -> SetUAMap(fPar -> GetFilePar(fPar -> GetIntPar("UAMapFile")));
      fCore[iCore][iData] -> SetAGETMap(fPar -> GetFilePar(fPar -> GetIntPar("AGETMapFile")));
      fCore[iCore][iData] -> SetFPNPedestal(fFPNThreshold);
      fCore[iCore][iData] -> SetData(0);
    }
  }

  Double_t **mean = new Double_t*[2];
  Double_t **sigma = new Double_t*[2];
  for (Int_t iCore = 0; iCore < 2; iCore++) {
    mean[iCore] = new Double_t[fNumPulserData];
    sigma[iCore] = new Double_t[fNumPulserData];
  }

  Double_t *fArrayVoltages = new Double_t[fNumPulserData];
  for (Int_t iData = 0; iData < fNumPulserData; iData++) 
    fArrayVoltages[iData] = fVoltages.at(iData);

  std::vector<Double_t> dummy = fVoltages;
  sort(dummy.begin(), dummy.end());
  Double_t *ybins = new Double_t[2*fNumPulserData];
  for (Int_t iBin = 0; iBin < fNumPulserData; iBin++) {
    ybins[2*iBin] = dummy[iBin] - 0.08;
    ybins[2*iBin + 1] = dummy[iBin] + 0.08;
  }

  TH2D *hist[2] = {NULL};
  hist[0] = new TH2D("hist1", "", 4096, 0, 4096, 2*fNumPulserData - 1, ybins);
  hist[1] = new TH2D("hist2", "", 4096, 0, 4096, 2*fNumPulserData - 1, ybins);

  GETMath *math = new GETMath();
  for (Int_t iCore = 0; iCore < 2; iCore++) {
    for (Int_t iData = 0; iData < fNumPulserData; iData++) {
      math -> Reset();
      STRawEvent *event = fCore[iCore][iData] -> GetRawEvent();

      std::vector<STPad> numPads = *(event -> GetPads());
      for (STPad pad : numPads) {
        Double_t *adc = pad.GetADC();
        Double_t max = -9999;

        Int_t numTbs = fCore[iCore][iData] -> GetNumTbs();
        for (Int_t iTb = 0; iTb < numTbs; iTb++)
          if (max < adc[iTb])
            max = adc[iTb];

        math -> Add(max);
        hist[iCore] -> Fill(max, fVoltages.at(iData));
      }

      mean[iCore][iData] = math -> GetMean();
      sigma[iCore][iData] = math -> GetRMS();
    }
  }

  for (Int_t iCore = 0; iCore < 2; iCore++) {
    cout << (iCore == 0 ? "Not Calib" : "Calib") << endl;
    for (Int_t iData = 0; iData < fNumPulserData; iData++) {
      cout << fVoltages.at(iData) << " Mean: " << mean[iCore][iData] << " sigma: " << sigma[iCore][iData] << endl;
    }
  }

  TCanvas *fCvs = new TCanvas("cvs", "", 1200, 560);
  fCvs -> Divide(2, 1);
  fCvs -> cd(1);
  gPad -> SetRightMargin(gPad -> GetRightMargin() + 0.01);
  gPad -> SetLeftMargin(gPad -> GetRightMargin() + 0.01);
  hist[0] -> SetTitle("Before gain calibration");
  hist[0] -> GetXaxis() -> SetTitle("ADC (ADC Ch.)");
  hist[0] -> GetXaxis() -> SetTitleOffset(hist[0] -> GetXaxis() -> GetTitleOffset() + 0.5);
  hist[0] -> GetXaxis() -> CenterTitle();
  hist[0] -> GetYaxis() -> SetTitle("Pulser Voltage (V)");
  hist[0] -> GetYaxis() -> SetTitleOffset(hist[0] -> GetYaxis() -> GetTitleOffset() + 0.4);
  hist[0] -> GetYaxis() -> CenterTitle();
  hist[0] -> Draw("colz");
  fReferenceLine -> Draw("same");
  fCvs -> cd(2);
  gPad -> SetRightMargin(gPad -> GetRightMargin() + 0.01);
  gPad -> SetLeftMargin(gPad -> GetRightMargin() + 0.01);
  hist[1] -> SetTitle("After gain calibration");
  hist[1] -> GetXaxis() -> SetTitle("ADC (ADC Ch.)");
  hist[1] -> GetXaxis() -> SetTitleOffset(hist[1] -> GetXaxis() -> GetTitleOffset() + 0.5);
  hist[1] -> GetXaxis() -> CenterTitle();
  hist[1] -> GetYaxis() -> SetTitle("Pulser Voltage (V)");
  hist[1] -> GetYaxis() -> SetTitleOffset(hist[1] -> GetYaxis() -> GetTitleOffset() + 0.4);
  hist[1] -> GetYaxis() -> CenterTitle();
  hist[1] -> Draw("colz");
  fReferenceLine -> Draw("same");
}
