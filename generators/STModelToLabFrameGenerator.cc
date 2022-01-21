#include "STModelToLabFrameGenerator.hh"
#include "FairLogger.h"
#include "FairRunSim.h"
#include "FairRootManager.h"
#include "FairMCEventHeader.h"
#include "FairIon.h"
#include "TSystem.h"
#include "TRandom.h"

#include "STFairMCEventHeader.hh"
#include "TDatabasePDG.h"
#include "TParticlePDG.h"

TParticlePDG* Elements::PDGToParticleData(int pdg)
{
  if(auto particle = TDatabasePDG::Instance()->GetParticle(pdg))
    return particle;
  else
  {
    // heavy ioins. Otherwise TDatabasePDG should already have the information
    int Z = (pdg%10000000)/10000 + 0.5;
    int A = (pdg%10000)/10 + 0.5;
    // add particle to TDatabasePDG so the memory management behavior is identical for light particles or ions.
    return TDatabasePDG::Instance()->AddParticle(Form("%d",A) + symbol[Z-1], Form("%d",A)+symbol[Z-1], A*Elements::kProtonMass, true, 0, 3*Z, "ion", pdg, 0, 0); 
  }
}

STTransportReader* ReaderFactory(TString prefix, TString filename)
{
  if (filename.BeginsWith("imqmdNew")) { return new STImQMDNewReader(prefix + filename); }
  else if (filename.BeginsWith("imqmdRaw")) { return new STImQMDRawReader(prefix + filename); }
  else if(filename.BeginsWith("imqmd") || filename.BeginsWith("approx")) { return new STImQMDReader(prefix + filename); }
  else if(filename.BeginsWith("urqmd"))  { return new STUrQMDReader(prefix + filename); }
  else if(filename.BeginsWith("pbuu")) { return new STpBUUReader(prefix + filename); }
  else if(filename.BeginsWith("amdNew")) { return new STAMDReader(prefix + filename); }
  else if(filename.BeginsWith("ibuu") || filename.BeginsWith("amd")) { return new STIBUUReader(prefix + filename, filename.BeginsWith("ibuu")); }
  else if(filename.BeginsWith("dcqmdNew")) { return new STDcQMDNewReader(prefix + filename); }
  else if(filename.BeginsWith("dcqmdPion")) { return new STDcQMDPionReader(prefix + filename); }
  else if(filename.BeginsWith("dcqmd")) { return new STDcQMDReader(prefix + filename); }
  else if(filename.BeginsWith("hw")) { return new STHWReader(prefix + filename); }
  else if(filename.BeginsWith("iqmd")) { return new STIQMDReader(prefix + filename); }
  else return nullptr;
}

/***********************************************
 * Transport Reader base class
 * *********************************************/
STTransportReader::STTransportReader()
{}

STTransportReader::~STTransportReader()    
{}

std::vector<FairIon*> STTransportReader::GetParticleList()
{
  int initEventID = this -> GetEntry();

  auto n = this -> GetEntries();
  std::set<int> pdgList;
  std::vector<STTransportParticle> particles;
  while(this -> GetNext(particles))
  {
    for(const auto& part : particles) 
      if(part.pdg > 3000) // if pdg <= 3000 it is consider a regular elements, not heavy ions
        pdgList.insert(part.pdg);
  }

  std::vector<FairIon*> particleList;
  for(auto pdg : pdgList)
  {
    auto particle = Elements::PDGToParticleData(pdg);
    if(!particle) continue;
    int a = int(particle -> Mass()/Elements::kProtonMass + 0.5);
    int z = int(particle -> Charge()/3. + 0.5);
 
    if(a > 1) particleList.push_back(new FairIon(Form("%d",a)+Elements::symbol[z-1],z,a,z));
  }

  // rewind the reader to its initial point
  this -> SetEntry(initEventID);
  return particleList; 
}

/**********************************************
 * Deried reader class for ImQMD particles
 * ********************************************/
STImQMDReader::STImQMDReader(TString fileName) : fFile(fileName), fLocalID(0)
{
  if(!fFile.IsOpen())
    LOG(INFO) << "Reading ImQMD data from " << fileName << FairLogger::endl;

  fTree = (TTree*) fFile.Get("cluster");
  if(!fTree)
    LOG(FATAL) << "Tree cluster is not found in file " << fileName << FairLogger::endl;
  
  fTree -> SetBranchAddress("eventid", &fTreeEventID);
  fTree -> SetBranchAddress("A", &fPartA);
  fTree -> SetBranchAddress("Z", &fPartZ);
  fTree -> SetBranchAddress("px", &fPx);
  fTree -> SetBranchAddress("py", &fPy);
  fTree -> SetBranchAddress("pz", &fPz);
  fTree -> SetBranchAddress("x", &fX);
  fTree -> SetBranchAddress("y", &fY);
  fTree -> SetBranchAddress("z", &fZ);
  int n = fTree -> GetEntries();
  fTree -> GetEntry(n - 1);
  fEntries = fTreeEventID;

  fTree -> GetEntry(fLocalID);
}

