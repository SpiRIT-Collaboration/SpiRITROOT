#include "STSingleTrackGenerator.hh"
#include "FairRunSim.h"
#include "FairLogger.h"
#include "FairMCEventHeader.h"
#include "TMath.h"
#include "TDatabasePDG.h"
#include "TParticlePDG.h"
#include "TRandom.h"
#include "TFile.h"

void VertexReader::OpenFile(const std::string& t_filename)
{
  std::ifstream file;
  file.open(t_filename.c_str());
  if(!file.is_open())
    LOG(FATAL) << "Vertex file " << t_filename << " cannot be opened!" << FairLogger::endl;
  else
    LOG(INFO) << "Loadiing vertex file " << t_filename << FairLogger::endl;
  // get rid of the header
  std::string line;
  std::getline(file, line);

  // load file into vectors
  while(std::getline(file, line))
  {
    std::stringstream ss(line);
    int temp;
    double x, y, z;
    if(!(ss >> temp >> temp >> x >> y >> z))
      LOG(FATAL) << "Vertex file cannot be read properly this line: " << line << FairLogger::endl;
    vectors_.push_back(TVector3(x, y, z));
  }

  it_ = vectors_.begin();
}

TrackParser::TrackParser(const std::string& t_filename)
{
  LOG(INFO) << "Reading configuration file " << t_filename << FairLogger::endl;
  LOG(INFO) << std::setw(20) << "Key" << std::setw(20) << "Content" << FairLogger::endl;

  std::ifstream file(t_filename.c_str());
  if(!file.is_open())
    LOG(FATAL) << "Cannot read generator file " << t_filename << FairLogger::endl;

  std::string line, key, content;
  while(std::getline(file, line))
  {
    // erase everything after # char
    auto pos = line.find("#");
    if(pos != std::string::npos)
      line.erase(line.begin() + pos, line.end());

    const auto first_char = line.find_first_not_of(" \t\r\n");
    if(first_char == std::string::npos) // skip if the line is empty
      continue;


    std::stringstream ss(line);
    // first element is the key, second one is content
    ss >> key >> std::ws;
    std::getline(ss, content);
    
    // check and see if key exist
    if( keys2lines_.find(key) != keys2lines_.end() )
      LOG(ERROR) << "Key value " << key << " is duplicated. Only the newest value will be loaded\n";

    keys2lines_[key] = content;
    
    LOG(INFO) << std::setw(20) << key << std::setw(20) << content << FairLogger::endl;
  }
}

bool TrackParser::AllKeysExist(const std::vector<std::string>& t_list_of_keys)
{
  for(const auto& key : t_list_of_keys)
    if(keys2lines_.find(key) == keys2lines_.end())
      return false;
  return true;
}

bool TrackParser::KeyExist(const std::string& t_key)
{
  if(keys2lines_.find(t_key) == keys2lines_.end())
    return false;
  else
    return true;
}

TVector3 TrackParser::GetVector3(const std::string& t_key)
{
  std::stringstream ss(keys2lines_.at(t_key));
  double x, y, z;
  if(!(ss >> x >> y >> z))
    LOG(FATAL) << t_key << " cannot be read as TVector as its content does not contain 3 values" << FairLogger::endl;

  return TVector3(x, y, z);
}

std::pair<double, double> TrackParser::GetBound(const std::string& t_key)
{
  std::stringstream ss(keys2lines_.at(t_key));
  double lower, upper;
  if(!(ss >> lower >> upper))
    LOG(FATAL) << t_key << " cannot be read as Bound as its content does not contain 2 values" << FairLogger::endl;

  return std::pair<double, double>{lower, upper};
}


ClassImp(STSingleTrackGenerator);

