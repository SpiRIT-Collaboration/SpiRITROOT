/**
  * Macro for generating pedestal checking data
 **/

////////////////////////////
//                        //
//   Configuration part   //
//                        //
////////////////////////////

// Put the parameter file name. Path is automatically concatenated.
TString fParameterFile = "ST.parameters.par";

// Pedestal data file
TString fData = "";

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

  STGenerator *fGenerator = new STGenerator("pedestal");
  fGenerator -> SetParameterFile(parameterDir + fParameterFile);
  fGenerator -> SetFPNPedestal(fFPNThreshold);

  fGenerator -> AddData(fData);

  fGenerator -> SetOutputFile(fOutputFilename);

  fGenerator -> Print();
  fGenerator -> StartProcess();
}
