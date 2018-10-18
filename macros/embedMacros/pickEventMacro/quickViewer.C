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
TString fParameterFile = "ST.parameters.Commissioning_201604.par";

// Set use the meta data files
Bool_t fUseMetadata = kTRUE;

// Set the supplement path which contains data list and meta data
// Only works when fUseMetadata is kTRUE
TString supplementPath = "./picked/";

// Set use the gain calibration data file.
Bool_t fUseGainCalibration = kFALSE;

// FPN pedestal range selection threshold
Int_t fFPNThreshold = 10;

// Set the gating grid noise data. If left blank, it will use FPN pedestal.
TString fGGNoiseData = "/mnt/spirit/rawdata/misc/ggNoise/ggNoise_2853.root";

//////////////////////////////////////////////////////////
//                                                      //
//   Don't edit the below if you don't know about it.   //
//                                                      //
//////////////////////////////////////////////////////////

#define cRED "\033[1;31m"
#define cYELLOW "\033[1;33m"
#define cNORMAL "\033[0m"

STCore *fCore = NULL;
STPlot *fPlot = NULL;

void next(Int_t eventID = -1) {
  fPlot -> DrawPadplane(eventID);
  fPlot -> DrawSideview(fCore -> GetEventID());
}

void quickViewer() {
  if (!(gSystem -> Getenv("RUN"))) {
    cout << endl;
    cout << cYELLOW << "== Usage: " << cNORMAL << "RUN=" << cRED << "####" << cNORMAL << " root quickViewer.C" << endl;
    cout << "          There're some settings in the file. Open and check for yourself." << endl;
    cout << endl;
    gSystem -> Exit(0);
  }

  Int_t runNo = atoi(gSystem -> Getenv("RUN"));
  TString dataFile = "";
  TString metaFile = "";
  if (fUseMetadata) {
    dataFile = Form("%s/run_%04d/dataList.txt", supplementPath.Data(), runNo);
    metaFile = Form("%s/run_%04d/metadataList.txt", supplementPath.Data(), runNo);
  } else {
    gSystem -> Exec(Form("./createList.sh %d", runNo));

    dataFile = Form("list_run%04d.txt", runNo);
  }

  TString workDir = gSystem -> Getenv("VMCWORKDIR");
  TString parameterDir = workDir + "/parameters/";

  STParReader *fPar = new STParReader(parameterDir + fParameterFile);

  Bool_t fUseSeparatedData = kFALSE;
  if (dataFile.EndsWith(".txt"))
    fUseSeparatedData = kTRUE;

  if (!fUseSeparatedData) {
    fCore = new STCore(dataFile);
  } else {
    fCore = new STCore();
    fCore -> SetUseSeparatedData(fUseSeparatedData);

    TString dataFileWithPath = dataFile;
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

  fCore -> SetNumTbs(fPar -> GetIntPar("NumTbs"));

  if (!fGGNoiseData.IsNull()) {
    fCore -> SetGGNoiseData(fGGNoiseData);
    fCore -> InitGGNoiseSubtractor();
  }

  fCore -> SetGainMatchingData(Form("%sRelativeGain.list", parameterDir.Data()));

  if (fUseMetadata) {
    std::ifstream metalistFile(metaFile.Data());
    TString dataFileWithPath;
    for (auto iCobo = 0; iCobo < 12; iCobo++) {
      dataFileWithPath.ReadLine(metalistFile);
      dataFileWithPath = Form("%s/run_%04d/%s", supplementPath.Data(), runNo, dataFileWithPath.Data());
      fCore -> LoadMetaData(dataFileWithPath, iCobo);
    }
  }

  fPlot = fCore -> GetSTPlot();
  if (fUseGainCalibration) {
    fPlot -> SetPadplaneTitle(Form("Run#%04d - Event ID: %%d (Gain calibrated) - Top view", runNo));
    fPlot -> SetSideviewTitle(Form("Run#%04d - Event ID: %%d (Gain calibrated) - Beam right view", runNo));
  } else {
    fPlot -> SetPadplaneTitle(Form("Run#%04d - Event ID: %%d (Gain not calibrated) - Top view", runNo));
    fPlot -> SetSideviewTitle(Form("Run#%04d - Event ID: %%d (Gain not calibrated) - Beam right view", runNo));
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
