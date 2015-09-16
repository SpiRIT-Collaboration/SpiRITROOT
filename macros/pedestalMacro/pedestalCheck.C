/**
  * Macro for drawing pedestal checking data
  * This draws
  * - summary of all pedestal data
  * - 2D mean and RMS map of raw and FPN subtracted data (when 2D map is on)
  * - summary of a pad by clicking the pad (when 2D map is on)
 **/

////////////////////////////
//                        //
//   Configuration part   //
//                        //
////////////////////////////

// Pedestal checking file
TString fPedestalCheckingFile = "";

// If kFALSE is set, 2D maps are not generated.
Bool_t fIsTurnOnPadplane = kTRUE;

//////////////////////////////////////////////////////////
//                                                      //
//   Don't edit the below if you don't know about it.   //
//                                                      //
//////////////////////////////////////////////////////////

TFile *fPCFile = NULL;

TCanvas *fSummaryCanvas = NULL;
TH1D *fSummaryHist[4] = {NULL};

TCanvas *fPadplaneCvs[4] = {NULL};
TH2D *fPadplaneHist[4] = {NULL};

Double_t fValueMin[4] = {1.E100, 1.E100, 1.E100, 1.E100};
Double_t fValueMax[4] = {-1.E100, -1.E100, -1.E100, -1.E100};

Bool_t fLog = kTRUE;

void ToggleLog() {
  fLog = !fLog;

  for (Int_t iCvs = 0; iCvs < 4; iCvs++) {
    fSummaryCanvas -> cd(iCvs + 1) -> SetLogy(fLog);

    if (!fLog) {
      fSummaryCanvas -> cd(iCvs + 1) -> SetLeftMargin(0.15);
      fSummaryHist[iCvs] -> GetYaxis() -> SetTitleOffset(1.05);
    } else {
      fSummaryCanvas -> cd(iCvs + 1) -> SetLeftMargin(0.1);
      fSummaryHist[iCvs] -> GetYaxis() -> SetTitleOffset(0.80);
    }

    fSummaryCanvas -> cd(iCvs + 1) -> Modified();
    fSummaryCanvas -> cd(iCvs + 1) -> Update();
    fSummaryCanvas -> cd(iCvs + 1) -> Modified();
    fSummaryCanvas -> cd(iCvs + 1) -> Update();
  }

  if (fIsTurnOnPadplane) {
    for (Int_t iCvs = 0; iCvs < 4; iCvs++) {
      fPadplaneCvs[iCvs] -> SetLogz(fLog);

      fPadplaneHist[iCvs] -> SetMinimum(fValueMin[iCvs]);
      fPadplaneHist[iCvs] -> SetMaximum(fValueMax[iCvs]);

      fPadplaneCvs[iCvs] -> Modified();
      fPadplaneCvs[iCvs] -> Update();
    }
  }
}

void SaveAll() {
  fSummaryCanvas -> SaveAs("summary.png");
  fPadplaneCvs[0] -> SaveAs("ADCmean_bs.png");
  fPadplaneCvs[1] -> SaveAs("ADCRMS_bs.png");
  fPadplaneCvs[2] -> SaveAs("ADCmean_as.png");
  fPadplaneCvs[3] -> SaveAs("ADCRMS_as.png");
}