TString STImQMDReader::Print()
{ return TString::Format("ImQMD reader with source %s", fFile.GetName()); }

STImQMDReader::~STImQMDReader() {}

bool STImQMDReader::GetNext(std::vector<STTransportParticle>& particleList)
{
  particleList.clear();
  if(fEventID >= fEntries) return false;
  while(fTreeEventID == fEventID)
  {
    int pdg = 0;
    int Z = fPartZ, A = fPartA;
    if(Z == 1 && A == 1) pdg = 2212;
    else if(Z == 0 && A == 1) pdg = 2112;
    // special case for ImQMD files
    // When Z = +/- 1 and A = 0, it refers to pions
    else if(Z == 1 && A == 0) pdg = 211;
    else if(Z == -1 && A == 0) pdg = -211;
    else pdg = 1000000000 + Z*10000 + A*10;

    particleList.push_back({pdg, fPx, fPy, fPz, fX, fY, fZ});
    ++fLocalID;
    fTree -> GetEntry(fLocalID);
  }
  ++fEventID;
  return true;;
}

void STImQMDReader::SetEntry(int t_entry)
{
  fLocalID = 0;
  fTree -> GetEntry(fLocalID);
  while(fTreeEventID < t_entry)
  {
    ++fLocalID;
    if(fLocalID > fTree->GetEntries()) 
      LOG(FATAL) << "ImQMD Reader cannot find start entry for event " << t_entry << FairLogger::endl;
    fTree -> GetEntry(fLocalID);
  }
  fEventID = t_entry;
}

/**********************************************
 * Deried reader class for ImQMD particles
 * ********************************************/
STImQMDNewReader::STImQMDNewReader(TString fileName) : fFile(fileName)
{
  if(!fFile.IsOpen())
    LOG(INFO) << "Reading ImQMD data (new format) from " << fileName << FairLogger::endl;

  fTree = (TTree*) fFile.Get("cluster");
  if(!fTree)
    LOG(FATAL) << "Tree cluster is not found in file " << fileName << FairLogger::endl;
  
  fTree -> SetBranchAddress("multi", &fMulti);
  fTree -> SetBranchAddress("A", &fPartA);
  fTree -> SetBranchAddress("Z", &fPartZ);
  fTree -> SetBranchAddress("px", &fPx);
  fTree -> SetBranchAddress("py", &fPy);
  fTree -> SetBranchAddress("pz", &fPz);
  fTree -> SetBranchAddress("x", &fX);
  fTree -> SetBranchAddress("y", &fY);
  fTree -> SetBranchAddress("z", &fZ);
}

TString STImQMDNewReader::Print()
{ return TString::Format("ImQMD reader (new format) with source %s", fFile.GetName()); }

STImQMDNewReader::~STImQMDNewReader() {}

bool STImQMDNewReader::GetNext(std::vector<STTransportParticle>& particleList)
{
  particleList.clear();
  if(fEventID >= fTree -> GetEntries()) return false;
  fTree -> GetEntry(fEventID);
  for(int i = 0; i < fMulti; ++i)
  {
    int pdg = 0;
    int Z = fPartZ[i], A = fPartA[i];
    if(Z == 1 && A == 1) pdg = 2212;
    else if(Z == 0 && A == 1) pdg = 2112;
    // special case for ImQMD files
    // When Z = +/- 1 and A = 0, it refers to pions
    else if(Z == 1 && A == 0) pdg = 211;
    else if(Z == -1 && A == 0) pdg = -211;
    else pdg = 1000000000 + Z*10000 + A*10;

    particleList.push_back({pdg, fPx[i], fPy[i], fPz[i], fX[i], fY[i], fZ[i]});
  }
  ++fEventID;
  return true;;
}

/**********************************************
 * Deried reader class for ImQMD raw (no coalesce) particles
 * ********************************************/
STImQMDRawReader::STImQMDRawReader(TString fileName) : fFile(fileName)
{
  if(!fFile.IsOpen())
    LOG(INFO) << "Reading ImQMD data (new format) from " << fileName << FairLogger::endl;

  fTree = (TTree*) fFile.Get("raw");
  if(!fTree) fTree = (TTree*) fFile.Get("nucleon");
  if(!fTree)
    LOG(FATAL) << "Tree raw is not found in file " << fileName << FairLogger::endl;
  
  fTree -> SetBranchAddress("multi", &fMulti);
  fTree -> SetBranchAddress("iso", &fIso);
  fTree -> SetBranchAddress("px", &fPx);
  fTree -> SetBranchAddress("py", &fPy);
  fTree -> SetBranchAddress("pz", &fPz);
  fTree -> SetBranchAddress("x", &fX);
  fTree -> SetBranchAddress("y", &fY);
  fTree -> SetBranchAddress("z", &fZ);
}

TString STImQMDRawReader::Print()
{ return TString::Format("ImQMD reader (new format) with source %s", fFile.GetName()); }

STImQMDRawReader::~STImQMDRawReader() {}

