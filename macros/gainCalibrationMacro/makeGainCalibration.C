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

// Set the output filename with the extension. Checking filename is automatically determined.
TString fOutputFilename = "";

// FPN pedestal range selection threshold
Int_t fFPNThreshold = 5;

// Set the directory path where the data files are.
TString fDataDir = ".";

// Voltage list - Ignored if separated data files is set
Double_t fVoltage[] = {1.0, 2.0, 3.0, 4.0, 5.0};

// Data file list - Ignored if separated data file is set
TString fData[] = {"pulser1.dat",
                   "pulser2.dat",
                   "pulser3.dat",
                   "pulser4.dat",
                   "pulser5.dat"};

// Set use separated data files
Bool_t fUseSeparatedData = kTRUE;

// If set use separated data files, data file list should be set. 
// Voltage list, data file list and data dir above are neglected.
// Only data used in the list file below.
TString fDataList = "list.txt";

//////////////////////////////////////////////////////////
//                                                      //
//   Don't edit the below if you don't know about it.   //
//                                                      //
//////////////////////////////////////////////////////////

void makeGainCalibration() {
  TString workDir = gSystem -> Getenv("VMCWORKDIR");
  TString parameterDir = workDir + "/parameters/";

  STGenerator *fGenerator = new STGenerator("gain");
  fGenerator -> SetUseSeparatedData(fUseSeparatedData);
  fGenerator -> SetParameterFile(parameterDir + fParameterFile);
  fGenerator -> SetFPNPedestal(fFPNThreshold);

  if (!fUseSeparatedData) {
    Int_t numData = (Int_t)(sizeof(fVoltage)/sizeof(Double_t));
    for (Int_t iData = 0; iData < numData; iData++) 
      fGenerator -> AddData(fVoltage[iData], fDataDir + "/" + fData[iData]);
  } else {
    TString fileListWithPath = fDataDir + "/" + fDataList;
    std::ifstream listFile(fileListWithPath.Data());
    TString buffer;
    buffer.ReadLine(listFile);
    while (!buffer.IsNull()) {
      Double_t voltage = buffer.Atof();

      for (Int_t iCobo = 0; iCobo < 12; iCobo++) {
        buffer.ReadLine(listFile);
        fGenerator -> AddData(voltage, buffer, iCobo);
      }

      buffer.ReadLine(listFile);
    }
  }

  fGenerator -> SetOutputFile(fOutputFilename);

  fGenerator -> Print();
  fGenerator -> StartProcess();
}
