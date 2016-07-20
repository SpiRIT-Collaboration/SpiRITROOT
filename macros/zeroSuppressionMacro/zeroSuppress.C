/**
  * This macro stores zero suppressed data.
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
Bool_t fUseMetadata = kFALSE;

// Set the supplement path which contains data list and meta data
// Only works when fUseMetadata is kTRUE
TString supplementPath = "/data/Q16264/rawdataSupplement";

// Set use the gain calibration data file.
Bool_t fUseGainCalibration = kFALSE;

// FPN pedestal range selection threshold
Int_t fFPNThreshold = 10;

// Set the gating grid noise data. If left blank, it will use FPN pedestal.
TString fGGNoiseData = "";

//////////////////////////////////////////////////////////
//                                                      //
//   Don't edit the below if you don't know about it.   //
//                                                      //
//////////////////////////////////////////////////////////

#define cRED "\033[1;31m"
#define cYELLOW "\033[1;33m"
#define cNORMAL "\033[0m"

void zeroSuppress() {
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
//    gSystem -> Exec(Form("./createList.sh %d", runNo));

    dataFile = Form("list_run%04d_skimmed.txt", runNo);
  }

  TString workDir = gSystem -> Getenv("VMCWORKDIR");
  TString parameterDir = workDir + "/parameters/";

  STParReader *fPar = new STParReader(parameterDir + fParameterFile);

  Bool_t fUseSeparatedData = kFALSE;
  if (dataFile.EndsWith(".txt"))
    fUseSeparatedData = kTRUE;

  STCore *fCore = NULL;
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

  if (fUseMetadata) {
    std::ifstream metalistFile(metaFile.Data());
    TString dataFileWithPath;
    for (iCobo = 0; iCobo < 12; iCobo++) {
      dataFileWithPath.ReadLine(metalistFile);
      dataFileWithPath = Form("%s/run_%04d/%s", supplementPath.Data(), runNo, dataFileWithPath.Data());
      fCore -> LoadMetaData(dataFileWithPath, iCobo);
    }
  }

  TFile *fOutFile = new TFile(Form("run%04d.zeroSuppressed.root", runNo), "recreate");
  TTree *fOutTree = new TTree("tree", "Zero suppressed event tree");

  Int_t fEventID = -1;
  std::vector<STSlimPad> pads;
  std::vector<STSlimPad> *padsPtr = &pads;
  STSlimPad slimPad;
  fOutTree -> Branch("eventid", &fEventID);
  fOutTree -> Branch("pads", "std::vector<STSlimPad>", &padsPtr);

  for (Int_t iEvent = 0; iEvent < 10; iEvent++) {
    STRawEvent *rawEvent = fCore -> GetRawEvent(iEvent);

    fEventID = fCore -> GetEventID();

    Bool_t flag[270];
    memset(flag, kFALSE, sizeof(Bool_t)*270);

    padsPtr -> clear();
    Int_t numPads = rawEvent -> GetNumPads();
    for (Int_t iPad = 0; iPad < numPads; iPad++) {
      memset(flag, kFALSE, sizeof(Bool_t)*270);

      slimPad.id = -1;
      slimPad.baseline = -1;
      slimPad.sigma = -1;
      slimPad.tb.clear();
      slimPad.adc.clear();

      STPad *pad = rawEvent -> GetPad(iPad);

      slimPad.id = pad -> GetLayer()*108 + pad -> GetRow();
      slimPad.baseline = 0;
      slimPad.sigma = 0;

      Double_t *adc = pad -> GetADC();
      for (Int_t iTb = 0; iTb < 270; iTb++)
        flag[iTb] = (adc[iTb] > 15);

      for (Int_t iTb = 1; iTb < 270; iTb++) {
        if (flag[iTb - 1] == 0 && flag[iTb] == 1) {
          Int_t startTb = (iTb - 5 < 0 ? 0 : iTb - 5);
          for (Int_t tb = startTb; tb < iTb; tb++)
            flag[tb] |= kTRUE;
        } else if (flag[iTb - 1] == 1 && flag[iTb] == 0) {
          Int_t endTb = (iTb + 6 > 270 ? 270 : iTb + 6);
          for (Int_t tb = iTb; tb < endTb; tb++)
            flag[tb] |= kTRUE;

          iTb = endTb + 1;
        }
      }

/*
      for (Int_t iTb = 0; iTb < 270; iTb++)
        cout << flag[iTb];;
      cout << endl;
      */
/*
      for (Int_t iTb = 0; iTb < 270; iTb++)
        cout << setw(15) << adc[iTb] << " " << flag[iTb] << endl;
      cout << endl;
      char ha;
      cin >> ha;
*/

      Int_t startTb = -1;
      Int_t numTbs = 0;
      for (Int_t iTb = 1; iTb < 270; iTb++) {
//        cout << "iTb: " << iTb << " iTb - 1: " << flag[iTb - 1] << " iTb: " << flag[iTb] << endl;
        if (flag[iTb - 1] == 0 && flag[iTb] == 1) {
          startTb = iTb;
          numTbs++;
        } else if (flag[iTb - 1] == 1 && flag[iTb] == 1) {
          if (iTb == 1)
            startTb = 0;
          numTbs++;
        }
        else if (flag[iTb - 1] == 1 && flag[iTb] == 0) {
          if (startTb == -1 && iTb != 0) {
            cout << "Error!" << endl;
            return;
          } else if (startTb == -1 && iTb == 0) {
            slimPad.tb.push_back(Short_t(0));
            slimPad.tb.push_back(Short_t(1));

            slimPad.adc.push_back(Float_t(adc[0]));

            startTb = -1;
            numTbs = 0;
          } else {
            slimPad.tb.push_back(Short_t(startTb));
            slimPad.tb.push_back(Short_t(numTbs));

            for (Int_t tb = startTb; tb < startTb + numTbs; tb++)
              slimPad.adc.push_back(Float_t(adc[tb]));

            startTb = -1;
            numTbs = 0;
          }
        }
      }

      if (slimPad.tb.size() != 0)
        padsPtr -> push_back(slimPad);
    }

    fOutTree -> Fill();
  }

  fOutFile -> Write();

}
