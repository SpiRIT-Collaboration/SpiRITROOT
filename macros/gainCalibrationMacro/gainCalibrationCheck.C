/**
  * This macro generates the following plots for fitting parameters A, B, and C 
  * - 1D distributions
  * - 2D map 
  * 
  * By clicking a pad on 2D map it will draw detailed information about the calibration
  * when gain calibration checking file is set with turning on pad plane.
 **/

////////////////////////////
//                        //
//   Configuration part   //
//                        //
////////////////////////////

// Gain calibration file
TString fGainCalibrationFile = "";


// Gain calibration checking file generated simultaneousely with calibration file. It contains chcecking in the filename.
TString fGainCalibrationCheckingFile = "";

// If kFALSE is set, 2D maps are not generated.
Bool_t fIsTurnOnPadplane = kTRUE;

//////////////////////////////////////////////////////////
//                                                      //
//   Don't edit the below if you don't know about it.   //
//                                                      //
//////////////////////////////////////////////////////////

TCanvas *fParCanvas = NULL;
TH1D *fParHist[3] = {NULL};

TCanvas *fPadplaneCvs[3] = {NULL};
TH2D *fPadplaneHist[3] = {NULL};

Double_t fParMin[3] = {1.E100, 1.E100, 1.E100};
Double_t fParMax[3] = {-1.E100, -1.E100, -1.E100};

Bool_t fLog = kTRUE;

TFile *fGCCFile = NULL;

void ToggleLog() {
  fLog = !fLog;

  for (Int_t iCvs = 0; iCvs < 3; iCvs++) {
    fParCanvas -> cd(iCvs + 1) -> SetLogy(fLog);

    if (!fLog) {
      fParCanvas -> cd(iCvs + 1) -> SetLeftMargin(0.15);
      fParHist[iCvs] -> GetYaxis() -> SetTitleOffset(1.05);
    } else {
      fParCanvas -> cd(iCvs + 1) -> SetLeftMargin(0.1);
      fParHist[iCvs] -> GetYaxis() -> SetTitleOffset(0.80);
    }

    fParCanvas -> cd(iCvs + 1) -> Modified();
    fParCanvas -> cd(iCvs + 1) -> Update();
    fParCanvas -> cd(iCvs + 1) -> Modified();
    fParCanvas -> cd(iCvs + 1) -> Update();
  }

  if (fIsTurnOnPadplane) {
    for (Int_t iCvs = 0; iCvs < 3; iCvs++) {
      fPadplaneCvs[iCvs] -> SetLogz(fLog);

      fPadplaneHist[iCvs] -> GetZaxis() -> SetLimits(fParMin[iCvs], fParMax[iCvs]);
      fPadplaneHist[iCvs] -> GetZaxis() -> SetRangeUser(fParMin[iCvs], fParMax[iCvs]);
      fPadplaneHist[iCvs] -> SetContour(50);
//      ((TPaletteAxis *) fPadplaneHist[iCvs] -> FindObject("palette")) -> GetAxis() -> SetMaxDigits(2);

      fPadplaneCvs[iCvs] -> Modified();
      fPadplaneCvs[iCvs] -> Update();
    }
  }
}

void SaveAll() {
  fParCanvas -> SaveAs("parameters.png");
  fPadplaneCvs[0] -> SaveAs("parameterA.png");
  fPadplaneCvs[1] -> SaveAs("parameterB.png");
  fPadplaneCvs[2] -> SaveAs("parameterC.png");
}