STSingleTrackGenerator::STSingleTrackGenerator()
: FairGenerator("STSingleTrackGenerator"),
  fNEvents(500),
  fPrimaryVertex(TVector3(0.,-21.33,-0.89)),
  fRandomMomentum(kFALSE),
  fUniRandomDirection(kFALSE), 
  fSpheRandomDirection(kFALSE), 
  fGausTheta(kFALSE),
  fGausPhi(kFALSE),
  fUniTheta(kFALSE),
  fUniPhi(kFALSE),
  fIsCocktail(kFALSE), fBrho(0.),
  fIsDiscreteTheta(kFALSE), fIsDiscretePhi(kFALSE),
  fNStepTheta(0), fNStepPhi(0),
  fGausMomentum(kFALSE), fGausMomentumMean(0), fGausMomentumSD(0)
{
  fPdgList.clear();
  fMomentum.SetXYZ(0., 0., .5);
  fMomentumRange[0] = 0.;
  fMomentumRange[1] = 0.;
  fThetaRange[0] = 0.;
  fThetaRange[1] = TMath::Pi()/2.;
  fPhiRange[0] = -TMath::Pi();
  fPhiRange[1] = TMath::Pi();

  RegisterHeavyIon();

}

STSingleTrackGenerator::~STSingleTrackGenerator()
{ delete fPhaseSpaceCut; }

void STSingleTrackGenerator::SetPhaseSpaceCut(const std::string& filename)
{
  TFile cutfile(filename.c_str());
  if(!cutfile.IsOpen())
    LOG(FATAL) << "Phase Space cut file " << filename << " cannot be opened." << FairLogger::endl;
  // search for CUTG in the file. 
  fPhaseSpaceCut = (TCutG*) cutfile.Get("CUTG");
  if(!fPhaseSpaceCut)
    LOG(FATAL) << "CUTG is not found in phase space cut file " << filename << FairLogger::endl;
}

void STSingleTrackGenerator::ReadConfig(const std::string& t_config)
{
  const double DEGTORAD = M_PI/180.;
  TrackParser parser(t_config);

  if(parser.KeyExist("VertexFile")) 
  {
    // load from VMCWORKDIR/parameters
    auto vmc_dir = gSystem->Getenv("VMCWORKDIR");
    SetVertexFile(std::string(vmc_dir) + "/parameters/" + parser.Get<std::string>("VertexFile"));
    SetNEvents(fVertexReader.GetNumEvent());
  }

  if(parser.KeyExist("Particle"))    SetParticleList(parser.GetList<int>("Particle"));
  if(parser.KeyExist("PrimaryVertex"))  SetPrimaryVertex(parser.GetVector3("PrimaryVertex"));
  if(parser.KeyExist("Momentum"))    SetMomentum(parser.Get<double>("Momentum"));
  if(parser.KeyExist("Momentum3"))  SetMomentum(parser.GetVector3("Momentum3"));
  if(parser.KeyExist("Direction"))  SetDirection(parser.GetVector3("Direction"));
  if(parser.KeyExist("PhaseSpaceCut"))  SetPhaseSpaceCut(parser.Get<std::string>("PhaseSpaceCut"));

  if(parser.KeyExist("Theta") && parser.KeyExist("Phi"))   
    SetThetaPhi(  parser.Get<double>("Theta")*DEGTORAD, 
        parser.Get<double>("Phi")*DEGTORAD);

  if(parser.KeyExist("RandomMomentum"))
  {
    auto bound = parser.GetBound("RandomMomentum");
    SetRandomMomentum(true, bound.first, bound.second);
  }

  if(parser.KeyExist("ThetaLimit"))
  {
    auto theta_bound = parser.GetBound("ThetaLimit");
    //SetUniformRandomDirection(true);
                fUniTheta = kTRUE;
    SetThetaLimit(  theta_bound.first*DEGTORAD, 
        theta_bound.second*DEGTORAD); 
  }

  if(parser.KeyExist("PhiLimit"))
  {
    auto phi_bound = parser.GetBound("PhiLimit");
    //SetUniformRandomDirection(true);
                fUniPhi = kTRUE;
    SetPhiLimit(  phi_bound.first*DEGTORAD, 
        phi_bound.second*DEGTORAD);
  }

  if(parser.KeyExist("CocktailEvent"))  SetCocktailEvent(parser.Get<double>("CocktailEvent"));
  if(parser.KeyExist("Brho"))    SetBrho(parser.Get<double>("Brho"));

  if(parser.KeyExist("DiscreteTheta"))
  {
    auto list = parser.GetList<double>("DiscreteTheta");
    SetDiscreteTheta(static_cast<int>(list[0]+0.5), list[1], list[2]);
  }

  if(parser.KeyExist("DiscretePhi"))
  {
    auto list = parser.GetList<double>("DiscretePhi");
    SetDiscretePhi(static_cast<int>(list[0]+0.5), list[1], list[2]);
  }

  if(parser.KeyExist("GausMomentum"))
  {
    auto list = parser.GetList<double>("GausMomentum");
    SetGausMomentum(list[0], list[1]);
  }

  if(parser.KeyExist("GausTheta"))
  {
    auto list = parser.GetList<double>("GausTheta");
    SetGausTheta(list[0]*DEGTORAD, list[1]*DEGTORAD);
  }

  if(parser.KeyExist("GausPhi"))
  {
    auto list = parser.GetList<double>("GausPhi");
    SetGausPhi(list[0]*DEGTORAD, list[1]*DEGTORAD);
  }
  
}