bool STImQMDRawReader::GetNext(std::vector<STTransportParticle>& particleList)
{
  particleList.clear();
  if(fEventID >= fTree -> GetEntries()) return false;
  fTree -> GetEntry(fEventID);
  for(int i = 0; i < fMulti; ++i)
  {
    int pdg = 0;
    if(fIso[i] == 1) pdg = 2212;
    else pdg = 2112;

    particleList.push_back({pdg, fPx[i], fPy[i], fPz[i], fX[i], fY[i], fZ[i]});
  }
  ++fEventID;
  return true;;
}

/*************************************************
* STpBUUReader
**************************************************/
STpBUUReader::STpBUUReader(TString fileName) : fFile(fileName)
{
  if(fFile.IsOpen())
    LOG(INFO) << "Reading pBUU data from " << fileName << FairLogger::endl;
  fTree = (TTree*) fFile.Get("tree");
  if(!fTree)
    LOG(FATAL) << "Tree is not found in pBUU file " << fileName << "!" << FairLogger::endl;

  fTree -> SetBranchAddress("multi", &fMulti);
  fTree -> SetBranchAddress("pid", fPID);
  fTree -> SetBranchAddress("px", fPx);
  fTree -> SetBranchAddress("py", fPy);
  fTree -> SetBranchAddress("pzCMS", fPz);
  fTree -> SetBranchAddress("x", fX);
  fTree -> SetBranchAddress("y", fY);
  fTree -> SetBranchAddress("z", fZ);
  fTree -> SetBranchAddress("multi", &fMulti);

  fEntries = fTree -> GetEntries();
}

bool STpBUUReader::GetNext(std::vector<STTransportParticle>& particleList)
{
  particleList.clear();
  if(fEventID < fEntries)
  {
    fTree -> GetEntry(fEventID);
    int pdg;
    for(int i = 0; i < fMulti; ++i)
    {
      switch(fPID[i])
      {
        case 1: pdg = 2212; break;
        case 2: pdg = 2112; break;
        case 3: pdg = 1000010020; break;
        case 4: pdg = 1000020030; break;
        case 5: pdg = 1000010030; break;
        case 6: pdg = 1000020040; break;
        default: continue;
      }
      particleList.push_back({pdg, (double) fPx[i]/1000., (double) fPy[i]/1000., (double) fPz[i]/1000., // convert to GeV 
                                   (double) fX[i], (double) fY[i], (double) fZ[i]});
    }
    ++fEventID;
    return true;
  }
  else return false;
}

TString STpBUUReader::Print()
{ return TString::Format("pBUU reader with source %s", fFile.GetName()); }

std::vector<FairIon*> STpBUUReader::GetParticleList()
{
  std::vector<FairIon*> particleList;
  // pBUU only support ions of up to He4
  for(auto pdg : std::vector<std::pair<int, int>>{{2,1},{3,1},{3,2},{4,2}})
  {
    int a = pdg.first, z = pdg.second;
    particleList.push_back(new FairIon(Form("%d",a)+Elements::symbol[z-1],z,a,z));
  }
  return particleList; 
}

/*************************************************
* STTXTReader
**************************************************/
STTXTReader::STTXTReader(TString fileName, std::unique_ptr<STReaderHelper>&& helper) : fFile(fileName), fFilename(fileName), fHelper(std::move(helper))
{
  if(fFile.is_open())
    LOG(INFO) << "Reading data from " << fileName << FairLogger::endl;

  // IMPORTANT! this allows getline to throw exception when it reaches eof
  fFile.exceptions(std::ifstream::failbit|std::ifstream::badbit);

  std::vector<STTransportParticle> temp;
  while(true)
  {
    try
    {
      fHelper -> ReadNextEvent_(this, temp, true);
      ++fTotEntries;
    }
    catch(...)
    { break; }
  }
  this -> GoToEvent(-1);

}

bool STTXTReader::GoToEvent(int event)
{
  if(event >= fTotEntries) return false;
  if(event == fEventID) return true;
  // revert back to beginning if requested event is from before the current event
  if(event < fEventID)
  {
    fFile.clear();
    fFile.seekg(0);
    //fFile.seekg(std::ios::beg);
    fEventID = -1;
    fCurrentB = -1;
  }
  std::vector<STTransportParticle> temp;
  for(;fEventID < event - 1;)
    fHelper -> ReadNextEvent_(this, temp, true);
  fHelper -> ReadNextEvent_(this, fCurrentParticleList);
  return true;
}

bool STTXTReader::GetNext(std::vector<STTransportParticle>& particleList)
{
  if(fEventID >= fTotEntries) return false;
  particleList = fCurrentParticleList;
  try
  { fHelper -> ReadNextEvent_(this, fCurrentParticleList); }
  catch(...){ fCurrentB = -1; ++fEventID; }
  return true;
}

/*************
* UrQMD and helper
**************/

