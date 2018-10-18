/**
  * This macro will count the number of events in the Vertex file
  * The correct total is needed for run_reco_experiment
  * Used by pixel_create_submit.C
 **/

TString eventListFile = "/mnt/spirit/analysis/estee/SpiRITROOT.develop/parameters/VertexLocation.txt";

void makeRunInfo() {


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
    }

    temp -> push_back(eventid);
  }

  ofstream out;
  out.open("run_info_embed.txt");
  
  for (auto i = 0; i < runs.size(); i++)
    out<<runs.at(i)<<" "<<events.at(i)->size()<<endl;


  out.close();
}
