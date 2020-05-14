#include "STMassCalSimpleBB.hh"
#include "TError.h"
#include "TSystem.h"

STMassCalSimpleBB::STMassCalSimpleBB()
{
	gErrorIgnoreLevel = kBreak, kSysError, kFatal;
	finder.SetMethod(ROOT::Math::RootFinder::kGSL_SECANT);
	nPar=-1;
	bbPar = nullptr;
	bbParArray = new TObjArray();
	bbFitArray = new TObjArray();
	gradfunc1dBB = nullptr;
	//bbPar = new Double_t[8];
	//bbPar[2]=1.; bbPar[3]=0.;
	//funcBB  = [&](double x){ return bbfunc.fSimpleBBMassFinderEq(&x,bbPar); };
	//dfuncBB = [&](double x){ return bbfunc.fSimpleBBMassFinderDeriv(&x,bbPar); };
	//gradfunc1dBB = new ROOT::Math::GradFunctor1D(funcBB,dfuncBB);
	std::cout<<" === STMassCalSimpleBB is constructed."<<std::endl;
	std::cout<<" === Please keep in mind that gErrorIgnoreLevel is changed."<<std::endl;
}

STMassCalSimpleBB::STMassCalSimpleBB(TString funcName)
{
	gErrorIgnoreLevel = kBreak, kSysError, kFatal;
	finder.SetMethod(ROOT::Math::RootFinder::kGSL_SECANT);
	nPar=-1;
	bbParArray = new TObjArray();
	bbFitArray = new TObjArray();
	bbPar = nullptr;
	gradfunc1dBB = nullptr;
	std::cout<<" === STMassCalSimpleBB is constructed."<<std::endl;
	std::cout<<" === Please keep in mind that gErrorIgnoreLevel is changed."<<std::endl;
	if(funcName.EqualTo("BetheBloch")||funcName.EqualTo("BB")) UseBetheBloch();
	else if(funcName.EqualTo("SimpleBB")) UseSimpleBB();
	else if(funcName.EqualTo("EmpiricalBB")) UseEmpiricalBB();
	else{
		std::cout<<" === STMassCalSimpleBB::STMassCalSimpleBB(TString funcName)"<<std::endl;
		std::cout<<" === funcName: "<<funcName<<" is not implemented."<<std::endl;
		std::cout<<" === -> construct the object without setting the function to be used."<<std::endl;
	}
}

STMassCalSimpleBB::~STMassCalSimpleBB()
{
	delete bbPar;
	bbParArray->Delete();
	delete bbParArray;
	delete bbFitArray;
	delete gradfunc1dBB;
}

void STMassCalSimpleBB::UseBetheBloch()
{
	if(bbPar){ delete bbPar; bbPar=nullptr; }
	nPar=7;
	bbPar = new Double_t[7];
	bbPar[2]=1.; bbPar[3]=0.;
	funcBB  = [&](double x){ return bbfunc.fBBMassFinderEq(&x,bbPar); };
	dfuncBB = [&](double x){ return bbfunc.fBBMassFinderDeriv(&x,bbPar); };
	if(gradfunc1dBB){ delete gradfunc1dBB; gradfunc1dBB=nullptr; }
	gradfunc1dBB = new ROOT::Math::GradFunctor1D(funcBB,dfuncBB);
	std::cout<<" === STMassCalSimpleBB::UseBetheBloch() "<<std::endl;
}
void STMassCalSimpleBB::UseSimpleBB()
{
	if(bbPar){ delete bbPar; bbPar=nullptr; }
	nPar=8;
	bbPar = new Double_t[8];
	funcBB  = [&](double x){ return bbfunc.fSimpleBBMassFinderEq(&x,bbPar); };
	dfuncBB = [&](double x){ return bbfunc.fSimpleBBMassFinderDeriv(&x,bbPar); };
	if(gradfunc1dBB){ delete gradfunc1dBB; gradfunc1dBB=nullptr; }
	gradfunc1dBB = new ROOT::Math::GradFunctor1D(funcBB,dfuncBB);
	std::cout<<" === STMassCalSimpleBB::UseSimpleBB() "<<std::endl;
}
void STMassCalSimpleBB::UseEmpiricalBB()
{
	if(bbPar){ delete bbPar; bbPar=nullptr; }
	nPar=10;
	bbPar = new Double_t[10];
	funcBB  = [&](double x){ return bbfunc.fEmpiricalBBMassFinderEq(&x,bbPar); };
	dfuncBB = [&](double x){ return bbfunc.fEmpiricalBBMassFinderDeriv(&x,bbPar); };
	if(gradfunc1dBB){ delete gradfunc1dBB; gradfunc1dBB=nullptr; }
	gradfunc1dBB = new ROOT::Math::GradFunctor1D(funcBB,dfuncBB);
	std::cout<<" === STMassCalSimpleBB::UseEmpirical() "<<std::endl;
}


