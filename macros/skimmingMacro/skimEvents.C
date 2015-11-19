/**
  * This macro will skim off empty events
  * Please make sure there's no file having the same name as that of the output file.
  * In that case, those files are not overwritten.
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

// Set the output data file list for the skimmed data.
TString fOutputFile = "";

// Set use the gain calibration data file.
Bool_t fUseGainCalibration = kTRUE;

// FPN pedestal range selection threshold
Int_t fFPNThreshold = 5;

// ADC threshold
Int_t fADCThreshold = 400;

//////////////////////////////////////////////////////////
//                                                      //
//   Don't edit the below if you don't know about it.   //
//                                                      //
//////////////////////////////////////////////////////////

STCore *fCore = NULL;

void skimEvents() {
  gSystem -> Exec("mkdir -p skimmed");

  TString workDir = gSystem -> Getenv("VMCWORKDIR");
  TString parameterDir = workDir + "/parameters/";

  STParReader *fPar = new STParReader(parameterDir + fParameterFile);

  Bool_t fUseSeparatedData = kFALSE;
  if (fDataFile.EndsWith(".txt")) {
    if (!fOutputFile.EndsWith(".txt")) {
      cout << "== skimEvents.C: Output file should be end with 'txt' extension!" << endl;

      return;
    }

    fUseSeparatedData = kTRUE;
  }

  if (!fUseSeparatedData) {
    fCore = new STCore(fDataFile);
    fCore -> SetWriteFile(fOutputFile);
  } else {
    fCore = new STCore();
    fCore -> SetUseSeparatedData(fUseSeparatedData);

    std::ifstream listFile(fDataFile.Data());
    TString buffer;
    Int_t iCobo = -1;
    while (buffer.ReadLine(listFile)) {
      if (buffer.Contains("s."))
        fCore -> AddData(buffer, iCobo);
      else {
        iCobo++;
        fCore -> AddData(buffer, iCobo);
      }
    }
    listFile.close();
    fCore -> SetData(0);

    listFile.open(fOutputFile.Data());
    iCobo = 0;
    while (buffer.ReadLine(listFile))
      fCore -> SetWriteFile(buffer, iCobo++);
    listFile.close();
  }

  if (fUseGainCalibration) {
    fCore -> SetGainCalibrationData(fPar -> GetFilePar(fPar -> GetIntPar("GainCalibrationDataFile")));
    fCore -> SetGainReference(fPar -> GetDoublePar("GCConstant"), fPar -> GetDoublePar("GCLinear"), fPar -> GetDoublePar("GCQuadratic"));
  }

  fCore -> SetUAMap(fPar -> GetFilePar(fPar -> GetIntPar("UAMapFile")));
  fCore -> SetAGETMap(fPar -> GetFilePar(fPar -> GetIntPar("AGETMapFile")));
  fCore -> SetFPNPedestal(fFPNThreshold);

  STMap *fMap = fCore -> GetSTMap();

  STRawEvent *rawEvent = NULL;
  while ((rawEvent = fCore -> GetRawEvent())) {
    Int_t UAIdx = 0;
    Int_t dummy = 0;
    Bool_t overThreshold = kFALSE;

    Int_t numPads = rawEvent -> GetNumPads();
    for (Int_t iPad = 0; iPad < numPads; iPad++) {
      STPad *pad = rawEvent -> GetPad(iPad);
      fMap -> GetMapData(pad -> GetRow(), pad -> GetLayer(), UAIdx, dummy, dummy, dummy, dummy);

      if (UAIdx > 3 && UAIdx < 8) {
        Double_t *adc = pad -> GetADC();
        for (Int_t iTb = 0; iTb < fCore -> GetNumTbs(); iTb++)
          if (adc[iTb] > fADCThreshold)
            overThreshold = kTRUE;

        if (overThreshold) {
          fCore -> WriteData();
          break;
        }
      }
    }

    if (rawEvent -> GetEventID()%100 == 0)
      cout << "Event ID: " << rawEvent -> GetEventID() << " events processed!" << endl;
  }
}