void STUrQMDHelper::ReadNextEvent_(STTXTReader* reader, std::vector<STTransportParticle>& particleList, bool skip)
{
  std::string line, temp;
  // ignore line 1 - 3
  for(int i = 1; i <= 3; ++i) std::getline(reader -> fFile, line);
  // impact parameter
  std::getline(reader -> fFile, line);

  if(!skip)
  {
    particleList.clear();
    std::stringstream ss(line);
    ss >> temp >> reader -> fCurrentB >> temp; 
  }
  // ignore line 5 to when the line starts with "pvec:"
  bool foundData = false;
  bool readMultNextLine = false; // if the line starts with "vec:" instead, multplicity is one line over (don't know why)
  for(int i = 5; i <= 100; ++i) 
  {
    std::getline(reader -> fFile, line);
    std::stringstream ss(line);
    ss >> temp;
    if(temp == "pvec:" || temp == "vec:") 
    {
      foundData = true;
      readMultNextLine = temp == "vec:";
      break;
    }
  }
  if(!foundData) throw std::runtime_error("UrQMD reader cannot find particle data");
  // first entry in the following line is multiplicity
  int mult;
  if(readMultNextLine) std::getline(reader -> fFile, line);
  std::getline(reader -> fFile, line);
  {
    std::stringstream ss(line);
    ss >> mult >> temp;
  }
  // discard one more line
  if(!readMultNextLine) std::getline(reader -> fFile, line);
  // particle information
  //const std::vector<int> supported_pdg{-211,211,2212,1000010020,1000010030,1000020030,1000020040};
  for(int i = 0; i < mult; ++i)
  {
    std::getline(reader -> fFile, line);
    if(!skip)
    {
      int itype, charge;
      double px, py, pz, x, y, z;
      std::stringstream ss(line);
      ss >> temp >> x >> y >> z >> temp >> px >> py >> pz >> temp >> itype >> temp >> charge;
      int pdg = this -> ITypeChargeToPDG(itype, charge);
      //if(std::find(supported_pdg.begin(), supported_pdg.end(), pdg) != supported_pdg.end())
      particleList.push_back({pdg, px, py, pz, x, y, z});
    }
  }
  ++(reader -> fEventID);
}

int STUrQMDHelper::ITypeChargeToPDG(int itype, int charge)
{
  int failed_pdg = -1;
  if(itype == 1) // nucleons, either proton or neutron
  {
    if(charge == 0) return 2112; // n
    else return 2212; // p
  }
  else if(itype == 101) // pions
  {
    if(charge == 1) return 211; // pi+
    else if(charge == -1) return -211; // pi-
    else return 111; // pi 0
  }
  else if(500 < itype && itype < 700)
  {
    int A = itype - 500;
    int Z = charge;
    if(Z == 0 || A == Z) return failed_pdg; // reject unphysical clusters that is either only proton or neutron
    return 1000000000 + Z*10000 + A*10;
  }
  else return failed_pdg;
}

TString STUrQMDReader::Print()
{ return "UrQMD reader with source " + fFilename; }

/**************************************************
* TPME HW reader
**************************************************/

void STHWHelper::ReadNextEvent_(STTXTReader* reader, std::vector<STTransportParticle>& particleList, bool skip)
{
  int mult;
  std::string line, temp;
  std::getline(reader -> fFile, line);
  {
    std::stringstream ss(line);
    ss >> mult;
  }
  if(!skip) particleList.clear();
  for(int i = 0; i < mult; ++i)
  {
    std::getline(reader -> fFile, line);
    if(!skip)
    {
      int itype;
      double mass, x, y, z, px, py, pz;
      std::stringstream ss(line);
      ss >> itype >> mass >> x >> y >> z >> px >> py >> pz;
      auto it = fIDToPDG.find(itype);
      if(it != fIDToPDG.end())
        particleList.push_back({it -> second, px, py, pz, x, y, z});
    }
  }
  ++(reader -> fEventID);
}

TString STHWReader::Print()
{ return "TPME HW reader with source " + fFilename; }

/**************************************************
* AMD reader
**************************************************/

void STAMDHelper::ReadNextEvent_(STTXTReader* reader, std::vector<STTransportParticle>& particleList, bool skip)
{
  std::string line, temp;
  // ignore first line if this is the start of the file
  if(reader -> fEventID < 0) std::getline(reader -> fFile, line);
  if(!skip) particleList.clear();
  int Z, N, curr_eventId = -1, eventId;
  double px, py, pz, b;
  int place;
  while(std::getline(reader -> fFile, line))
  {
    std::stringstream ss(line);
    ss >> Z >> N >> px >> py >> pz >> temp >> temp >> temp >> b >> eventId >> temp;
    if(curr_eventId == -1) curr_eventId = eventId;
    if(curr_eventId != eventId) 
    {  // we reach the next event. Go up by one line
      reader -> fFile.seekg(place);
      break;
    }
    else 
    {
      reader -> fCurrentB = b;
      place = reader -> fFile.tellg();
      if(!skip)
      {
        int A = Z + N;
        if(Z == 0 && N == 0) break; // end of file
        int pdg;
        if(Z == 1 && N == 0) pdg = 2212;
        else if(Z == 0 && N == 1) pdg = 2112;
        else pdg = 1000000000 + Z*10000 + A*10;
        particleList.push_back({pdg, A*px/1000, A*py/1000, A*pz/1000, 0, 0, 0});
      }
    }
  }
  ++(reader -> fEventID);
}