void pedestalCheck() {
  fPCFile = new TFile(fPedestalCheckingFile);
  TTree *fPCTree = (TTree *) fPCFile -> Get("PedestalData");

  gStyle -> SetOptStat(111110);
  gStyle -> SetStatStyle(0);
  gStyle -> SetStatX(0.95);
  gStyle -> SetStatY(0.91);
  gStyle -> SetStatW(0.35);
  gStyle -> SetStatH(0.25);
  gStyle -> SetStatFontSize(0.06);

  fSummaryCanvas = new TCanvas("SummaryCanvas", "", 1000, 700);
  fSummaryCanvas -> Divide(2, 2);

  TString fTreeName[4] = {"meanBS", "sigmaBS", "meanAS", "sigmaAS"};
  TString fHistName[4] = {"Raw data", "Raw data", "FPN subtracted data", "FPN subtracted data"};
  TString fXTitle[4] = {"Mean ADC (ADC Ch.)", "RMS (ADC Ch.)", "Mean ADC (ADC Ch.)", "RMS (ADC Ch.)"};
  for (Int_t iCvs = 0; iCvs < 4; iCvs++) {
    fSummaryCanvas -> cd(iCvs + 1) -> SetLeftMargin(0.10);
    fSummaryCanvas -> cd(iCvs + 1) -> SetRightMargin(0.05);
    fSummaryCanvas -> cd(iCvs + 1) -> SetTopMargin(0.09);
    fSummaryCanvas -> cd(iCvs + 1) -> SetBottomMargin(0.10);

    fSummaryCanvas -> cd(iCvs + 1);
    fPCTree -> Draw(Form("%s >> %sHist", fTreeName[iCvs].Data(), fTreeName[iCvs].Data()));
  
    TIter next(fSummaryCanvas -> cd(iCvs + 1) -> GetListOfPrimitives());
    while (TObject *obj = next()) {
      TString name = obj -> GetName();
      if (name.EqualTo(Form("%sHist", fTreeName[iCvs].Data()))) {
        fSummaryHist[iCvs] = (TH1D *) obj;
        break;
      }
    }

    fSummaryHist[iCvs] -> SetTitleSize(0.05);
    fSummaryHist[iCvs] -> SetTitle(fHistName[iCvs]);
    fSummaryHist[iCvs] -> SetFillColor(kAzure - 2);
    fSummaryHist[iCvs] -> SetFillStyle(3003);
    fSummaryHist[iCvs] -> GetXaxis() -> SetTitle(fXTitle[iCvs]);
    fSummaryHist[iCvs] -> GetXaxis() -> SetTitleSize(0.06);
    fSummaryHist[iCvs] -> GetXaxis() -> SetTitleOffset(0.75);
    fSummaryHist[iCvs] -> GetXaxis() -> CenterTitle();
    fSummaryHist[iCvs] -> GetYaxis() -> SetTitle("Counts");
    fSummaryHist[iCvs] -> GetYaxis() -> SetTitleSize(0.06);
    fSummaryHist[iCvs] -> GetYaxis() -> SetTitleOffset(0.80);
    fSummaryHist[iCvs] -> GetYaxis() -> CenterTitle();
  }

  if (fIsTurnOnPadplane) {
    STPlot *fPlot = new STPlot();

    TString f2DHistName[4] = {"ADC mean (Raw data)", "ADC RMS (Raw data)", "ADC mean (FPN subtracted)", "ADC RMS (FPN subtracted)"};
    for (Int_t iCvs = 0; iCvs < 4; iCvs++) {
      fPlot -> SetPadplaneTitle(Form("%s", f2DHistName[iCvs].Data()));
      fPadplaneCvs[iCvs] = fPlot -> GetPadplaneCanvas();
      fPadplaneCvs[iCvs] -> AddExec("DrawPad", ".x DrawPadHelper.C");

      TIter next(fPadplaneCvs[iCvs] -> GetListOfPrimitives());
      while (TObject *obj = next()) {
        if (obj -> InheritsFrom("TH2D")) {
          fPadplaneHist[iCvs] = (TH2D *) obj;
          fPadplaneHist[iCvs] -> SetContour(50);
          break;
        }
      }
    }

    Int_t row = -1, layer = -1;
    Double_t value[4];

    fPCTree -> SetBranchAddress("row", &row);
    fPCTree -> SetBranchAddress("layer", &layer);
    fPCTree -> SetBranchAddress("meanBS", &value[0]);
    fPCTree -> SetBranchAddress("sigmaBS", &value[1]);
    fPCTree -> SetBranchAddress("meanAS", &value[2]);
    fPCTree -> SetBranchAddress("sigmaAS", &value[3]);

    Int_t numEntries = fPCTree -> GetEntries();
    for (Int_t iEntry = 0; iEntry < numEntries; iEntry++) {
      fPCTree -> GetEntry(iEntry);
      
      for (Int_t iCvs = 0; iCvs < 4; iCvs++) {
        fPadplaneHist[iCvs] -> SetBinContent(layer + 1, row + 1, value[iCvs]);

        if (value[iCvs] < fValueMin[iCvs]) fValueMin[iCvs] = value[iCvs];
        if (value[iCvs] > fValueMax[iCvs]) fValueMax[iCvs] = value[iCvs];
      }
    }
  }

  ToggleLog();

  cout << endl;
  cout << "///////////////////////////////////////////////////////////////////" << endl;
  cout << "//                                                               //" << endl;
  cout << "//  == Type \033[1;31mToggleLog()\033[0m to turn on and off logarithmic scaling.  //" << endl;
  cout << "//                                                               //" << endl;
  cout << "//  == Type \033[1;31mSaveAll()\033[0m to save all canvases except for a pad.     //" << endl;
  cout << "//                                                               //" << endl;
  cout << "///////////////////////////////////////////////////////////////////" << endl;
  cout << endl;
}
