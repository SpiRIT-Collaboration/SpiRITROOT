/**
  * This macro dumps all 2D displays of top and side views.
  * (Either gain not calibrated or calibrated plot.)
  *
  * Make sure to put the correct run number.
 **/

////////////////////////////
//                        //
//   Configuration part   //
//                        //
////////////////////////////

// Put the parameter file name. Path is automatically concatenated.
TString fParameterFile = "ST.parameters.par";

// Set the numbers.
Int_t fRunNo = ###; // Run number. This macro will create a folder named as 'run_fRunNo' and save images in it.
Int_t fStart = 0; // Start event number. Both 0 means from 0 to the end.
Int_t fEnd = 0;   // End event number. 

// Set the raw data file with path. If the file having txt with its extension, the macro will load separated data files in the list automatically.
TString fDataFile = "";

// Set use the gain calibration data file.
Bool_t fUseGainCalibration = kTRUE;

// FPN pedestal range selection threshold
Int_t fFPNThreshold = 5;

//////////////////////////////////////////////////////////
//                                                      //
//   Don't edit the below if you don't know about it.   //
//                                                      //
//////////////////////////////////////////////////////////

STCore *fCore = NULL;
STPlot *fPlot = NULL;

TCanvas *fCvs = NULL;

TCanvas *next(Int_t eventID = -1) {
  fCvs = fPlot -> DrawPadplane(eventID);
  fCvs -> SaveAs(Form("run_%04d/event_%d_top.png", fRunNo, fCore -> GetEventID()));
  fCvs = fPlot -> DrawSideview(fCore -> GetEventID());
  fCvs -> SaveAs(Form("run_%04d/event_%d_side.png", fRunNo, fCore -> GetEventID()));

  return fCvs;
}

void dumpAllEvents() {
  gSystem -> Exec(Form("mkdir -p run_%04d", fRunNo));
  TString workDir = gSystem -> Getenv("VMCWORKDIR");
  TString parameterDir = workDir + "/parameters/";

  STParReader *fPar = new STParReader(parameterDir + fParameterFile);

  Bool_t fUseSeparatedData = kFALSE;
  if (fDataFile.EndsWith(".txt"))
    fUseSeparatedData = kTRUE;

  if (!fUseSeparatedData) {
    fCore = new STCore(fDataFile);
  } else {
    fCore = new STCore();
    fCore -> SetUseSeparatedData(fUseSeparatedData);

    TString dataFileWithPath = fDataFile;
    std::ifstream listFile(dataFileWithPath.Data());
    TString buffer;
    Int_t iCobo = -1;
    while (dataFileWithPath.ReadLine(listFile)) {
      if (dataFileWithPath.Contains("s."))
        fCore -> AddData(dataFileWithPath, iCobo);
      else {
        iCobo++;
        fCore -> AddData(dataFileWithPath, iCobo);
      }
    }
  }

  if (fUseGainCalibration) {
    fCore -> SetGainCalibrationData(fPar -> GetFilePar(fPar -> GetIntPar("GainCalibrationDataFile")));
    fCore -> SetGainReference(fPar -> GetDoublePar("GCConstant"), fPar -> GetDoublePar("GCLinear"), fPar -> GetDoublePar("GCQuadratic"));
  }

  fCore -> SetUAMap(fPar -> GetFilePar(fPar -> GetIntPar("UAMapFile")));
  fCore -> SetAGETMap(fPar -> GetFilePar(fPar -> GetIntPar("AGETMapFile")));
  fCore -> SetFPNPedestal(fFPNThreshold);
  fCore -> SetData(0);

  fPlot = fCore -> GetSTPlot();
  if (fUseGainCalibration) {
    fPlot -> SetPadplaneTitle("Event ID: %d (Gain calibrated) - Top view");
    fPlot -> SetSideviewTitle("Event ID: %d (Gain calibrated) - Beam right view");
  } else {
    fPlot -> SetPadplaneTitle("Event ID: %d (Gain not calibrated) - Top view");
    fPlot -> SetSideviewTitle("Event ID: %d (Gain not calibrated) - Beam right view");
  }

  if (fStart == 0 && fEnd == 0)
    while (next());
  else {
    for (Int_t iEvent = fStart; iEvent < fEnd; iEvent++)
      next(iEvent);
  }
}
