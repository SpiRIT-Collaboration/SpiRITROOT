/**
  * This macro provides the following features.
  * - Display event in 2D pad plane and show raw and pedestal subtracted signals when the pad clicked.
  * - When gain calibration data is set, this gives two 2D pad plane canvases of gain not calibrated and calibrated event display.
  * - One can check how the pulse changes by the gain calibration process by setting pulser data and gain calibration data.
  * - One can check the pedestal subtraction is properly done pad by pad in an event by setting pedestal data.
 **/

////////////////////////////
//                        //
//   Configuration part   //
//                        //
////////////////////////////

// Put the parameter file name. Path is automatically concatenated.
TString fParameterFile = "ST.parameters.par";

// Set the raw data file with path
TString fDataFile = "anEvent_0421.dat";
//TString fDataFile = "";

// Set the gain calibration data file. If not, assign "".
TString fGainCalibrationData = "";

// Set the reference values for gain calibration
Double_t fReferenceConstant = 0.0410293;
Double_t fReferenceLinear = 1.69946E-3;
Double_t fReferenceQuadratic = -3.05356E-8;

// FPN pedestal range selection threshold
Int_t fFPNThreshold = 5;

//////////////////////////////////////////////////////////
//                                                      //
//   Don't edit the below if you don't know about it.   //
//                                                      //
//////////////////////////////////////////////////////////

STPlot *fPlot[2] = {NULL};

void next(Int_t eventID = -1) {
  fPlot[0] -> DrawPadplane(eventID);
  if (fPlot[1] != NULL)
    fPlot[1] -> DrawPadplane(eventID);
}

void eventCheck() {
  TString workDir = gSystem -> Getenv("VMCWORKDIR");
  TString parameterDir = workDir + "/parameters/";

  STParReader *fPar = new STParReader(parameterDir + fParameterFile);

  STCore *fCore = new STCore(fDataFile);
  fCore -> SetUAMap(fPar -> GetFilePar(fPar -> GetIntPar("UAMapFile")));
  fCore -> SetAGETMap(fPar -> GetFilePar(fPar -> GetIntPar("AGETMapFile")));
  fCore -> SetFPNPedestal(fFPNThreshold);
  fCore -> SetData(0);

  fPlot[0] = fCore -> GetSTPlot();
  fPlot[0] -> SetPadplaneTitle("Event ID: %d (Gain not calibrated)");
  fPlot[0] -> DrawPadplane();

  if (!fGainCalibrationData.EqualTo("")) {
    fCore = new STCore(fDataFile);
    fCore -> SetGainCalibrationData(fGainCalibrationData);
    fCore -> SetGainReference(fReferenceConstant, fReferenceLinear, fReferenceQuadratic);
    fCore -> SetUAMap(fPar -> GetFilePar(fPar -> GetIntPar("UAMapFile")));
    fCore -> SetAGETMap(fPar -> GetFilePar(fPar -> GetIntPar("AGETMapFile")));
    fCore -> SetFPNPedestal(fFPNThreshold);
    fCore -> SetData(0);

    fPlot[1] = fCore -> GetSTPlot();
    fPlot[1] -> SetPadplaneTitle("Event ID: %d (Gain calibrated)");
    fPlot[1] -> DrawPadplane();
  }

  cout << endl;
  cout << "////////////////////////////////////////////////////////////////////////" << endl;
  cout << "//                                                                    //" << endl;
  cout << "//  == Type \033[1;31mnext()\033[0m for displaying the next event.                     //" << endl;
  cout << "//     You can put the event ID as an argument to spectfy the event.  //" << endl;
  cout << "//     ex) \033[1;31mnext()\033[0m or \033[1;31mnext(\033[1;33m5\033[1;31m)\033[0m                                          //" << endl;
  cout << "//                                                                    //" << endl;
  cout << "////////////////////////////////////////////////////////////////////////" << endl;
  cout << endl;
}

