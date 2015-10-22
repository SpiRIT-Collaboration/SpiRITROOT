/**
  * Macro for generating pedestal checking data
 **/

////////////////////////////
//                        //
//   Configuration part   //
//                        //
////////////////////////////

// Put the parameter file name. Path is automatically concatenated.
TString fParameterFile = "ST.parameters.RIKEN_20151021.par";

// Pedestal data file. You can set one merged data file or the list file ending with txt.
TString fData = "list_run0460.txt";

// Set the output filename with the extension.
TString fOutputFilename = "PedestalData.root";

// FPN pedestal range selection threshold
Int_t fFPNThreshold = 5;

//////////////////////////////////////////////////////////
//                                                      //
//   Don't edit the below if you don't know about it.   //
//                                                      //
//////////////////////////////////////////////////////////

void makePedestalChecking() {
  TString workDir = gSystem -> Getenv("VMCWORKDIR");
  TString parameterDir = workDir + "/parameters/";

  Bool_t fUseSeparatedData = kFALSE;
  if (fData.EndsWith(".txt"))
    fUseSeparatedData = kTRUE;

  STGenerator *fGenerator = new STGenerator("pedestal");
  fGenerator -> SetUseSeparatedData(fUseSeparatedData);
  fGenerator -> SetParameterFile(parameterDir + fParameterFile);
  fGenerator -> SetFPNPedestal(fFPNThreshold);

  if (!fUseSeparatedData)
    fGenerator -> AddData(fData);
  else {
    std::ifstream listFile(fData.Data());
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