TString STAMDReader::Print()
{ return "AMD reader with source " + fFilename; }

/***************************************************
* DcQMD reader 2. Dan upload events with different format each time
****************************************************/

void STDcQMDNewHelper::ReadNextEvent_(STTXTReader* reader, std::vector<STTransportParticle>& particleList, bool skip)
{
  std::string line, temp;
  if(!skip) particleList.clear();
  int A, Z, multi;
  double px, py, pz, b;
  { // read first line for event info
    std::getline(reader -> fFile, line);
    std::stringstream ss(line);
    ss >> b >> multi;
    reader -> fCurrentB = b;
  }
  for(int i = 0; i < multi; ++i)
  {
    std::getline(reader -> fFile, line);
    std::stringstream ss(line);
    ss >> A >> Z >> px >> py >> pz;
    if(!skip)
    {
      int N = A - Z;
      int pdg;
      if(N < 0 || Z < 0) continue;
      if(Z == 1 && N == 0) pdg = 2212;
      else if(Z == 0 && N == 1) pdg = 2112;
      else pdg = 1000000000 + Z*10000 + (Z + N)*10;
      particleList.push_back({pdg, A*px, A*py, A*pz, 0, 0, 0});
    }
  }
  ++(reader -> fEventID);
}

TString STDcQMDNewReader::Print()
{ return "DcQMDNew reader with source " + fFilename; }

/***************************************************
* DcQMD reader for pion data. pion data is separated from nuclear data
****************************************************/

void STDcQMDPionHelper::ReadNextEvent_(STTXTReader* reader, std::vector<STTransportParticle>& particleList, bool skip)
{
  std::string line, temp;
  if(!skip) particleList.clear();
  int A, Z, multi, iso;
  double px, py, pz, x, y, z, b;
  { // read first line for event info
    std::getline(reader -> fFile, line);
    std::stringstream ss(line);
    ss >> multi >> b;
    reader -> fCurrentB = b;
  }
  for(int i = 0; i < multi; ++i)
  {
    std::getline(reader -> fFile, line);
    std::stringstream ss(line);
    ss >> temp >> x >> y >> z >> px >> py >> pz >> iso;
    if(!skip)
    {
      if(iso == 0) particleList.push_back({-211, px, py, -pz, 0, 0, 0});
      else if(iso == 2) particleList.push_back({211, px, py, -pz, 0, 0, 0});
    }
  }
  ++(reader -> fEventID);
}

TString STDcQMDPionReader::Print()
{ return "DcQMDPion reader with source " + fFilename; }



/***************************************************
* IQMD
****************************************************/

void STIQMDHelper::ReadNextEvent_(STTXTReader* reader, std::vector<STTransportParticle>& particleList, bool skip)
{
  std::string line, temp;
  if(!skip) particleList.clear();
  double Af, Zf, multi;
  double px, py, pz, b;
  { // read first line for event info
    std::getline(reader -> fFile, line);
    std::stringstream ss(line);
    ss >> temp >> multi;
  }
  int imulti = int(multi + 0.5);
  for(int i = 0; i < imulti; ++i)
  {
    std::getline(reader -> fFile, line);
    std::stringstream ss(line);
    ss >> Af >> Zf >> temp >> temp >> temp >> px >> py >> pz;
    int A = int(Af + 0.5);
    int Z = int(Zf + 0.5);
    if(!skip)
    {
      int N = A - Z;
      int pdg;
      if(Z == 1 && N == 0) pdg = 2212;
      else if(Z == 0 && N == 1) pdg = 2112;
      else pdg = 1000000000 + Z*10000 + (Z + N)*10;
      particleList.push_back({pdg, A*px, A*py, A*pz, 0, 0, 0});
    }
  }
  ++(reader -> fEventID);
}

TString STIQMDReader::Print()
{ return "IQMD reader with source " + fFilename; }


/**************************************************
* IBUU Reader
***************************************************/

STIBUUReader::STIBUUReader(TString fileName, bool flipZ) : fFile(fileName), fFilename(fileName)
{
  std::string line;
  while(std::getline(fFile, line))
  {
    double mass, x, y, z, px, py, pz;
    int ptype;
    std::stringstream ss(line);
    // In IBUU, projectile moves in -x direction
    if(ss >> ptype >> mass >> x >> y >> z >> px >> py >> pz)
      fParticleList.back().push_back({((ptype == 1)? 2212 : 2112), px, py, (flipZ)? -pz : pz, x, y, z});  
    else
      fParticleList.push_back(std::vector<STTransportParticle>());
  }
}

int STIBUUReader::GetEntries() { return fParticleList.size(); }
int STIBUUReader::GetEntry() { return fEventID; }
void STIBUUReader::SetEntry(int t_entry) { fEventID = t_entry; }

bool STIBUUReader::GetNext(std::vector<STTransportParticle>& particleList)
{
  if(fEventID >= fParticleList.size()) return false;

  particleList = fParticleList[fEventID];
  ++fEventID;
  return true;
}

