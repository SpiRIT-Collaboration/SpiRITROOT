/**
  * Macro for generating gating grid noise data.
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

// The number of time buckets in a channel
Int_t fNumTbs = 512;

// Set the directory path where the data files are.
TString fDataDir = ".";

// Data file list - Ignored if separated data file is set
TString fData = "ggNoise.dat";

// Set use separated data files
Bool_t fUseSeparatedData = kTRUE;

// If set use separated data files, data file list should be set. 
// Voltage list, data file list and data dir above are neglected.
// Only data used in the list file below.
TString fDataList = "";

//////////////////////////////////////////////////////////
//                                                      //
//   Don't edit the below if you don't know about it.   //
//                                                      //
//////////////////////////////////////////////////////////

void makeGGNoiseData() {
  TString workDir = gSystem -> Getenv("VMCWORKDIR");
  TString parameterDir = workDir + "/parameters/";

  STGenerator *fGenerator = new STGenerator("ggNoise");
  fGenerator -> SetUseSeparatedData(fUseSeparatedData);
  fGenerator -> SetParameterFile(parameterDir + fParameterFile);
  fGenerator -> SetFPNPedestal(fFPNThreshold);

  if (!fUseSeparatedData) 
    fGenerator -> AddData(fDataDir + "/" + fData);
  else {
    std::ifstream listFile(fDataList.Data());
    TString dataFileWithPath;
    Int_t iCobo = -1;
    while (dataFileWithPath.ReadLine(listFile)) {
      if (dataFileWithPath.Contains("s."))
        fGenerator -> AddData(dataFileWithPath, iCobo);
      else {
        iCobo++;
        fGenerator -> AddData(dataFileWithPath, iCobo);
      }
    }
  }

  fGenerator -> SetOutputFile(fOutputFilename);

  fGenerator -> Print();
  fGenerator -> StartProcess();
}
