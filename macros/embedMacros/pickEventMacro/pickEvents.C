/**
  * This macro will pick out selected events
  * Please make sure there's no file having the same name as that of the output file.
  * In that case, those files are not overwritten.
 **/

////////////////////////////
//                        //
//   Configuration part   //
//                        //
////////////////////////////

// Put the parameter file name. Path is automatically concatenated.
TString fParameterFile = "ST.parameters.PhysicsRuns_201707.par";

// Set the supplement path which contains data list and meta data
TString supplementPath = "/mnt/spirit/rawdata/misc/rawdataSupplement";

TString workDir = gSystem -> Getenv("VMCWORKDIR");
TString parameterDir = workDir + "/parameters/";
//TString eventListFile = parameterDir + "VertexLocation.txt";
TString eventListFile = "VertexLocation.txt";

//////////////////////////////////////////////////////////
//                                                      //
//   Don't edit the below if you don't know about it.   //
//                                                      //
//////////////////////////////////////////////////////////

#define cRED "\033[1;31m"
#define cYELLOW "\033[1;33m"
#define cNORMAL "\033[0m"

void run(Int_t &runNo, vector<Int_t> *eventList) {
  TString fOutPath = Form("picked/run_%d", runNo);
  gSystem -> Exec(Form("mkdir -p %s", (fOutPath + "/pickedData").Data()));

  STParReader *fPar = new STParReader(parameterDir + fParameterFile);

  TString fDataFile = Form("%s/run_%04d/dataList.txt", supplementPath.Data(), runNo);
  TString fMetaFile = Form("%s/run_%04d/metadataList.txt", supplementPath.Data(), runNo);
  TString fOutputFile = fOutPath + "/dataList.txt";

  Bool_t fUseSeparatedData = kTRUE;

  STCore *fCore = new STCore();
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

  std::ifstream metalistFile(fMetaFile.Data());
  TString dataFileWithPath;
  for (auto iCobo = 0; iCobo < 12; iCobo++) {
    dataFileWithPath.ReadLine(metalistFile);
    dataFileWithPath = Form("%s/run_%04d/%s", supplementPath.Data(), runNo, dataFileWithPath.Data());
    fCore -> LoadMetaData(dataFileWithPath, iCobo);
  }

  listFile.open(fOutputFile.Data());
  iCobo = 0;
  while (buffer.ReadLine(listFile))
    fCore -> SetWriteFile(buffer.Data(), iCobo++);
    //    fCore -> SetWriteFile(Form("%s/%s", fOutPath.Data(), buffer.Data()), iCobo++);
  listFile.close();

  fCore -> SetUAMap(fPar -> GetFilePar(fPar -> GetIntPar("UAMapFile")));
  fCore -> SetAGETMap(fPar -> GetFilePar(fPar -> GetIntPar("AGETMapFile")));

  STRawEvent *rawEvent = NULL;
  for (auto iEvent = 0; iEvent < eventList -> size(); iEvent++) {
    rawEvent = fCore -> GetRawEvent(eventList -> at(iEvent) - 1);
    fCore -> WriteData();

    cout << "Event ID: " << rawEvent -> GetEventID() << "(entry: " << eventList -> at(iEvent) << ") is processed!" << endl;
  }

  delete rawEvent;
  delete fPar;
  delete fCore;
}

void pickEvents(int run_num) {


  ifstream vertexf(eventListFile.Data());
  if(!vertexf.good())
    {
      cout<<"Vertex file not found!"<<endl;
      return;
    }  
 
  ifstream eventList(eventListFile);
  Int_t numEvents = 0;
  Int_t oldRunid = 0;
  Int_t runid, eventid;
  Double_t vx,vy,vz;
  string header;
  
  vector<Int_t> runs;
  vector<vector<Int_t> *> events;
  vector<Int_t> *temp = new vector<Int_t>;
  std::getline(eventList,header);
  while (1) {
    eventList >> runid >> eventid >> vx >> vy >> vz;
    cout<<"Read in "<<runid<<" "<< eventid <<" "<< vx <<" "<< vy <<" "<< vz<<endl;
    if (eventList.eof()) {
      events.push_back(temp);

      break;
    }
    
    if (oldRunid == 0) {
      runs.push_back(runid);

      oldRunid = runid;
    }

    if (oldRunid != runid) {
      runs.push_back(runid);
      events.push_back(temp);

      oldRunid = runid;
      temp = new vector<Int_t>;
//      numEvents = 0;
    }

    temp -> push_back(eventid);
//    numEvents++;
  }

/*
  cout << "Num runs: " << runs.size() << endl;
  Int_t total = 0;
  for (auto i = 0; i < runs.size(); i++) {
    cout << runs.at(i) << " " << events.at(i) -> size() << endl;
    total += events.at(i) -> size();
  }
  cout << "Total: " << total << endl;
  */

  for (auto i = 0; i < runs.size(); i++)
    {
      if(run_num == runs.at(i))
	run(runs.at(i), events.at(i));
    }

}
