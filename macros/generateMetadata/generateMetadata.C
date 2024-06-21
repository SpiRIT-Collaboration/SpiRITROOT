/**
  * This macro generates meta data
 **/

#define cRED "\033[1;31m"
#define cYELLOW "\033[1;33m"
#define cNORMAL "\033[0m"

void generateMetadata() {
  if (!(gSystem -> Getenv("RUN"))) {
    cout << endl;
    cout << cYELLOW << "== Usage: " << cNORMAL << "RUN=" << cRED << "####" << cNORMAL << " root generateMetadata.C" << endl;
    cout << endl;
    gSystem -> Exit(0);
  }

  Int_t runNo = atoi(gSystem -> Getenv("RUN"));
  Bool_t fIsFRIBDAQ = true;

  if(fIsFRIBDAQ)
    gSystem -> Exec(Form("./createList_FRIBDAQ.sh %d", runNo));
  else
    gSystem -> Exec(Form("./createList.sh %d", runNo));

  TString fDataFile = Form("list_run%04d.txt", runNo);
  gSystem -> Exec(Form("mkdir -p run_%04d/metadata", runNo));

  Bool_t fUseSeparatedData = kFALSE;
  if (fDataFile.EndsWith(".txt"))
    fUseSeparatedData = kTRUE;

  STCore *fCore = nullptr;
  if (!fUseSeparatedData) {
    fCore = new STCore(fDataFile, fIsFRIBDAQ);
  } else {
    fCore = new STCore(fIsFRIBDAQ);
    fCore -> SetUseSeparatedData(fUseSeparatedData);

    TString dataFileWithPath = fDataFile;
    std::ifstream listFile(dataFileWithPath.Data());
    TString buffer;
    Int_t iCobo = -1;
    while (dataFileWithPath.ReadLine(listFile)) {
      if (dataFileWithPath.Contains("00.evt")) {
        iCobo++;
        cout << "#Cobo: " << iCobo << "; dataFilePath: " << dataFileWithPath << endl;
        fCore -> AddData(dataFileWithPath, iCobo);
      }
      else {
        cout << "Cobo: " << iCobo << "; dataFilePath: " << dataFileWithPath << endl;
        fCore -> AddData(dataFileWithPath, iCobo);
      }
    }
  }
  
  fCore -> SetData(0);
  fCore -> GenerateMetaData(runNo);

  gSystem -> Exec(Form("mv list_run%04d.txt run_%04d/dataList.txt", runNo, runNo));
}
