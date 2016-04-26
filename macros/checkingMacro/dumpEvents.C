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
TString fParameterFile = "ST.parameters.Commissioning_201604.par";

// Set the numbers.
Int_t fStart = 0; // Start event number. Both 0 means from 0 to the end.
Int_t fEnd = 100;   // End event number. 

// Set use the gain calibration data file.
Bool_t fUseGainCalibration = kFALSE;

// FPN pedestal range selection threshold
Int_t fFPNThreshold = 5;

// Set the gating grid noise data. If left blank, it will use FPN pedestal.
TString fGGNoiseData = "";

// Set use the meta data files
Bool_t fUseMetadata = kFALSE;

// Set the supplement path which contains data list and meta data
// Only works when fUseMetadata is kTRUE
TString supplementPath = "/data/Q16264/rawdataSupplement";

//////////////////////////////////////////////////////////
//                                                      //
//   Don't edit the below if you don't know about it.   //
//                                                      //
//////////////////////////////////////////////////////////

#define cRED "\033[1;31m"
#define cYELLOW "\033[1;33m"
#define cNORMAL "\033[0m"

Int_t fRunNo = 0;

STCore *fCore = NULL;
STPlot *fPlot = NULL;

TCanvas *fCvs = NULL;

TCanvas *next(Int_t eventID = -1) {
  fCvs = fPlot -> DrawPadplane(eventID);
  fCvs -> SaveAs(Form("run_%04d/r%04de%05d_top.png", fRunNo, fRunNo, fCore -> GetEventID()));
  fCvs = fPlot -> DrawSideview(fCore -> GetEventID());
  fCvs -> SaveAs(Form("run_%04d/r%04de%05d_side.png", fRunNo, fRunNo, fCore -> GetEventID()));

  return fCvs;
}

void dumpEvents() {
  if (!(gSystem -> Getenv("RUN"))) {
    cout << endl;
    cout << cYELLOW << "== Usage: " << cNORMAL << "RUN=" << cRED << "####" << cNORMAL << " root dumpEvents.C -b -q -l" << endl;
    cout << "          There're some settings in the file. Open and check for yourself." << endl;
    cout << endl;
    gSystem -> Exit(0);
  }

  fRunNo = atoi(gSystem -> Getenv("RUN"));
  TString dataFile = "";
  TString metaFile = "";
  if (fUseMetadata) {
    dataFile = Form("%s/run_%04d/dataList.txt", supplementPath.Data(), fRunNo);
    metaFile = Form("%s/run_%04d/metadataList.txt", supplementPath.Data(), fRunNo);
  } else {
    if (GETFileChecker::CheckFile(Form("list_run%04d.txt", fRunNo)).IsNull())
      gSystem -> Exec(Form("./createList.sh %d", fRunNo));

    dataFile = Form("list_run%04d.txt", fRunNo);
  }

  gSystem -> Exec(Form("mkdir -p run_%04d", fRunNo));
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

  fPlot = fCore -> GetSTPlot();
  if (fUseGainCalibration) {
    fPlot -> SetPadplaneTitle(Form("Run#%04d - Event ID: %%d (Gain calibrated) - Top view", fRunNo));
    fPlot -> SetSideviewTitle(Form("Run#%04d - Event ID: %%d (Gain calibrated) - Beam right view", fRunNo));
  } else {
    fPlot -> SetPadplaneTitle(Form("Run#%04d - Event ID: %%d (Gain not calibrated) - Top view", fRunNo));
    fPlot -> SetSideviewTitle(Form("Run#%04d - Event ID: %%d (Gain not calibrated) - Beam right view", fRunNo));
  }

  if (fStart == 0 && fEnd == 0)
    while (next());
  else {
    next(fStart);
    for (Int_t iEvent = fStart + 1; iEvent < fEnd; iEvent++)
      next();
  }
}