Double_t STMassCalSimpleBB::CalcMass(Double_t z, TVector3 mom, Double_t dEdx, Double_t *par)
{
	if(!(z==1.||z==2.)) return -1.;
	Double_t mass  = -1.;
	for(int iPar=0; iPar<nPar-3; ++iPar)
		bbPar[iPar] = par[iPar];
	bbPar[nPar-3] = z;
	bbPar[nPar-2] = mom.Mag();
	bbPar[nPar-1] = dEdx;
	finder.SetFunction(*gradfunc1dBB,z==1?1500:3500);
	if(finder.Solve())
		mass = finder.Root();
	return mass;
}

void STMassCalSimpleBB::AddParameter(TGraph2D* g2Par)
{
	bbParArray->Add(g2Par);
}

void STMassCalSimpleBB::AddParameters(TGraph2D** g2Pars)
{
	for(int iPar=0; iPar<nPar-3; ++iPar)
		bbParArray->Add(g2Pars[iPar]);
}

void STMassCalSimpleBB::AddParameter(TH2D* h2Par)
{
	bbParArray->Add(h2Par);
}

void STMassCalSimpleBB::AddParameters(TH2D** h2Pars)
{
	for(int iPar=0; iPar<nPar-3; ++iPar)
		bbParArray->Add(h2Pars[iPar]);
}

Double_t STMassCalSimpleBB::CalcMass(Double_t z, TVector3 mom, Double_t dEdx, Bool_t isInterpolation)
{
	if(!(z==1.||z==2.)) return -1.;
	if(bbParArray->GetEntries()!=nPar-3) return -1.;
	Double_t mass  = -1.;
	Double_t pitch = TMath::RadToDeg()*TMath::ATan(mom.Py()/mom.Pz());
	Double_t yaw   = TMath::RadToDeg()*TMath::ATan(mom.Px()/mom.Pz());
	for(int iPar=0; iPar<nPar-3; ++iPar){
		if(bbParArray->At(iPar)->InheritsFrom("TH2")){
			auto hist = (TH2D*)bbParArray->At(iPar);
			if(isInterpolation) bbPar[iPar] = hist->Interpolate(yaw,pitch);
			else                bbPar[iPar] = hist->GetBinContent(hist->FindBin(yaw,pitch));
		}
		else if(bbParArray->At(iPar)->InheritsFrom("TGraph2D"))
			bbPar[iPar] = ((TGraph2D*)bbParArray->At(iPar))->Interpolate(yaw,pitch);
	}
	bbPar[nPar-3] = z;
	bbPar[nPar-2] = mom.Mag();
	bbPar[nPar-1] = dEdx;
	finder.SetFunction(*gradfunc1dBB,z==1?1500:3500);
	if(finder.Solve())
		mass = finder.Root();
	return mass;
}

Double_t STMassCalSimpleBB::CalcZLog(Int_t pid, TVector3 mom, Double_t dEdx, Bool_t isInterpolation)
{
	if(!(pid>=0&&pid<=5)) return -1.;
	if(bbParArray->GetEntries()!=nPar-3) return -1.;
	Double_t mass  = -1.;
	Double_t pitch = TMath::RadToDeg()*TMath::ATan(mom.Py()/mom.Pz());
	Double_t yaw   = TMath::RadToDeg()*TMath::ATan(mom.Px()/mom.Pz());
	Double_t R     = mom.Mag();
	for(int iPar=0; iPar<nPar-3; ++iPar){
		if(bbParArray->At(iPar)->InheritsFrom("TH2")){
			auto hist = (TH2D*)bbParArray->At(iPar);
			if(isInterpolation) bbPar[iPar] = hist->Interpolate(yaw,pitch);
			else                bbPar[iPar] = hist->GetBinContent(hist->FindBin(yaw,pitch));
		}
		else if(bbParArray->At(iPar)->InheritsFrom("TGraph2D"))
			bbPar[iPar] = ((TGraph2D*)bbParArray->At(iPar))->Interpolate(yaw,pitch);
	}

	double z = TMath::Log(dEdx/bbfunc.fEmpiricalBB_PID(pid,&R,bbPar));
	return z;
}
