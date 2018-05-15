#include "STTransportModelEventGenerator.hh"
#include "FairLogger.h"
#include "FairRunSim.h"
#include "FairMCEventHeader.h"
#include "FairIon.h"
#include "TSystem.h"
#include "TRandom.h"
#include "UrQMDParticle.hh"

STTransportModelEventGenerator::STTransportModelEventGenerator()
	:  FairGenerator(),
	fInputFile(NULL), fInputTree(NULL),
	fB(-1.), fPartArray(NULL),
	fCurrentEvent(0), fNEvents(0),
	fVertex(TVector3()), fVertexXYSigma(TVector2()), fTargetThickness(),
	fBeamAngle(TVector2()), fBeamAngleABSigma(TVector2()), fIsRandomRP(kTRUE)
{
}

STTransportModelEventGenerator::STTransportModelEventGenerator(TString fileName)
	:  FairGenerator("STTransportModelEvent",fileName),
	fInputFile(NULL), fInputTree(NULL),
	fB(-1.), fPartArray(NULL),
	fCurrentEvent(0), fNEvents(0),
	fVertex(TVector3()), fVertexXYSigma(TVector2(0.42,0.36)), fTargetThickness(0.083),
	fBeamAngle(TVector2(0.-0.06)), fBeamAngleABSigma(TVector2()), fIsRandomRP(kTRUE)
{
	TString treeName, partBranchName;
	if(fileName.BeginsWith("phits"))       { fGen = TransportModel::PHITS;  treeName = "tree";      partBranchName = "fparts"; }
	else if(fileName.BeginsWith("amd"))    { fGen = TransportModel::AMD;    treeName = "amdTree";   partBranchName = "AMDParticle"; }
	else if(fileName.BeginsWith("urqmd"))  { fGen = TransportModel::UrQMD;  treeName = "urqmdTree"; partBranchName = "partArray"; }
	else
		LOG(FATAL)<<"STTransportModelEventGenerator cannot accept event files without specifying generator names."<<FairLogger::endl;

	TString inputDir = gSystem->Getenv("VMCWORKDIR");
	fInputFile = new TFile(inputDir+"/input/"+fileName);
	fInputTree = (TTree*)fInputFile->Get(treeName);

	LOG(INFO)<<"-I Opening file: "<<fileName<<FairLogger::endl;

	fInputTree -> SetBranchAddress("b",&fB);
	fInputTree -> SetBranchAddress(partBranchName,&fPartArray);

	fNEvents = fInputTree->GetEntries();
}

STTransportModelEventGenerator::STTransportModelEventGenerator(TString fileName, TString treeName)
	:  FairGenerator("STTransportModelEvent",fileName),
	fInputFile(NULL), fInputTree(NULL),
	fB(-1.), fPartArray(NULL),
	fCurrentEvent(0), fNEvents(0),
	fVertex(TVector3()), fVertexXYSigma(TVector2(0.42,0.36)), fTargetThickness(0.083),
	fBeamAngle(TVector2(0.-0.06)), fBeamAngleABSigma(TVector2()), fIsRandomRP(kTRUE)
{
	TString partBranchName;
	if(fileName.BeginsWith("phits"))       { fGen = TransportModel::PHITS;   partBranchName = "fparts"; }
	else if(fileName.BeginsWith("amd"))    { fGen = TransportModel::AMD;     partBranchName = "AMDParticle"; }
	else if(fileName.BeginsWith("urqmd"))  { fGen = TransportModel::UrQMD;   partBranchName = "partArray"; }
	else
		LOG(FATAL)<<"STTransportModelEventGenerator cannot accept event files without specifying generator names."<<FairLogger::endl;

	TString inputDir = gSystem->Getenv("VMCWORKDIR");
	fInputFile = new TFile(inputDir+"/input/"+fileName);
	fInputTree = (TTree*)fInputFile->Get(treeName);

	LOG(INFO)<<"-I Opening file: "<<fileName<<FairLogger::endl;

	fInputTree -> SetBranchAddress("b",&fB);
	fInputTree -> SetBranchAddress(partBranchName,&fPartArray);

	fNEvents = fInputTree->GetEntries();
}

STTransportModelEventGenerator::~STTransportModelEventGenerator()
{
}

