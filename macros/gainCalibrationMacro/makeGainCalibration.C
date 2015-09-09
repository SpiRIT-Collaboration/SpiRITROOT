/**
  * Macro for generating gain calibration data.
 **/

////////////////////////////
//                        //
//   Configuration part   //
//                        //
////////////////////////////

// Put the parameter file name. Path is automatically concatenated.
TString fParameterFile = "ST.parameters.par";

// Set the directory path where the data files are.
TString fDataDir = "/path/to/data/dir/";

// Set the output filename with the extension. Checking filename is automatically determined.
TString fOutputFilename = "";

// FPN pedestal range selection threshold
Int_t fFPNThreshold = 5;

// Voltage list
Double_t fVoltage[] = {1.0, 2.0, 3.0, 4.0, 5.0};

// Data file list
TString fData[] = {"pulser1.dat",
                   "pulser2.dat",
                   "pulser3.dat",
                   "pulser4.dat",
                   "pulser5.dat"};

//////////////////////////////////////////////////////////
//                                                      //
//   Don't edit the below if you don't know about it.   //
//                                                      //
//////////////////////////////////////////////////////////

void makeGainCalibration() {
  TString workDir = gSystem -> Getenv("VMCWORKDIR");
  TString parameterDir = workDir + "/parameters/";

  STGenerator *fGenerator = new STGenerator("gain");
  fGenerator -> SetParameterFile(parameterDir + fParameterFile);
  fGenerator -> SetFPNPedestal(fFPNThreshold);

  Int_t numData = (Int_t)(sizeof(fVoltage)/sizeof(Double_t));
  for (Int_t iData = 0; iData < numData; iData++) 
    fGenerator -> AddData(fVoltage[iData], fDataDir + "/" + fData[iData]);

  fGenerator -> SetOutputFile(fOutputFilename);

  fGenerator -> Print();
  fGenerator -> StartProcess();
}