void gainCalibrationCheck() {
  TString fName[3] = {"A", "B", "C"};
  TString fFullName[3] = {"constant", "linear", "quadratic"};

  TChain *fGCFile = new TChain("GainCalibrationData");
  fGCFile -> Add(fGainCalibrationFile);

  gStyle -> SetOptStat(1110);
  gStyle -> SetStatStyle(0);
  gStyle -> SetStatX(0.95);
  gStyle -> SetStatY(0.95);
  gStyle -> SetStatW(0.35);
  gStyle -> SetStatH(0.25);
  gStyle -> SetStatFontSize(0.06);
  fParCanvas = new TCanvas("paramterCanvas", "", 1200, 350);
  fParCanvas -> Divide(3, 1);

  for (Int_t iCvs = 0; iCvs < 3; iCvs++) {
    fParCanvas -> cd(iCvs + 1) -> SetLeftMargin(0.10);
    fParCanvas -> cd(iCvs + 1) -> SetRightMargin(0.05);
    fParCanvas -> cd(iCvs + 1) -> SetTopMargin(0.05);
    fParCanvas -> cd(iCvs + 1) -> SetBottomMargin(0.10);

    fParCanvas -> cd(iCvs + 1);
    fGCFile -> Draw(Form("%s >> %sHist", fFullName[iCvs].Data(), fFullName[iCvs].Data()));
  
    TIter next(fParCanvas -> cd(iCvs + 1) -> GetListOfPrimitives());
    while (TObject *obj = next()) {
      TString name = obj -> GetName();
      if (name.EqualTo(Form("%sHist", fFullName[iCvs].Data()))) {
        fParHist[iCvs] = (TH1D *) obj;
        break;
      }
    }

    fParHist[iCvs] -> SetTitle("");
    fParHist[iCvs] -> SetFillColor(kAzure - 2);
    fParHist[iCvs] -> SetFillStyle(3003);
    fParHist[iCvs] -> GetXaxis() -> SetTitle(fName[iCvs]);
    fParHist[iCvs] -> GetXaxis() -> SetTitleSize(0.06);
    fParHist[iCvs] -> GetXaxis() -> SetTitleOffset(0.75);
    fParHist[iCvs] -> GetXaxis() -> CenterTitle();
    fParHist[iCvs] -> GetYaxis() -> SetTitle("Counts");
    fParHist[iCvs] -> GetYaxis() -> SetTitleSize(0.06);
    fParHist[iCvs] -> GetYaxis() -> SetTitleOffset(0.80);
    fParHist[iCvs] -> GetYaxis() -> CenterTitle();
  }

  if (fIsTurnOnPadplane) {
    if (!fGainCalibrationCheckingFile.EqualTo(""))
      fGCCFile = new TFile(fGainCalibrationCheckingFile);

    STPlot *fPlot = new STPlot();

    for (Int_t iCvs = 0; iCvs < 3; iCvs++) {
      fPlot -> SetPadplaneTitle(Form("%s in y = A + Bx + Cx^{2}", fName[iCvs].Data()));
      fPadplaneCvs[iCvs] = fPlot -> GetPadplaneCanvas();
      if (!fGainCalibrationCheckingFile.EqualTo(""))
        fPadplaneCvs[iCvs] -> AddExec("DrawPad", ".x DrawPadHelper.C");

      TIter next(fPadplaneCvs[iCvs] -> GetListOfPrimitives());
      while (TObject *obj = next()) {
        if (obj -> InheritsFrom("TH2D")) {
          fPadplaneHist[iCvs] = (TH2D *) obj;
          break;
        }
      }
    }

    Int_t row = -1, layer = -1;
    Double_t par[3] = {-9999, -9999, -9999};

    fGCFile -> SetBranchAddress("padRow", &row);
    fGCFile -> SetBranchAddress("padLayer", &layer);
    fGCFile -> SetBranchAddress("constant", &par[0]);
    fGCFile -> SetBranchAddress("linear", &par[1]);
    fGCFile -> SetBranchAddress("quadratic", &par[2]);

    Int_t numEntries = fGCFile -> GetEntries();
    for (Int_t iEntry = 0; iEntry < numEntries; iEntry++) {
      fGCFile -> GetEntry(iEntry);
      
      for (Int_t iCvs = 0; iCvs < 3; iCvs++) {
        fPadplaneHist[iCvs] -> SetBinContent(layer + 1, row + 1, par[iCvs]);

        if (par[iCvs] < fParMin[iCvs]) fParMin[iCvs] = par[iCvs];
        if (par[iCvs] > fParMax[iCvs]) fParMax[iCvs] = par[iCvs];
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