void STSingleTrackGenerator::SetParticleList(Int_t* pdgs)
{
  if(sizeof(pdgs)/sizeof(Int_t)==0)return;

  for(Int_t i=0; i<sizeof(pdgs)/sizeof(Int_t); i++)
    fPdgList.push_back(pdgs[i]);

  fPdgList.erase( std::unique(fPdgList.begin(), fPdgList.end()), fPdgList.end() );   // erase dupulicated index.

}


void STSingleTrackGenerator::SetCocktailEvent(Double_t setting=100.)
{

  if(setting==300.) fBrho = 5.4127*0.98; // BigRIPS F7 magnet value * 98%
  else if(setting==100.) fBrho = 3.0026*0.98;  // BigRIPS value * 98%
  else{
    LOG(ERROR) << "unknown cocktail event?? please set 100 or 300 as argument." << FairLogger::endl;
    return;
  }

  fIsCocktail = kTRUE;

  SetParticleList({2212,1000010020,1000010030,1000020030,1000020040,1000030060,1000030070,1000040070});
}

Bool_t STSingleTrackGenerator::ReadEvent(FairPrimaryGenerator* primGen)
{

  if(fPdgList.size()<=0){
    LOG(INFO)<<"No initial track pdg is set !! -> only Proton is produced in this run."<<FairLogger::endl;
    fPdgList.push_back(2212);
  }


  Int_t pdg;  // pdg code of the particle for this event
  TVector3 momentum=fMomentum, vertex=fPrimaryVertex;
  if(fVertexReader.IsOpen())
  {
      if(fVertexReader.IsEnd())
      {
          LOG(ERROR) << "Number of events should equal to number of vertex. This should not happens" << FairLogger::endl;
          fVertexReader.LoopOver();
      }
      vertex = fVertexReader.GetVertex();
      fVertexReader.Next();
  }

  auto index = (Int_t)gRandom->Uniform(0,fPdgList.size());
  pdg = fPdgList.at(index);

  // loop until the particle falles within the phase space
  const double RADTODEG=180./TMath::Pi();
  do 
  {
    if(fRandomMomentum){
      Double_t mom = gRandom->Uniform(fMomentumRange[0], fMomentumRange[1]);
      momentum.SetMag(mom);
    }

    if(fGausMomentum){
      Double_t mom = gRandom->Gaus(fGausMomentumMean, fGausMomentumSD);
      momentum.SetMag(mom);
    }

    if(fUniRandomDirection){
      Double_t randTheta = gRandom->Uniform(fThetaRange[0],fThetaRange[1]);
      Double_t randPhi   = gRandom->Uniform(fPhiRange[0],fPhiRange[1]);
      momentum.SetMagThetaPhi(momentum.Mag(), randTheta, randPhi);
    }

    if(fGausTheta){
      Double_t randTheta = gRandom->Gaus(fGausThetaMean, fGausThetaSD);
      momentum.SetTheta(fabs(randTheta));
    }


    if(fUniTheta){
      Double_t randTheta = gRandom->Uniform(fThetaRange[0],fThetaRange[1]);
      momentum.SetTheta(randTheta);
    }

    if(fUniPhi){
      Double_t randPhi   = gRandom->Uniform(fPhiRange[0],fPhiRange[1]);
      momentum.SetPhi(randPhi);
    }

    if(fGausPhi){
      Double_t randPhi = gRandom->Gaus(fGausPhiMean, fGausPhiSD);
      momentum.SetPhi(randPhi);
    }

    if(fSpheRandomDirection){
      Double_t px=0., py=0., pz=0., theta=-999., phi=0.;
      Double_t mom = momentum.Mag();
      while(theta<fThetaRange[0]||theta>fThetaRange[1]||phi<fPhiRange[0]||phi>fPhiRange[1]){
        gRandom->Sphere(px,py,pz,mom);
        TVector3 tempP(px,py,pz);
        theta = tempP.Theta();
        phi = tempP.Phi();
      }
      momentum.SetXYZ(px,py,pz);
    }

    if(fIsDiscreteTheta){
      auto tIndex = (Int_t)gRandom->Uniform(0,fNStepTheta);
      momentum.SetTheta(tIndex*(fThetaRange[1]-fThetaRange[0])/(Double_t)fNStepTheta);
    }
    if(fIsDiscretePhi){
      auto pIndex = (Int_t)gRandom->Uniform(0,fNStepPhi);
      momentum.SetPhi(pIndex*(fPhiRange[1]-fPhiRange[0])/(Double_t)fNStepPhi);
    }


    if(fIsCocktail||fBrho!=0.)
      momentum.SetMag(0.3*fBrho*GetQ(pdg));

    // exit the loop only when momentum is outside of phase space cut 
    if(!fPhaseSpaceCut) break;
  }while(fPhaseSpaceCut->IsInside(momentum.Theta()*RADTODEG, momentum.Phi()*RADTODEG));

  auto event = (FairMCEventHeader*)primGen->GetEvent();
  if( event && !(event->IsSet()) ){
    event->MarkSet(kTRUE);
    event->SetVertex(vertex);
  }

  primGen->AddTrack(pdg,momentum.X(),momentum.Y(),momentum.Z(),vertex.X(),vertex.Y(),vertex.Z());
  return kTRUE;
}

