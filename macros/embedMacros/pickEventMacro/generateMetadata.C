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

  TString fDataFile = Form("./picked/run_%04d/dataList.txt", runNo);
  gSystem -> Exec(Form("mkdir -p ./picked/run_%04d/metadata", runNo));

  STCore *fCore = new STCore();
  fCore -> SetUseSeparatedData(kTRUE);

  TString dataFileWithPath = fDataFile;
  std::ifstream listFile(dataFileWithPath.Data());
  TString buffer;
  Int_t iCobo = -1;
  while (dataFileWithPath.ReadLine(listFile)) {
    if (dataFileWithPath.Contains("s."))
      fCore -> AddData(dataFileWithPath, iCobo);
//      fCore -> AddData("/mnt/spirit/analysis/estee/SpiRITROOT.develop/macros/pickEventMacro/" + dataFileWithPath, iCobo);
    else {
      iCobo++;
      fCore -> AddData(dataFileWithPath, iCobo);
//      fCore -> AddData("/mnt/spirit/analysis/estee/SpiRITROOT.develop/macros/pickEventMacro/" + dataFileWithPath, iCobo);
    }
  }
  
  fCore -> SetData(0);
  fCore -> GenerateMetaData(runNo);
}
