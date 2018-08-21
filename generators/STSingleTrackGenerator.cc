#include "STSingleTrackGenerator.hh"
#include "FairRunSim.h"
#include "FairLogger.h"
#include "FairMCEventHeader.h"
#include "TMath.h"
#include "TDatabasePDG.h"
#include "TParticlePDG.h"
#include "TRandom.h"

void VertexReader::OpenFile(const std::string& t_filename)
{
	file_.open(t_filename.c_str());
	if(!file_.is_open())
		LOG(FATAL) << "Vertex file " << t_filename << " cannot be opened!" << FairLogger::endl;
	else
		LOG(INFO) << "Loadiing vertex file " << t_filename << FairLogger::endl;
	// get rid of the header
	for(int i = 0; i < 2; ++i) std::getline(file_, line_);
}

void VertexReader::Next() 
{
	if(!std::getline(file_, line_))
		line_ = "";
} 

void VertexReader::LoopOver() 
{ 
	file_.clear(); 
	file_.seekg(0, std::ios::beg); 
	/* get rid of header*/ 
	for(int i = 0; i < 2; ++i) std::getline(file_, line_); 
}

TVector3 VertexReader::GetVertex()
{
	std::stringstream ss(line_);
	int temp;
	double x, y, z;
	if(!(ss >> temp >> temp >> x >> y >> z))
		LOG(FATAL) << "Vertex file cannot be read properly this line: " << line_ << FairLogger::endl;

	return TVector3(x, y, z);
}

ClassImp(STSingleTrackGenerator);

STSingleTrackGenerator::STSingleTrackGenerator()
: FairGenerator("STSingleTrackGenerator"),
  fNEvents(500),
  fPrimaryVertex(TVector3(0.,-21.33,-0.89)),
  fRandomMomentum(kFALSE),
  fUniRandomDirection(kFALSE), 
  fSpheRandomDirection(kFALSE), 
  fIsCocktail(kFALSE), fBrho(0.),
  fIsDiscreteTheta(kFALSE), fIsDiscretePhi(kFALSE),
  fNStepTheta(0), fNStepPhi(0)
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
{}

void STSingleTrackGenerator::ReadConfig(const std::string& t_config)
{
	TrackParser parser(t_config);
	std::vector<std::string> keys{	"NEvent",
					"Momentum",
					"VertexFile",
					"Particle",
					"Theta",
					"Phi"	};

	if(!parser.AllKeysExist(keys))
	{
		LOG(FATAL) << "Some keys are missing from the config file " << t_config << FairLogger::endl;
		return;
	}

	SetNEvents(parser.Get<int>("NEvent"));
	SetMomentum(parser.GetVector3("Momentum"));
	SetVertexFile(parser.Get<std::string>("VertexFile"));
	SetParticleList(std::vector<int>{parser.Get<int>("Particle")});
	SetThetaPhi(parser.Get<double>("Theta")*180./M_PI, parser.Get<double>("Phi")*180./M_PI);
	
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
      fVertexReader.Next();
      if(fVertexReader.IsEnd())
          fVertexReader.LoopOver();
      vertex = fVertexReader.GetVertex();
  }

  auto index = (Int_t)gRandom->Uniform(0,fPdgList.size());
  pdg = fPdgList.at(index);

  if(fRandomMomentum){
    Double_t mom = gRandom->Uniform(fMomentumRange[0], fMomentumRange[1]);
    momentum.SetMag(mom);
  }

  if(fUniRandomDirection){
    Double_t randTheta = gRandom->Uniform(fThetaRange[0],fThetaRange[1]);
    Double_t randPhi   = gRandom->Uniform(fPhiRange[0],fPhiRange[1]);
    momentum.SetMagThetaPhi(momentum.Mag(), randTheta, randPhi);
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