void STSingleTrackGenerator::RegisterHeavyIon()
{

  TString symbol[50] = {"H", "He", "Li", "Be", "B", "C", "N", "O", "F", "Ne",
    "Na", "Mg", "Al", "Si", "P", "S", "Cl", "Ar", "K", "Ca",
    "Sc", "Ti", "V", "Cr", "Mn", "Fe", "Co", "Ni", "Cu", "Zn",
    "Ga", "Ge", "As", "Se", "Br", "Kr", "Rb", "Sr", "Y", "Zr",
    "Nb", "Mo", "Tc", "Ru", "Rh", "Pd", "Ag", "Cd", "In", "Sn"};

  for(Int_t iH=2; iH<4; iH++)  // register d, t
    FairRunSim::Instance()->AddNewIon(new FairIon(Form("%d",iH)+symbol[0],1,iH,1));
  for(Int_t iHe=3; iHe<5; iHe++)       // register 3he, alpha
    FairRunSim::Instance()->AddNewIon(new FairIon(Form("%d",iHe)+symbol[1],2,iHe,2));
  for(Int_t iLi=6; iLi<8; iLi++)       // register 6Li, 7Li
    FairRunSim::Instance()->AddNewIon(new FairIon(Form("%d",iLi)+symbol[2],3,iLi,3));
  for(Int_t iBe=7; iBe<8; iBe++)       // register 7Be
    FairRunSim::Instance()->AddNewIon(new FairIon(Form("%d",iBe)+symbol[3],4,iBe,4));

}

Int_t STSingleTrackGenerator::GetQ(Int_t pdg)
{
  TParticlePDG* part = TDatabasePDG::Instance()->GetParticle(pdg);
  if(part)
    return TMath::Abs(part->Charge()*1./3.);
  else if( (pdg%10000000)/10000<=120 && (pdg%10000000)/10000>-2 )
    return (pdg%10000000)/10000;
  else
    return 0;
}

Int_t STSingleTrackGenerator::GetA(Int_t pdg)
{
  if(pdg==2212||pdg==2112)
    return 1;
  else if( (pdg%10000)/10<300 && (pdg%10000)/10>1 )
    return (pdg%10000)/10;
  else
    return 0;

}

