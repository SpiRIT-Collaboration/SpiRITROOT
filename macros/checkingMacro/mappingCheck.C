/**
  * This macro reads parameter file and set the mapping file selected in it
  * and displays AsAd mapping and channel mapping in an AGET chip.
 **/

////////////////////////////
//                        //
//   Configuration part   //
//                        //
////////////////////////////

// Put the parameter file name. Path is automatically concatenated.
TString fParameterFile = "ST.parameters.par";

//////////////////////////////////////////////////////////
//                                                      //
//   Don't edit the below if you don't know about it.   //
//                                                      //
//////////////////////////////////////////////////////////

void mappingCheck() {
  TString workDir = gSystem -> Getenv("VMCWORKDIR");
  TString parameterDir = workDir + "/parameters/";

  STParReader *fPar = new STParReader(parameterDir + fParameterFile);

  STMapTest *fMapTest = new STMapTest();
  fMapTest -> SetUAMap(fPar -> GetFilePar(fPar -> GetIntPar("UAMapFile")));
  fMapTest -> SetAGETMap(fPar -> GetFilePar(fPar -> GetIntPar("AGETMapFile")));

  fMapTest -> ShowUAMap();
  fMapTest -> ShowAGETMap();
}