TString STIBUUReader::Print()
{ return "IBUU reader with sou8rce " + fFilename; }

/**************************************************
* DcQMD Reader
***************************************************/

STDcQMDReader::STDcQMDReader(TString fileName) : fFile(fileName), fFilename(fileName)
{
  std::string line;
  while(std::getline(fFile, line))
  {
    double x, y, z, px, py, pz;
    int ptype, temp;
    std::stringstream ss(line);
    if(ss >> x >> y >> z >> px >> py >> pz >> ptype >> temp)
    {  
      // for dcQMD, the projectile is moving in negative z direction
      if(ptype < 100) fParticleList.back().push_back({((ptype == 1)? 2212 : 2112), px, py, -pz, x, y, z});  
    }
    else
    {
      // discard header columns
      std::getline(fFile, line);
      std::getline(fFile, line);

      fParticleList.push_back(std::vector<STTransportParticle>());
    }
  }
}

int STDcQMDReader::GetEntries() { return fParticleList.size(); }
int STDcQMDReader::GetEntry() { return fEventID; }
void STDcQMDReader::SetEntry(int t_entry) { fEventID = t_entry; }

bool STDcQMDReader::GetNext(std::vector<STTransportParticle>& particleList)
{
  if(fEventID >= fParticleList.size()) return false;

  particleList = fParticleList[fEventID];
  ++fEventID;
  return true;
}

TString STDcQMDReader::Print()
{ return "DcQMD reader with sou8rce " + fFilename; }




/**************************************************
 * Event generator itself
 * ************************************************/

STModelToLabFrameGenerator::STModelToLabFrameGenerator()
:FairGenerator(),
  fReader(nullptr), fInputPath(), fInputName(), fB(-1.), 
  fBeamInfo(NULL), fHeavyIonsRegistered(kFALSE),
  fCurrentEvent(0), fNEvents(0),
  fVertex(TVector3()), fVertexXYSigma(TVector2()), fTargetThickness(),
  fBeamAngle(TVector2(-0.022,0.)), fBeamAngleABSigma(TVector2(0.035,0)), fIsRandomRP(kTRUE)
{
  this -> RegisterReader();
}

STModelToLabFrameGenerator::STModelToLabFrameGenerator(TString fileName)
:FairGenerator("STModelToLabFrame",fileName),
  fReader(nullptr), fInputPath(), fInputName(fileName),
  fB(-1.), 
  fBeamInfo(NULL), fHeavyIonsRegistered(kFALSE),
  fCurrentEvent(0), fNEvents(0),
  fVertex(TVector3()), fVertexXYSigma(TVector2(0.42,0.36)), fTargetThickness(0.083),
  fBeamAngle(TVector2(-0.022,0.)), fBeamAngleABSigma(TVector2(0.035,0)), fIsRandomRP(kTRUE)
{
  fInputPath = TString::Format("%s/input/", gSystem->Getenv("VMCWORKDIR"));
  this -> RegisterReader();
}

STModelToLabFrameGenerator::STModelToLabFrameGenerator(TString filePath, TString fileName)
:FairGenerator("STModelToLabFrame",fileName),
  fReader(nullptr), fInputPath(filePath), fInputName(fileName),
  fB(-1.), 
  fBeamInfo(NULL), fHeavyIonsRegistered(kFALSE),
  fCurrentEvent(0), fNEvents(0),
  fVertex(TVector3()), fVertexXYSigma(TVector2(0.42,0.36)), fTargetThickness(0.043),
  fBeamAngle(TVector2(-0.022,0.)), fBeamAngleABSigma(TVector2(0.035,0)), fIsRandomRP(kTRUE)
{
  this -> RegisterReader();
}

STModelToLabFrameGenerator::~STModelToLabFrameGenerator()
{
}

void STModelToLabFrameGenerator::Print()
{
  auto info = TString::Format("Data info                 :\t%s\n"
                              "Beam energy per nucleon   :\t%.3g (GeV/c2)\n"
                              "Beam mass                 :\t%d\n"
                              "Beam charge               :\t%d\n"
                              "Target mass               :\t%d\n"
                              "Target thickness          :\t%.3g (cm)\n"
                              "Vertex mean XY            :\t%.3g\t%.3g (cm)\n"
                              "Vertex sigma XY           :\t%.3g\t%.3g (cm)\n"
                              "Vertex detector sigma XY  :\t%.3g\t%.3g (cm)\n"
                              "Beam angle mean AB        :\t%.3g\t%.3g (rad)\n"
                              "Beam angle sigma AB       :\t%.3g\t%.3g (rad)\n"
                              "Beam detector sigma AB    :\t%.3g\t%.3g (rad)\n",
                              (fReader)? fReader -> Print().Data() : "",
                              fBeamEnergyPerN,
                              fBeamMass,
                              fBeamCharge,
                              fTargetMass,
                              fTargetThickness,
                              fVertex.X(), fVertex.Y(),
                              fVertexXYSigma.X(), fVertexXYSigma.Y(),
                              fBeamDetectorVertexSigma.X(), fBeamDetectorVertexSigma.Y(),
                              fBeamAngle.X(), fBeamAngle.Y(),
                              fBeamAngleABSigma.X(), fBeamAngleABSigma.Y(),
                              fBeamDetectorABSigma.X(), fBeamDetectorABSigma.Y());
  TObjArray *tx = info.Tokenize("\n");
  for(int i = 0; i < tx->GetEntries(); ++i)
    LOG(INFO) << ((TObjString*) tx->At(i))->String() << FairLogger::endl;
}

