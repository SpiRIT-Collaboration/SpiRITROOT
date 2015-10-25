/**
  * This macro provides only one 2D pad plane with signal plots on clicked pad.
  * (Either gain not calibrated or calibrated plot.)
 **/

////////////////////////////
//                        //
//   Configuration part   //
//                        //
////////////////////////////

// Put the parameter file name. Path is automatically concatenated.
TString fParameterFile = "ST.parameters.par";

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

void next(Int_t eventID = -1) {
  fPlot -> DrawPadplane(eventID);
  fPlot -> DrawSideview(fCore -> GetEventID());
}

void quickViewer() {
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
  fPlot -> DrawPadplane();
  fPlot -> DrawSideview(fCore -> GetEventID());

  cout << endl;
  cout << "////////////////////////////////////////////////////////////////////////" << endl;
  cout << "//                                                                    //" << endl;
  cout << "//  == Type \033[1;31mnext()\033[0m for displaying the next event.                     //" << endl;
  cout << "//     You can put the event ID as an argument to spectfy the event.  //" << endl;
  cout << "//     ex) \033[1;31mnext()\033[0m or \033[1;31mnext(\033[1;33m5\033[1;31m)\033[0m                                          //" << endl;
  cout << "//                                                                    //" << endl;
  cout << "//  == Click the pad if you want to investigate the signal shape.     //" << endl;
  cout << "//                                                                    //" << endl;
  cout << "////////////////////////////////////////////////////////////////////////" << endl;
  cout << endl;
}