Bool_t STTransportModelEventGenerator::ReadEvent(FairPrimaryGenerator* primGen)
{
	if(!fInputTree||fCurrentEvent==fNEvents)
		return kFALSE;

	TVector3 eventVertex(0,0,0);  // gaus dist. in target XY, uniform in target Z
	eventVertex.SetX(gRandom->Gaus(fVertex.X(),fVertexXYSigma.X()));
	eventVertex.SetY(gRandom->Gaus(fVertex.Y(),fVertexXYSigma.Y()));
	eventVertex.SetZ(fVertex.Z()+gRandom->Uniform(-fTargetThickness,fTargetThickness));

	TVector3 eventRotation(0,0,0);
	eventRotation.SetX(gRandom->Gaus(fBeamAngle.X(),fBeamAngleABSigma.X()));
	eventRotation.SetY(gRandom->Gaus(fBeamAngle.Y(),fBeamAngleABSigma.Y()));
	if(fIsRandomRP)
		eventRotation.SetZ(gRandom->Uniform(-1,1)*TMath::Pi());

	fInputTree -> GetEntry(fCurrentEvent);
	Int_t nPart = fPartArray->GetEntries();
	auto event = (FairMCEventHeader*)primGen->GetEvent();
	if( event && !(event->IsSet()) ){
		event->SetEventID(fCurrentEvent);
		event->MarkSet(kTRUE);
		event->SetVertex(eventVertex);
		event->SetRotX(eventRotation.X());
		event->SetRotY(eventRotation.Y());
		event->SetRotZ(eventRotation.Z());
		event->SetB(fB);
		event->SetNPrim(nPart);
	}

	for(Int_t iPart=0; iPart<nPart; iPart++){
		Int_t pdg;
		TVector3 p;
		TVector3 pos;
		switch(fGen){
			case TransportModel::PHITS:
				{
					auto part = (PHITSParticle*)fPartArray->At(iPart);
					pdg = kfToPDG(part->kf);
					Double_t pMag = TMath::Sqrt(part->ke*(part->ke+2.*part->m))/1000.;
					p = TVector3(part->mom[0],part->mom[1],part->mom[2]);
					p.SetMag(pMag);
					break;
				}
			case TransportModel::AMD:
				{
					auto part = (AMDParticle*)fPartArray->At(iPart);
					pdg = part->fPdg;
					p = TVector3(part->fMomentum.Vect());
					p.SetMag(p.Mag()/1000.);
					break;
				}
			case TransportModel::UrQMD:
				{
					auto part = (UrQMDParticle*)fPartArray->At(iPart);
					pdg = part->GetPdg();
					p = TVector3(part->GetMomentum().Vect());
					p.SetMag(p.Mag()/1000.);
					break;
				}
			default:
				break;
		}

		p.RotateX(eventRotation.X());
		p.RotateY(eventRotation.Y());
		p.RotateZ(eventRotation.Z());
		pos.RotateX(eventRotation.X());
		pos.RotateY(eventRotation.Y());
		pos.RotateZ(eventRotation.Z());
		pos += eventVertex;

		primGen -> AddTrack(pdg, p.X(), p.Y(), p.Z(), pos.X(), pos.Y(), pos.Z());
	}

	fCurrentEvent++;
	return kTRUE;
}

void STTransportModelEventGenerator::RegisterHeavyIon()
{
	if(!fInputFile)
		return;

	TString symbol[50] = {"H", "He", "Li", "Be", "B", "C", "N", "O", "F", "Ne",
		"Na", "Mg", "Al", "Si", "P", "S", "Cl", "Ar", "K", "Ca",
		"Sc", "Ti", "V", "Cr", "Mn", "Fe", "Co", "Ni", "Cu", "Zn",
		"Ga", "Ge", "As", "Se", "Br", "Kr", "Rb", "Sr", "Y", "Zr",
		"Nb", "Mo", "Tc", "Ru", "Rh", "Pd", "Ag", "Cd", "In", "Sn"};

	std::vector<Int_t> ions;
	for(Int_t i=0; i<fNEvents; i++){
		fInputTree->GetEntry(i);
		for(Int_t iPart=0; iPart<fPartArray->GetEntries(); iPart++){
			Int_t pdg;
			switch(fGen){
				case TransportModel::PHITS:
					{
						auto part = (PHITSParticle*)fPartArray->At(iPart);
						pdg = kfToPDG(part->kf);
						break;
					}
				case TransportModel::AMD:
					{
						auto part = (AMDParticle*)fPartArray->At(iPart);
						pdg = part->fPdg;
						break;
					}
				case TransportModel::UrQMD:
					{
						auto part = (UrQMDParticle*)fPartArray->At(iPart);
						pdg = part->GetPdg();
						break;
					}
				default:
					break;
			}
			if(pdg>3000)
				ions.push_back(pdg);
		}
	}
	std::sort(ions.begin(),ions.end());
	ions.resize(std::distance(ions.begin(),std::unique(ions.begin(),ions.end())));

	for(Int_t iIon=0; iIon<ions.size(); iIon++){
		auto z = (ions.at(iIon)%10000000)/10000;
		auto a = (ions.at(iIon)%10000)/10;

		FairRunSim::Instance()->AddNewIon(new FairIon(Form("%d",a)+symbol[z-1],z,a,z));
	}


}


Int_t STTransportModelEventGenerator::kfToPDG(Long64_t kfCode)
{
	if (kfCode < 1000002)
		return kfCode;
	else {
		Int_t A = kfCode%1000000;
		Int_t Z = kfCode/1000000;

		return (1000000000 + Z*10000 + A*10);
	}
}

ClassImp(STTransportModelEventGenerator);