void STModelToLabFrameGenerator::RegisterReader()
{
  fReader = ReaderFactory(fInputPath, fInputName);
  if(!fReader)
    LOG(FATAL)<<"STModelToLabFrameGenerator cannot accept event files without specifying generator names."<<FairLogger::endl;
  LOG(INFO)<<"-I Opening file: "<<fInputName<<FairLogger::endl;

  fNEvents = fReader -> GetEntries();

  auto ioMan = FairRootManager::Instance();
}

Bool_t STModelToLabFrameGenerator::Init()
{
  TString treeName, partBranchName;
  if(!fReader)
    LOG(FATAL) << "Source reader is not being set up propertly!" << FairLogger::endl;

  auto EBeam = fBeamEnergyPerN*fBeamMass + fBeamMass*fNucleonMass;
  auto PBeam = sqrt(EBeam*EBeam - fBeamMass*fBeamMass*fNucleonMass*fNucleonMass);
  TLorentzVector vBeam(0, 0, PBeam, EBeam + fTargetMass*fNucleonMass);
  fBoostVector = vBeam.BoostVector();

  if(!fHeavyIonsRegistered) this -> RegisterHeavyIon();
  fReader -> SetEntry(fCurrentEvent);

  LOG(INFO) << "Generator configuration " << FairLogger::endl;
  this -> Print();

  return true;
}


