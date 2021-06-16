class TClonesArrayLinker
{
public:
    TClonesArrayLinker(const std::string& element_type, TTree* outputTree, const std::string& branch_name, TTree *checkInput) : fBranchName(branch_name)
    { 
      fContent = new TClonesArray(element_type.c_str());
      if(checkInput)
      {
        if(checkInput -> GetBranch(fBranchName.c_str())) outputTree -> Branch(fBranchName.c_str(), fContent);
        else std::cout << fBranchName << " will not be written because it is not found." << std::endl;
      } else std::cout << "No branches are added because there are no input tree for reference." << std::endl;
    }
    void SetInputTree(TTree* inputTree)
    { inputTree -> SetBranchAddress(fBranchName.c_str(), &fContent); }

    TClonesArray *fContent = nullptr;
private:
    std::string fBranchName;
};

class STVectorFLinker
{
public:
   STVectorFLinker(TTree* outputTree, const std::string& branch_name, TTree *checkInput) : fBranchName(branch_name)
    { 
      fContent = new STVectorF;
      if(checkInput)
      {
        if(checkInput -> GetBranch(fBranchName.c_str())) outputTree -> Branch(fBranchName.c_str(), fContent);
        else std::cout << fBranchName << " will not be written because it is not found." << std::endl;
      } else std::cout << "No branches are added because there are no input tree for reference." << std::endl;

    }
    void SetInputTree(TTree* inputTree)
    { inputTree -> SetBranchAddress(fBranchName.c_str(), &fContent); }
private:
    std::string fBranchName;
    STVectorF *fContent = nullptr;
};

void hadd()
{
  if(!gSystem -> Getenv("FRAC") || !gSystem -> Getenv("OUTPUT") || !gSystem -> Getenv("SOURCE")) {
    std::cout << "Usage: FRAC=<NUM> OUTPUT=<OUTPUT> SOURCE=<FILE1>,<FILE2>... root -q hadd.C" << std::endl;
    return;
  }
  double fraction = std::atof(gSystem -> Getenv("FRAC"));
  if(fraction < 0 || fraction > 1) {
    std::cout << "Fraction can only be a number between 0 and 1." << std::endl;
    return;
  }
  std::cout << "Will only merge " << int(fraction*100) << "% of the available statistics." << std::endl;
  std::string outFilename = std::string(gSystem -> Getenv("OUTPUT"));
  std::cout << "Load data from:" << std::endl;
  std::vector<std::string> filenames;
  std::string files(gSystem -> Getenv("SOURCE")), temp;
  for(int i = 0; i <= files.size(); ++i) {
    if(i == files.size() || files[i] == ',') {
        filenames.push_back(temp);
	temp.clear();
        std::cout << "    " << filenames.back() << std::endl;
    }
    else temp.push_back(files[i]);
  }
  std::cout << std::endl;

  TFile output(outFilename.c_str(), "RECREATE");
  TTree outputTree("cbmsim", "cbmsim");

  int entries = 0;
  FairEventHeader *eventHeader = new FairEventHeader;
  outputTree.Branch("EventHeader.", eventHeader);
  std::vector<TClonesArrayLinker> linkers;
  std::vector<STVectorFLinker> linkersVF;
  // These are TClonesArray of STVector
  std::unordered_map<std::string, std::string> arrList{{"STData",      "STData"},       
                                                       {"EventID",     "STVectorI"},    
                                                       {"EventType",   "STVectorI"},    
                                                       {"RunID",       "STVectorI"},    
                                                       {"Prob",        "STVectorF"},    
                                                       {"SD",          "STVectorF"},    
                                                       {"FlattenPion", "STVectorF"},    
                                                       {"CMVector",    "STVectorVec3"}, 
                                                       {"FragRapidity","STVectorF"},    
                                                       {"FragVelocity","STVectorVec3"}, 
                                                       {"LabRapidity", "STVectorF"},    
                                                       {"Eff",         "STVectorF"},    
                                                       {"EffErr",      "STVectorF"},    
                                                       {"V1RPAngle",   "STVectorF"},    
                                                       {"V2RPAngle",   "STVectorF"}};   

  // These are branches that store STVectorF directly, no TClonesArray
  std::vector<std::string> arrListSTF{"BeamMom", "BeamRapidity", "RP", "RPV2", 
                                      "QMag", "bML", "bMult", "bERat", "ET", 
				      "ERAT", "AllObs", "MCRotZ", "bTruth"};

  {
    TFile file(filenames[0].c_str());
    auto inputTree = (TTree*) file.Get("cbmsim");
    for(const auto& ele : arrList) 
      linkers.emplace_back(ele.second, &outputTree, ele.first, inputTree);
    for(const auto& ele : arrListSTF) linkersVF.emplace_back(&outputTree, ele, inputTree);

    // copy BranchList from the first source file to the destination
    // This is needed for SpiRITROOT to recognize how the ROOT file is formated
    output.cd();
    file.Get("BranchList") -> Write("BranchList", TObject::kSingleKey); 
    file.Get("TimeBasedBranchList") -> Write("TimeBasedBranchList", TObject::kSingleKey); 
    file.Get("FileHeader") -> Write("FileHeader", TObject::kSingleKey);
    file.Get("cbmout") -> Write("cbmout", TObject::kSingleKey);
  }

  for(const auto& filename : filenames)
  {
    TFile file(filename.c_str());
    auto inputTree = (TTree*) file.Get("cbmsim");
    for(auto& linker : linkers) linker.SetInputTree(inputTree);
    for(auto& linker : linkersVF) linker.SetInputTree(inputTree);
    inputTree -> SetBranchAddress("EventHeader.", &eventHeader);
    entries += ((TParameter<int>*) file.Get("Entries")) -> GetVal();

    // randomly select data if fraction < 1
    std::vector<int> ids;
    for(int i = 0; i < inputTree -> GetEntries(); ++i) ids.push_back(i);
    if(fraction < 1)
    {
      std::shuffle(ids.begin(), ids.end(), std::default_random_engine());
      ids.resize(ids.size()*fraction);
    }
    std::sort(ids.begin(), ids.end());

    // Fill all branches to tree
    std::cout << std::endl;
    int order = 0;
    for(int i : ids)
    {
      inputTree -> GetEntry(i);
      outputTree.Fill();
      if((order + 1) % 100 == 0 || i == ids.back()) std::cout << "Processing entry " << order + 1 << " out of a total of " << ids.size() << " entries\r" << std::flush;
      ++order;
    }
    std::cout << std::endl;
  }

  output.cd();
  (new TParameter<int>("Entries", entries)) -> Write();
  outputTree.Write();
  std::cout << "Data is saved to: " << outFilename << std::endl;
}
