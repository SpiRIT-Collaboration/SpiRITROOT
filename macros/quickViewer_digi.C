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

//////////////////////////////////////////////////////////
//                                                      //
//   Don't edit the below if you don't know about it.   //
//                                                      //
//////////////////////////////////////////////////////////

#define cRED "\033[1;31m"
#define cYELLOW "\033[1;33m"
#define cNORMAL "\033[0m"

TChain *fChain = NULL;
TClonesArray *fEventArray = nullptr;
TString fileName;
STPlot *fPlot = NULL;
Int_t fEventID = 0;

void next(Int_t eventID = -1) {
  if (eventID == -1)
    fEventID++;
  else
    fEventID = eventID;

  fChain -> GetEntry(fEventID);
  auto event = (STRawEvent *) fEventArray -> At(0);

  fPlot -> SetEvent(event);
  fPlot -> SetPadplaneTitle(Form("Name:%s - Event ID: %d - Top view", fileName.Data(), fEventID));
  fPlot -> SetSideviewTitle(Form("Name:%s - Event ID: %d - Beam right view", fileName.Data(), fEventID));
  fPlot -> DrawPadplane();
  fPlot -> DrawSideview();
}

void quickViewer_digi() {
  if (!(gSystem -> Getenv("NAME"))) {
    cout << endl;
    cout << cYELLOW << "== Usage: " << cNORMAL << "NAME=" << cRED << "####" << cNORMAL << " root quickViewer.C" << endl;
    cout << "          There're some settings in the file. Open and check for yourself." << endl;
    cout << endl;
    gSystem -> Exit(0);
  }

  fChain = new TChain("cbmsim");

  fileName = gSystem -> Getenv("NAME");

  TString workDir = gSystem -> Getenv("VMCWORKDIR");
  TString parameterDir = workDir + "/parameters/";
  TString dataDir = workDir + "/macros/data/";

  TString version; {
    TString name = workDir + "/VERSION";
    std::ifstream vfile(name);
    vfile >> version;
    vfile.close();
  }

//  TString dataFile = dataDir + fileName + ".digi." + version + ".root";
  TString dataFile = dataDir + fileName + ".digi.root";

  fChain -> AddFile(dataFile);

  fChain -> SetBranchAddress("STRawEvent", &fEventArray);

  STParReader *fPar = new STParReader(parameterDir + fParameterFile);

  STMap *map = new STMap();
  map -> SetUAMap(fPar -> GetFilePar(fPar -> GetIntPar("UAMapFile")));
  map -> SetAGETMap(fPar -> GetFilePar(fPar -> GetIntPar("AGETMapFile")));

  fPlot = new STPlot();
  fPlot -> SetNumTbs(fPar -> GetIntPar("NumTbs"));
  fPlot -> SetPadplaneTitle(Form("Name:%s - Event ID: %d - Top view", fileName.Data(), fEventID));
  fPlot -> SetSideviewTitle(Form("Name:%s - Event ID: %d - Beam right view", fileName.Data(), fEventID));

  fChain -> GetEntry(fEventID);
  auto event = (STRawEvent *) fEventArray -> At(0);

  fPlot -> SetEvent(event);
  fPlot -> DrawPadplane();
  fPlot -> DrawSideview();

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