Bool_t STModelToLabFrameGenerator::ReadEvent(FairPrimaryGenerator* primGen)
{
  std::vector<STTransportParticle> particleList;
  if(!fReader -> GetNext(particleList))
    return kFALSE;

  TVector3 eventVertex(0,0,0);  // gaus dist. in target XY, uniform in target Z
  eventVertex.SetX(gRandom->Gaus(fVertex.X(),fVertexXYSigma.X()));
  eventVertex.SetY(gRandom->Gaus(fVertex.Y(),fVertexXYSigma.Y()));
  eventVertex.SetZ(fVertex.Z()+gRandom->Uniform(-fTargetThickness*0.5,fTargetThickness*0.5));

  /** Event rotation **/
  // ex.) TVector3::RotateX() -> clockwise rotation in the direction of positive X-axis.
  // Generated event will be rotated w.r.t reaction plane at first.
  // To adapt the beam angle to the event, rotate ta w.r.t. Y axis at first.
  // Next, w.r.t the rotated X axis(X' axis), rotate tb'(like tb in rotated frame).

  // reaction plane.
  Double_t phiRP = 0.;
  if(fIsRandomRP)
    phiRP = gRandom->Uniform(-1,1)*TMath::Pi();

  // exchange ta, tb -> rotation angles
  Double_t beamAngleA = gRandom->Gaus(fBeamAngle.X(),fBeamAngleABSigma.X());
  Double_t beamAngleB = gRandom->Gaus(fBeamAngle.Y(),fBeamAngleABSigma.Y());
  Double_t tanBeamA  = TMath::Tan(beamAngleA);
  Double_t tanBeamB  = TMath::Tan(beamAngleB);

  // detected beam angle is different from the real beam angle
  Double_t detectedBeamAngleA = gRandom->Gaus(beamAngleA, fBeamDetectorABSigma.X());
  Double_t detectedBeamAngleB = gRandom->Gaus(beamAngleB, fBeamDetectorABSigma.Y());
  Double_t detectedVertexX = gRandom->Gaus(eventVertex.X(), fBeamDetectorVertexSigma.X());
  Double_t detectedVertexY = gRandom->Gaus(eventVertex.Y(), fBeamDetectorVertexSigma.Y());

  TRotation rotatedFrame;             // set rotation operator for ta.
  rotatedFrame.RotateY(beamAngleA);   // rotate by angle-A of beam with respect to Y axis
  TVector3 axisX(1,0,0);              // define local X axis
  axisX.Transform(rotatedFrame);      // X' axis in rotated frame.
  Double_t thetaInRotatedFrame = -1.*TMath::ACos(TMath::Sqrt( (1.+tanBeamA*tanBeamA)/(tanBeamA*tanBeamA+tanBeamB*tanBeamB+1) ));  // tb' (be careful about the sign!!)
  TRotation rotateInRotatedFrame;
  rotateInRotatedFrame.Rotate(thetaInRotatedFrame,axisX); // rotate by angle-B' of beam with respect to X' axis.


  auto nPart = particleList.size();
  auto event = (FairMCEventHeader*) primGen->GetEvent();
  STFairMCEventHeader* castedEvent = dynamic_cast<STFairMCEventHeader*>(event);
  if( event && !(event->IsSet()) ){
    event->SetEventID(fCurrentEvent + 1);
    event->MarkSet(kTRUE);
    event->SetVertex(TVector3(detectedVertexX, detectedVertexY, fVertex.Z()));
    event->SetRotX(detectedBeamAngleA);
    event->SetRotY(detectedBeamAngleB);
    event->SetRotZ(phiRP); 
    event->SetB(fB);
    event->SetNPrim(nPart);
    if(dynamic_cast<STFairMCEventHeader*>(event))
    {
      castedEvent -> SetBeamZ(fBeamCharge);
      castedEvent -> SetBeamA(fBeamMass);
      castedEvent -> SetEnergyPerNucleons(fBeamEnergyPerN);
      castedEvent -> ClearTrackZRecord();
    }
  }

  Int_t numDiscarded = 0;
  // set up lorentz vector to transform 
  Int_t tracksCounter = 0;

  // shuffle if maximum multiplicity is needed
  if(fMaxMult > 0) std::random_shuffle(particleList.begin(), particleList.end());
  std::vector<STTransportParticle> acceptedParticles;
  for(const auto& particle : particleList)
  {
    Int_t pdg = particle.pdg;
    auto particleData = Elements::PDGToParticleData(pdg);
    if(!particleData) continue;
    int charge = particleData -> Charge()/3;
    // register if Z >= fHvyFragZ residue is found
    if(!(charge >= fHvyFragZ && fHvyFragAsCa40))
    {
      if(charge == 0 || (charge > fMaxZ && fMaxZ > 0))
        continue;
      if(fAllowedPDG.size() > 0)
        if(fAllowedPDG.find(pdg) == fAllowedPDG.end())
          continue;
    }
   
    acceptedParticles.push_back(particle);
    if(acceptedParticles.size() >= fMaxMult && fMaxMult > 0) break;
  }

  //for(auto& particle : acceptedParticles)
  for(int i = 0; i < acceptedParticles.size(); ++i)
  {
    auto& particle = acceptedParticles[i];
    auto particleData = Elements::PDGToParticleData(particle.pdg);
    auto mass = particleData -> Mass(); // It's already in GeV

    // transform the data to CM frame
    TVector3 p(particle.px, particle.py, particle.pz);
    p.SetPhi(p.Phi()+phiRP);  // random reaction plane orientation.

    TLorentzVector pCM(p.x(), p.y(), p.z(), sqrt(p.Mag2() + mass*mass));
    pCM.Boost(fBoostVector);
    p = pCM.Vect();
   
    int charge = particleData -> Charge()/3;
    int CaZ = 20;
    if(charge >= fHvyFragZ && fHvyFragAsCa40)
    {
      // keep p/Z constant for similar track shape
      p = (double(CaZ)/charge)*p;
      particle.pdg = 1000200400;
      if(castedEvent) castedEvent -> RecordTrackZ(i, charge);
    }

    p.RotateY(beamAngleA);    // rotate w.r.t Y axis
    p.Transform(rotateInRotatedFrame);

    primGen -> AddTrack(particle.pdg, p.X(), p.Y(), p.Z(), eventVertex.X(), eventVertex.Y(), eventVertex.Z());
  }
  // add dummy particles if there are no particles. 
  // it shoot in the wrong direction so it should not leave a signal in the detector
  // a work around for geant4 not accepting empty particle stack
  if(acceptedParticles.empty())
    primGen -> AddTrack(2212, 0, 0, 0, eventVertex.X(), eventVertex.Y(), eventVertex.Z());

  
  event -> SetNPrim((acceptedParticles.empty())? 1 : acceptedParticles.size());
  fCurrentEvent++;
  return kTRUE;
}

void STModelToLabFrameGenerator::RegisterHeavyIon(std::set<int> pdgList)
{
  std::vector<FairIon*> particleList;
  if(pdgList.empty())
    particleList = fReader -> GetParticleList();
  else
  {
    fAllowedPDG = pdgList;
    for(auto pdg : pdgList)
    {
      auto particleData = Elements::PDGToParticleData(pdg);
      auto z = int(particleData -> Charge()/3. + 0.5);
      auto a = int(particleData -> Mass()/Elements::kProtonMass + 0.5);
      if(a > 1) particleList.push_back(new FairIon(Form("%d",a)+Elements::symbol[z-1],z,a,z));
    }
  }
  if(fHvyFragAsCa40)
  {
    LOG(INFO) << "Will treat all Z>=20 fragments as 40Ca for Katana bias simulation." << FairLogger::endl;
    particleList.push_back(new FairIon("40Ca",20,40,20));
    if(!pdgList.empty()) fAllowedPDG.insert(1000200400);
  }

  fHeavyIonsRegistered = true;

  for(auto ion : particleList)
    FairRunSim::Instance()->AddNewIon(ion);

}


void STModelToLabFrameGenerator::SetMaxAllowedZ(int t_z)
{
  fMaxZ = t_z;
  LOG(INFO) << "Generator will discard all heavy ions with Z > " << fMaxZ << FairLogger::endl;
}

ClassImp(STModelToLabFrameGenerator);
